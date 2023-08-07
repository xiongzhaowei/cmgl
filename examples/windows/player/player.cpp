#include <sdkddkver.h>
#include <Windows.h>
#include "CMGL.h"

using namespace wheel;
#include "player.h"

static std::wstring timeLabel(int32_t time) {
	int32_t second = time % 60;
	time /= 60;
	int32_t minute = time % 60;
	time /= 60;
	int32_t hour = time;

	if (hour > 99) hour = 99;
	WCHAR text[9];
	wsprintfW(text, L"%02d:%02d:%02d", hour, minute, second);
	return text;
}

PlayerWindowController::PlayerWindowController(const std::wstring& title) : _title(title) {
	_window = new ui::windows::PlatformWindow(
		ui::windows::PlatformWindow::Style::create(_kWindowClassName, _title.c_str(), true, true, true, true, false, false, true, true),
		this
	);
}

void PlayerWindowController::onInitWindow() {
	_window->render([this](render::egl::EGLRenderContext* context) {
		_renderLayer->setSource(_videoSource);
		context->addSource(_renderLayer);
		context->addSource(window()->renderLayer());
	});

	_caption->setCode(HTCAPTION);

	_topImageView->setImage(ui::Image::argb(16, 256, [](uint32_t x, uint32_t y) {
		int alpha = 255 - y;
		int color = 0x19 * alpha / 255;
		return (alpha << 24) + RGB(color, color, color);
	}));
	_topView->addSubview(_topImageView);

	_titleLabel->setText(_title);
	_titleLabel->setFontSize(11);
	_titleLabel->setTextColor(ui::Color(0xFFFFFFFF));

	_minimizeButton->setImage(ui::Button::State::normal, loadImage(L"minimize"));
	_minimizeButton->onClicked([this]() {
		ShowWindow((HWND)window()->handle(), SW_MINIMIZE);
	});

	_maximizeButton->setImage(ui::Button::State::normal, loadImage(L"maximize"));
	_maximizeButton->onClicked([this]() {
		if (IsZoomed((HWND)window()->handle())) {
			ShowWindow((HWND)window()->handle(), SW_RESTORE);
		} else {
			ShowWindow((HWND)window()->handle(), SW_MAXIMIZE);
		}
	});

	_closeButton->setImage(ui::Button::State::normal, loadImage(L"close"));
	_closeButton->onClicked([this]() {
		DestroyWindow((HWND)window()->handle());
	});

	_bottomImageView->setImage(ui::Image::argb(64, 153, [](uint32_t x, uint32_t y) { return y << 24; }));
	_bottomView->addSubview(_bottomImageView);

	_playButton->setImage(ui::Button::State::normal, loadImage(L"play"));
	_playButton->setImage(ui::Button::State::hover, loadImage(L"play_hover"));
	_playButton->onClicked([this]() { play(!_player->isPlaying()); });

	_timeLabel->setFontSize(10);
	_timeLabel->setTextColor(ui::Color(0xFFFFFFFF));
	_timeLabel->setTextVerticalAlignment(ui::TextVerticalAlignment::center);

	_progress->foregroundImageView()->setBackgroundColor(ui::Color(0xFF247FFF));
	_progress->backgroundImageView()->setBackgroundColor(ui::Color(0x80808080));
	_progress->setProgressHeight(4);
	_progress->button()->setFrame(ui::Rect(0, 0, 24, 24));
	_progress->button()->setImage(ui::Button::State::normal, loadImage(L"slider_button"));
	_progress->onValueChanged([this]() {
		if (_player->ready()) {
			_player->seek((double)_progress->value());
			_timeLabel->setText(timeLabel((int32_t)_player->time()));
			window()->layoutIfNeeded();
		}
	});

	_durationLabel->setFontSize(10);
	_durationLabel->setTextColor(ui::Color(0xFFFFFFFF));
	_durationLabel->setTextVerticalAlignment(ui::TextVerticalAlignment::center);

	_volumeButton->setImage(ui::Button::State::normal, loadImage(L"volume"));
	_volumeButton->setImage(ui::Button::State::hover, loadImage(L"volume_hover"));
	_volumeButton->onClicked([this]() { setSlient(!_slient); });
	_volumeButton->View::onMouseEnter([this](const ui::MouseEvent& event) {
		window()->addSubview(_volumeView);
	});
	auto volumeLeaveCallback = [this](const ui::MouseEvent& event) {
		RefPtr<ui::View> view = window()->hitTest(event.pt.x, event.pt.y);
		if (view != _volumeButton && view != _volumeView && view != _volumeLabel && view != _volumeSlider) {
			_volumeView->removeFromSuperview();
		}
		window()->setNeedsLayout();
	};
	_volumeButton->View::onMouseLeave(volumeLeaveCallback);
	_volumeView->onMouseLeave(volumeLeaveCallback);
	_volumeLabel->onMouseLeave(volumeLeaveCallback);
	_volumeSlider->onMouseLeave(volumeLeaveCallback);

	_fullscreenButton->onClicked([this]() {
		if (IsZoomed((HWND)window()->handle())) {
			ShowWindow((HWND)window()->handle(), SW_RESTORE);
		} else {
			ShowWindow((HWND)window()->handle(), SW_MAXIMIZE);
		}
	});

	_volumeLabel->setFontSize(9);
	_volumeLabel->setTextColor(0xFFFFFFFF);
	_volumeLabel->setTextAlignment(ui::TextAlignment::center);
	_volumeLabel->setText(L"100%");

	_volumeSlider->foregroundImageView()->setBackgroundColor(ui::Color(0xFF247FFF));
	_volumeSlider->backgroundImageView()->setBackgroundColor(ui::Color(0x80808080));
	_volumeSlider->foregroundImageView()->layer()->setCornerRadius(3);
	_volumeSlider->backgroundImageView()->layer()->setCornerRadius(3);
	_volumeSlider->setMaxValue(0x10000);
	_volumeSlider->setMinValue(0);
	_volumeSlider->setValue(0x10000);
	_volumeSlider->setRotate(-90);
	_volumeSlider->setProgressHeight(8);
	_volumeSlider->button()->setFrame(ui::Rect(0, 0, 24, 24));
	_volumeSlider->button()->setImage(ui::Button::State::normal, loadImage(L"slider_button"));
	_volumeSlider->onDragging([=](int64_t value) { setVolume(value / double(0x10000)); });
	_volumeSlider->onValueChanged([=]() { setVolume(_volumeSlider->value() / double(0x10000)); });

	_volumeView->setBackgroundColor(0xFF000000);
	_volumeView->layer()->setCornerRadius(4);
	_volumeView->addSubview(_volumeLabel);
	_volumeView->addSubview(_volumeSlider);

	window()->setBackgroundColor(0);
	window()->addSubview(_caption);
	window()->addSubview(_topView);
	window()->addSubview(_bottomView);
	window()->addSubview(_titleLabel);
	window()->addSubview(_minimizeButton);
	window()->addSubview(_maximizeButton);
	window()->addSubview(_closeButton);
	window()->addSubview(_progress);
	window()->addSubview(_timeLabel);
	window()->addSubview(_durationLabel);
	window()->addSubview(_playButton);
	window()->addSubview(_volumeButton);
	window()->addSubview(_fullscreenButton);

	window()->show();
}

