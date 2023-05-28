
OMP_UI_WINDOWS_NAMESPACE_BEGIN

class WindowController : Object {
	WeakPtr<Window> _window;
public:
	Window* window();

	virtual void load();
	virtual void unload();

	virtual void initWindow();
	virtual void destroyWindow();
};

class NCHitTestView : public View {
	LRESULT _code = HTCLIENT;
public:
	LRESULT code() const { return _code; }
	void setCode(LRESULT code) { _code = code; }
};

class PlatformWindow : public Window {
	uint32_t _borderSize = 0;
	ui::Size _minTrackSize = { 0, 0 };
	RefPtr<render::egl::EGLRenderContext> _eglContext;
	RefPtr<render::egl::EGLWindow> _eglWindow;
public:
	class EGLRenderContextImpl;
	class EGLWindowImpl;
	class Style;

	PlatformWindow(Style* config);

	HWND handle() const { return _hWnd; }

	bool render(std::function<void(render::egl::EGLRenderContext*)> callback) {
		if (!_eglContext) return false;
		if (!_eglWindow) return false;

		callback(_eglContext);
		_eglContext->render();
		return true;
	}

	bool create(int32_t width, int32_t height, PlatformWindow* parent = nullptr, bool isGLESEnabled = false);
	void destroy();
	bool showWindow(int nCmdShow);

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
public:
	static Style* create(
		std::basic_string<TCHAR> className,
		std::basic_string<TCHAR> title,
		bool doubleClick,
		bool minimize,
		bool maximize,
		bool close,
		bool fixedSize,
		bool layeredWindow
	);

	Style(HINSTANCE hInstance, std::basic_string<TCHAR> className, std::basic_string<TCHAR> title, uint32_t style, uint32_t exStyle, uint32_t classStyle);
	~Style();

	HINSTANCE instance() const { return _instance; }
	std::basic_string<TCHAR> className() const { return _className; }
	std::basic_string<TCHAR> title() const { return _title; }
	uint32_t style() const { return _style; }
	uint32_t exStyle() const { return _exStyle; }
	uint32_t classStyle() const { return _classStyle; }
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
