#include <sdkddkver.h>
#include <Windows.h>
#include "CMGL.h"

using namespace wheel;
#include "player.h"

PlayerWindowController::PlayerWindowController(const std::wstring& title) : _title(title) {
	_window = new ui::windows::PlatformWindow(
		ui::windows::PlatformWindow::Style::create(_windowClassName, _title.c_str(), true, true, true, true, false, false, true, true),
		this
	);
}

PlayerWindowController::~PlayerWindowController() {

}

void PlayerWindowController::load() {

}

void PlayerWindowController::unload() {

}

void PlayerWindowController::onInitWindow() {
	_window->render([this](render::egl::EGLRenderContext* context) {
		context->addSource(_videoSource);
		context->addSource(window()->renderLayer());
	});

	_caption->setCode(HTCAPTION);

	_progress->foregroundImageView()->setBackgroundColor(ui::Color(0xFF0000FF));
	_progress->backgroundImageView()->setBackgroundColor(ui::Color(0x80808080));
	_progress->onValueChanged([=]() {
		if (_player->ready()) _player->seek((double)_progress->value());
	});

	_timeLabel->setTextColor(ui::Color(0xFFFFFFFF));
	_durationLabel->setTextColor(ui::Color(0xFFFFFFFF));

	_playButton->setFrame(ui::Rect(0, 0, 90, 90));
	_playButton->setImage(ui::Button::State::normal, ui::Image::file(L"D:\\workspace\\ShareKaro\\src\\SHAREit\\shareit-pc\\windows\\resources\\themes\\default\\flutter\\close_normal.png"));
	_playButton->setImage(ui::Button::State::hover, ui::Image::file(L"D:\\workspace\\ShareKaro\\src\\SHAREit\\shareit-pc\\windows\\resources\\themes\\default\\flutter\\close_focus.png"));
	_playButton->setImage(ui::Button::State::pressed, ui::Image::file(L"D:\\workspace\\ShareKaro\\src\\SHAREit\\shareit-pc\\windows\\resources\\themes\\default\\flutter\\close_pressed.png"));
	_playButton->onClicked([=]() {
		_player->play(!_player->isPlaying());
	});

	window()->setBackgroundColor(ui::Color(0));
	window()->addSubview(_caption);
	window()->addSubview(_progress);
	window()->addSubview(_timeLabel);
	window()->addSubview(_durationLabel);
	window()->addSubview(_playButton);

	layoutWindow();

	window()->show();
}

void PlayerWindowController::onDestroyWindow() {
	_player->close();
	PostQuitMessage(0);
}

static std::wstring timeLabel(int32_t time) {
	int32_t second = time % 60;
	time /= 60;
	int32_t minute = time % 60;
	time /= 60;
	int32_t hour = time;

	std::wstring text = std::to_wstring(minute) + L":" + std::to_wstring(second);
	if (hour) text = std::to_wstring(hour) + L":" + text;
	return text;
}

void PlayerWindowController::layoutWindow() {
	ui::Size size = window()->bounds().size;

	_caption->setFrame(wheel::ui::Rect(0, 0, size.width, size.height));
	_progress->setFrame(ui::Rect(0, 32, size.width, 20));
	_timeLabel->setFrame(ui::Rect(0, 90, size.width, 30));
	_durationLabel->setFrame(ui::Rect(0, 120, size.width, 30));
}

void PlayerWindowController::open(const std::string& path) {
	_player->open(path);
	if (_window->handle() == nullptr) {
		_window->create(800, 600, nullptr, true);
	}
	_player->bind(AV_PIX_FMT_YUV420P, [this](wheel::RefPtr<wheel::ffmpeg::Frame> frame) {
		_videoSource->update(frame->frame());
		_progress->setValue((int64_t)_player->time());
		_timeLabel->setText(timeLabel((int32_t)_player->time()));
		window()->setNeedsDisplay();
	});
	_durationLabel->setText(timeLabel((int32_t)_player->duration()));
	_videoSource->setEnabled(true);
	_progress->setMaxValue((int64_t)_player->duration());
	_player->play(true);
}