void PlayerWindowController::layoutWindow() {
	ui::Size size = window()->bounds().size;

	glm::vec2 videoSize = glm::vec2(_player->width(), _player->height());
	float scale = std::max(videoSize.x / size.width, videoSize.y / size.height);
	videoSize /= scale;
	_renderLayer->setSize(glm::vec2(videoSize.x, videoSize.y));
	_renderLayer->setPosition(glm::vec2((size.width - videoSize.x) / 2, (size.height - videoSize.y) / 2));

	_fullscreenButton->setImage(ui::Button::State::normal, loadImage(IsZoomed((HWND)window()->handle()) ? L"exit_fullscreen" : L"enter_fullscreen"));
	_fullscreenButton->setImage(ui::Button::State::hover, loadImage(IsZoomed((HWND)window()->handle()) ? L"exit_fullscreen_hover" : L"enter_fullscreen_hover"));

	_caption->setFrame(wheel::ui::Rect(0, 0, size.width, size.height));

	ui::LayoutConstraint::setLeftWidth(_topView, 0, size.width);
	ui::LayoutConstraint::setTopHeight(_topView, 0, 72);
	ui::LayoutConstraint::setLeftWidth(_topImageView, 0, ui::LayoutConstraint::width(_topView));
	ui::LayoutConstraint::setTopHeight(_topImageView, 0, ui::LayoutConstraint::height(_topView));
	ui::LayoutConstraint::setRightWidth(_closeButton, size.width - 24, 40);
	ui::LayoutConstraint::setCenterHeight(_closeButton, 36, 40);
	ui::LayoutConstraint::setRightWidth(_maximizeButton, ui::LayoutConstraint::left(_closeButton) - 24, 40);
	ui::LayoutConstraint::setCenterHeight(_maximizeButton, 36, 40);
	ui::LayoutConstraint::setRightWidth(_minimizeButton, ui::LayoutConstraint::left(_maximizeButton) - 24, 40);
	ui::LayoutConstraint::setCenterHeight(_minimizeButton, 36, 40);
	ui::LayoutConstraint::setLeftRight(_titleLabel, 32, ui::LayoutConstraint::left(_minimizeButton) - 10);
	ui::LayoutConstraint::setCenterHeight(_titleLabel, 36, 28);
	ui::LayoutConstraint::setLeftWidth(_bottomView, 0, size.width);
	ui::LayoutConstraint::setBottomHeight(_bottomView, size.height, 200);
	ui::LayoutConstraint::setLeftWidth(_bottomImageView, 0, ui::LayoutConstraint::width(_bottomView));
	ui::LayoutConstraint::setTopHeight(_bottomImageView, 0, ui::LayoutConstraint::height(_bottomView));
	ui::LayoutConstraint::setLeftWidth(_playButton, 24, 48);
	ui::LayoutConstraint::setBottomHeight(_playButton, size.height - 20, 48);
	ui::LayoutConstraint::setRightWidth(_fullscreenButton, size.width - 24, 48);
	ui::LayoutConstraint::setBottomHeight(_fullscreenButton, size.height - 20, 48);
	ui::LayoutConstraint::setRightWidth(_volumeButton, ui::LayoutConstraint::left(_fullscreenButton) - 24, 48);
	ui::LayoutConstraint::setBottomHeight(_volumeButton, size.height - 20, 48);
	ui::LayoutConstraint::setLeftWidth(_timeLabel, ui::LayoutConstraint::right(_playButton) + 32, 75);
	ui::LayoutConstraint::setCenterHeight(_timeLabel, ui::LayoutConstraint::centerY(_playButton), 40);
	ui::LayoutConstraint::setRightWidth(_durationLabel, ui::LayoutConstraint::left(_volumeButton) - 32, 75);
	ui::LayoutConstraint::setCenterHeight(_durationLabel, ui::LayoutConstraint::centerY(_playButton), 40);
	ui::LayoutConstraint::setLeftRight(_progress, ui::LayoutConstraint::right(_timeLabel) + 16, ui::LayoutConstraint::left(_durationLabel) - 16);
	ui::LayoutConstraint::setCenterHeight(_progress, ui::LayoutConstraint::centerY(_playButton), 24);
	ui::LayoutConstraint::setCenterWidth(_volumeView, ui::LayoutConstraint::centerX(_volumeButton), 90);
	ui::LayoutConstraint::setBottomHeight(_volumeView, ui::LayoutConstraint::top(_volumeButton), 300);
	ui::LayoutConstraint::setLeftRight(_volumeLabel, 0, ui::LayoutConstraint::width(_volumeView));
	ui::LayoutConstraint::setTopHeight(_volumeLabel, 24, 40);
	ui::LayoutConstraint::setCenterWidth(_volumeSlider, ui::LayoutConstraint::width(_volumeView) / 2, 200);
	ui::LayoutConstraint::setBottomHeight(_volumeSlider, ui::LayoutConstraint::height(_volumeView) - 100, 50);
}

