#pragma once

class PlayerWindowController : public ui::WindowController {
	static constexpr WCHAR* _windowClassName = L"Player";
protected:
	std::wstring _title;
	std::wstring _filePath;
	RefPtr<render::YUV420PVideoSource> _videoSource = new render::YUV420PVideoSource;
	RefPtr<ffmpeg::MoviePlayer> _player = new ffmpeg::MoviePlayer;
	RefPtr<ui::windows::NCHitTestView> _caption = new ui::windows::NCHitTestView;
	RefPtr<ui::Slider> _progress = new ui::Slider;
	RefPtr<ui::Label> _timeLabel = new ui::Label;
	RefPtr<ui::Label> _durationLabel = new ui::Label;
	RefPtr<ui::Button> _playButton = new ui::Button;
	RefPtr<ui::Button> _maximizeButton = new ui::Button;
	RefPtr<ui::Button> _minimizeButton = new ui::Button;
public:
	PlayerWindowController(const std::wstring& title);
	~PlayerWindowController();

	void load();
	void unload();

	void onInitWindow() override;
	void onDestroyWindow() override;
	void layoutWindow() override;

	void open(const std::string& path);
	void onSizing(const MSG& event);
};
