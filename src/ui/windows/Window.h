
#pragma once

OMP_UI_WINDOWS_NAMESPACE_BEGIN

class NCHitTestView : public View {
	LRESULT _code = HTCLIENT;
public:
	LRESULT code() const { return _code; }
	void setCode(LRESULT code) { _code = code; }
};

class PlatformWindow : public Window {
	uint32_t _borderSize = 0;
	ui::Size _minTrackSize = { 0, 0 };
	bool _isMouseTracking = false;
	RefPtr<render::RGBAVideoSource> _renderLayer;
	RefPtr<render::egl::EGLRenderContext> _eglContext;
	RefPtr<render::egl::EGLWindow> _eglWindow;
	RefPtr<View> _enteredView;
public:
	class EGLRenderContextImpl;
	class EGLWindowImpl;
	class Style;

	PlatformWindow(RefPtr<Style> style, WindowController* windowController);

	void* handle() const override { return _hWnd; }

	Color backgroundColor() const override {
		Color result(0xFF000000);
		if (_eglWindow) {
			glm::vec3 color = _eglWindow->backgroundColor();
			result.r = (uint8_t)round(color.r * 255);
			result.g = (uint8_t)round(color.g * 255);
			result.b = (uint8_t)round(color.b * 255);
		}
		return result;
	}
	void setBackgroundColor(Color color) override {
		if (_eglWindow) {
			glm::vec3 value;
			value.r = color.r / 255.0f;
			value.g = color.g / 255.0f;
			value.b = color.b / 255.0f;
			_eglWindow->setBackgroundColor(value);
		}
	}

	bool render(std::function<void(render::egl::EGLRenderContext*)> callback) override {
		if (!_eglContext) return false;
		if (!_eglWindow) return false;

		callback(_eglContext);
		layoutIfNeeded();
		if (_layer && _renderLayer) {
			RefPtr<gdiplus::Layer> gdiLayer = _layer.as<gdiplus::Layer>();
			if (gdiLayer != nullptr) {
				gdiLayer->paint(_renderLayer);
			}
		}
		_eglContext->render();
		return true;
	}

	bool create(int32_t width, int32_t height, Window* parent = nullptr, bool isGLESEnabled = false) override;
	void destroy();
	void show() override;
	void hide() override;
	void close() override;
	void setCaptureView(RefPtr<View> view) override;
	void setTitle(const std::wstring& title) override;
	bool showWindow(int nCmdShow);
	RefPtr<render::RenderSource> renderLayer() override;
	void setNeedsDisplay() override;
	void layoutSubviews() override;

	std::optional<LRESULT> onCreate(const MSG& event);
	std::optional<LRESULT> onDestroy(const MSG& event);

	std::optional<LRESULT> onNcActivate(const MSG& event);
	std::optional<LRESULT> onNcCalcSize(const MSG& event);
	std::optional<LRESULT> onNcHitTest(const MSG& event);
	std::optional<LRESULT> onGetMinMaxInfo(const MSG& event);
	std::optional<LRESULT> onSysCommand(const MSG& event);
	std::optional<LRESULT> onDpiChanged(const MSG& event);
	std::optional<LRESULT> onSize(const MSG& event);
	std::optional<LRESULT> onPaint(const MSG& event);

	std::optional<LRESULT> onMouseMove(const MSG& event);
	std::optional<LRESULT> onMouseHover(const MSG& event);
	std::optional<LRESULT> onMouseLeave(const MSG& event);
	std::optional<LRESULT> onMouseWheel(const MSG& event);
	std::optional<LRESULT> onMouseDown(const MSG& event);
	std::optional<LRESULT> onMouseUp(const MSG& event);
	std::optional<LRESULT> onDoubleClick(const MSG& event);

	std::optional<LRESULT> handleMouseEvent(const MouseEvent& event) override;
	std::optional<LRESULT> handleKeyboardEvent(const KeyboardEvent& event) override;
	std::optional<LRESULT> handleNativeEvent(const MSG& event) override;

	static LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND _hWnd = NULL;
	RefPtr<Style> _style;
	WeakPtr<WindowController> _windowController;
};

class PlatformWindow::Style : public Object {
	HINSTANCE _instance;
	std::basic_string<TCHAR> _className;
	std::basic_string<TCHAR> _title;
	uint32_t _style;
	uint32_t _exStyle;
	uint32_t _classStyle;
	bool _fullscreen;
	bool _dwmShadow;
public:
	static RefPtr<Style> create(
		std::basic_string<TCHAR> className,
		std::basic_string<TCHAR> title,
		bool doubleClick,
		bool minimize,
		bool maximize,
		bool close,
		bool fixedSize,
		bool layeredWindow,
		bool fullscreen,
		bool dwmShadow
	);

	Style(HINSTANCE hInstance, std::basic_string<TCHAR> className, std::basic_string<TCHAR> title, uint32_t style, uint32_t exStyle, uint32_t classStyle, bool fullscreen, bool dwmShadow);
	~Style();

	HINSTANCE instance() const { return _instance; }
	std::basic_string<TCHAR> className() const { return _className; }
	std::basic_string<TCHAR> title() const { return _title; }
	uint32_t style() const { return _style; }
	uint32_t exStyle() const { return _exStyle; }
	uint32_t classStyle() const { return _classStyle; }
	bool fullscreen() const { return _fullscreen; }
	bool dwmShadow() const { return _dwmShadow; }
};

class PlatformWindow::EGLWindowImpl : public render::egl::EGLWindow {
	wheel::WeakPtr<PlatformWindow> _window;
public:
	EGLWindowImpl(PlatformWindow* window);
	HWND handle() const override;
	render::vec2 size() const override;
};

class PlatformWindow::EGLRenderContextImpl : public render::egl::EGLRenderContext {
	HWND _hWnd;
	HDC _hDC;
public:
	EGLRenderContextImpl(HWND hWnd);
	~EGLRenderContextImpl();
};

OMP_UI_WINDOWS_NAMESPACE_END