void PlayerWindowController::onDestroyWindow() {
	_player->close();
	PostQuitMessage(0);
}

RefPtr<ui::Image> PlayerWindowController::loadImage(const std::wstring& name) {
	const std::wstring prefix = L"D:\\workspace\\cmgl\\examples\\windows\\player\\images\\";
	const std::wstring suffix = L".png";
	return ui::Image::file(prefix + name + suffix);
}

void PlayerWindowController::setVolume(double volume) {
	if (volume < 0.01) volume = 0;
	if (volume > 0.99) volume = 1;
	_slient = volume == 0;
	_volume = volume;
	_volumeSlider->setValue(int64_t(volume * 0x10000));
	_volumeLabel->setText(std::to_wstring(int32_t(volume * 100)) + L"%");
	_volumeButton->setImage(ui::Button::State::normal, loadImage(_slient ? L"volume_slient" : L"volume"));
	_volumeButton->setImage(ui::Button::State::hover, loadImage(_slient ? L"volume_slient_hover" : L"volume_hover"));
	if (_player->ready()) _player->setVolume(_slient ? 0 : volume);
}

void PlayerWindowController::setSlient(bool slient) {
	_slient = slient;
	_volumeSlider->setValue(_slient ? 0 : int64_t(_volume * 0x10000));
	_volumeLabel->setText(std::to_wstring(int32_t((_slient ? 0 : _volume) * 100)) + L"%");
	_volumeButton->setImage(ui::Button::State::normal, loadImage(_slient ? L"volume_slient" : L"volume"));
	_volumeButton->setImage(ui::Button::State::hover, loadImage(_slient ? L"volume_slient_hover" : L"volume_hover"));
	if (_player->ready()) _player->setVolume(_slient ? 0 : _volume);
}

