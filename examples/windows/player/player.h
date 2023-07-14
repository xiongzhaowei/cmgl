#pragma once

class PlayerWindowController : public ui::WindowController {
protected:
	static constexpr WCHAR* _kWindowClassName = L"Player";
	std::wstring _title;
	std::wstring _filePath;
	double _volume = 1;
	bool _slient = false;
	RefPtr<render::YUV420PVideoSource> _videoSource = new render::YUV420PVideoSource;
	RefPtr<render::RenderLayer> _renderLayer = new render::RenderLayer;
	RefPtr<ffmpeg::MoviePlayer> _player = new ffmpeg::MoviePlayer;
	RefPtr<ui::windows::NCHitTestView> _caption = new ui::windows::NCHitTestView;
	RefPtr<ui::View> _topView = new ui::View;
	RefPtr<ui::ImageView> _topImageView = new ui::ImageView;
	RefPtr<ui::Label> _titleLabel = new ui::Label;
	RefPtr<ui::Button> _minimizeButton = new ui::Button;
	RefPtr<ui::Button> _maximizeButton = new ui::Button;
	RefPtr<ui::Button> _closeButton = new ui::Button;
	RefPtr<ui::View> _bottomView = new ui::View;
	RefPtr<ui::ImageView> _bottomImageView = new ui::ImageView;
	RefPtr<ui::Button> _playButton = new ui::Button;
	RefPtr<ui::Label> _timeLabel = new ui::Label;
	RefPtr<ui::Slider> _progress = new ui::Slider;
	RefPtr<ui::Label> _durationLabel = new ui::Label;
	RefPtr<ui::Button> _volumeButton = new ui::Button;
	RefPtr<ui::View> _volumeView = new ui::View;
	RefPtr<ui::View> _volumeRegionView = new ui::View;
	RefPtr<ui::Slider> _volumeSlider = new ui::Slider;
	RefPtr<ui::Label> _volumeLabel = new ui::Label;
	RefPtr<ui::Button> _fullscreenButton = new ui::Button;
public:
	PlayerWindowController(const std::wstring& title);

	void onInitWindow() override;
	void onDestroyWindow() override;
	void layoutWindow() override;

	RefPtr<ui::Image> loadImage(const std::wstring& name);

	void setVolume(double volume);
	void setSlient(bool slient);

	void open(const std::string& path);
	void play(bool state);
};