void PlayerWindowController::open(const std::string& path) {
	//bool dsvFormat = false;
	//if (path.size() > 4) {
	//	std::string extension = path.substr(path.size() - 4);
	//	std::transform(extension.begin(), extension.end(), extension.begin(), std::tolower);
	//	dsvFormat = extension == ".dsv";
	//}
	//if (dsvFormat) {
	//	struct DSVFile : public ffmpeg::MovieFile {
	//		std::unique_ptr<uint8_t> _buffer = std::make_unique<uint8_t>(65536);
	//		dsv_decrypt_context* opaque = nullptr;
	//		DSVFile(dsv_decrypt_context* opaque) : opaque(opaque) {}
	//		~DSVFile() { dsv_decrypt_context_free(opaque); }
	//		uint8_t* buffer() const { return _buffer.get(); }
	//		int32_t bufferSize() const { return 65536; }
	//		int read(uint8_t* buf, int buf_size) { return dsv_read_packet(opaque, buf, buf_size); }
	//		int write(const uint8_t* buf, int buf_size) { return 0; }
	//		int64_t seek(int64_t offset, int whence) { return dsv_read_seek(opaque, offset, whence); }
	//	};
	//	dsv_decrypt_context* opaque = dsv_decrypt_context_init(path.c_str(), nullptr, nullptr);
	//	RefPtr<DSVFile> dsvFile;
	//	if (opaque != nullptr) {
	//		dsvFile = new DSVFile(opaque);
	//	}
	//	_player->open(dsvFile);
	//}
	if (!_player->ready()) {
		_player->open(path);
	}
	if (!_player->ready()) return;
	if (_window->handle() == nullptr) {
		glm::vec2 size = glm::vec2(_player->width(), _player->height());
		float scale = std::max(size.x / 1024, size.y / 768);
		size /= scale;
		_window->create(int32_t(size.x), int32_t(size.y), nullptr, true);
	}
	_player->bind(AV_PIX_FMT_YUV420P, [this, self = RefPtr<PlayerWindowController>(this)](wheel::RefPtr<wheel::ffmpeg::Frame> frame) {
		_videoSource->update(frame->frame());
		_progress->setValue((int64_t)_player->time());
		_timeLabel->setText(timeLabel((int32_t)_player->time()));
		window()->setNeedsDisplay();
	});
	_timeLabel->setText(timeLabel((int32_t)_player->time()));
	_durationLabel->setText(timeLabel((int32_t)_player->duration()));
	_progress->setMaxValue((int64_t)_player->duration());
	_player->setVolume(_slient ? 0 : _volume);
	window()->setNeedsLayout();
}

void PlayerWindowController::play(bool state) {
	_player->play(state);
	_playButton->setImage(ui::Button::State::normal, loadImage(_player->isPlaying() ? L"pause" : L"play"));
	_playButton->setImage(ui::Button::State::hover, loadImage(_player->isPlaying() ? L"pause_hover" : L"play_hover"));
}
