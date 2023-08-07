#include "defines.h"
#include <dwmapi.h>
#include <windowsx.h>
#include <Windows.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

OMP_UI_WINDOWS_USING_NAMESPACE

PlatformWindow::Style::Style(HINSTANCE hInstance, std::basic_string<TCHAR> className, std::basic_string<TCHAR> title, uint32_t style, uint32_t exStyle, uint32_t classStyle, bool fullscreen, bool dwmShadow) : _instance(hInstance), _className(className), _title(title), _style(style), _exStyle(exStyle), _classStyle(classStyle), _fullscreen(fullscreen), _dwmShadow(dwmShadow) {
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

	wcex.style = _classStyle;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hIcon = NULL;
	wcex.lpfnWndProc = WindowProc;
	wcex.hInstance = _instance;
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _className.c_str();
	::RegisterClassEx(&wcex);
}

PlatformWindow::Style::~Style() {
	UnregisterClass(_className.c_str(), _instance);
}

RefPtr<PlatformWindow::Style> PlatformWindow::Style::create(
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
) {
	DWORD dwStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CLIPCHILDREN;
	DWORD dwExStyle = 0;
	DWORD dwClassStyle = 0;

	if (doubleClick) dwClassStyle |= CS_DBLCLKS;
	if (minimize) dwStyle |= WS_MINIMIZEBOX;
	if (maximize) dwStyle |= WS_MAXIMIZEBOX;
	if (!close) dwClassStyle |= CS_NOCLOSE;
	if (!fixedSize) dwStyle |= WS_THICKFRAME;
	if (layeredWindow) dwExStyle |= WS_EX_LAYERED;

	return new Style((HINSTANCE)&__ImageBase, className, title, dwStyle, dwExStyle, dwClassStyle, fullscreen, dwmShadow);
}

LRESULT PlatformWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PlatformWindow* self = nullptr;
	if (uMsg == WM_NCCREATE) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		self = static_cast<PlatformWindow*>(lpcs->lpCreateParams);
		self->retain();
		self->_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
	} else {
		self = reinterpret_cast<PlatformWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}
	std::optional<LRESULT> result;
	if (self != nullptr) {
		MSG event = { hWnd, uMsg, wParam, lParam };
		result = self->handleNativeEvent(event);
		if (uMsg == WM_NCDESTROY) {
			::SetWindowLongPtr(self->_hWnd, GWLP_USERDATA, 0L);
			self->_hWnd = NULL;
			self->release();
		}
	}
	if (!result.has_value()) {
		LRESULT value = 0;
		if (DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &value)) {
			result = value;
		}
	}
	return result.has_value() ? result.value() : DefWindowProc(hWnd, uMsg, wParam, lParam);
}

PlatformWindow::PlatformWindow(RefPtr<Style> style, WindowController* windowController) : _style(style), _windowController(windowController) {

}

bool PlatformWindow::create(int32_t width, int32_t height, Window* parent, bool isGLESEnabled) {
	_layer->setAnchorPoint(Point{ 0, 0 });
	_layer->setBackgroundColor(0);
	if (isGLESEnabled) _eglWindow = new EGLWindowImpl(this);
	HWND hWnd = CreateWindowEx(
		_style->exStyle(),
		_style->className().c_str(),
		_style->title().c_str(),
		_style->style(),
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		parent ? (HWND)parent->handle() : NULL,
		NULL,
		_style->instance(),
		this
	);
	LONG nWindowLong = GetWindowLong(hWnd, GWL_STYLE);
	if (nWindowLong & WS_CAPTION) {
		SetWindowLong(hWnd, GWL_STYLE, nWindowLong & ~WS_CAPTION);
	}
	if (hWnd == INVALID_HANDLE_VALUE || hWnd == NULL) return false;
	return true;
}

void PlatformWindow::destroy() {
	if (_hWnd != NULL) DestroyWindow(_hWnd);
}

bool PlatformWindow::showWindow(int nCmdShow) {
	return ::ShowWindow(_hWnd, nCmdShow);
}

void PlatformWindow::show() {
	showWindow(SW_SHOW);
}

void PlatformWindow::hide() {
	showWindow(SW_HIDE);
}

void PlatformWindow::close() {
	CloseWindow(_hWnd);
}

void PlatformWindow::setCaptureView(RefPtr<View> view) {
	if (_captureView == nullptr && view != nullptr) {
		SetCapture(_hWnd);
	} else if (_captureView != nullptr && view == nullptr) {
		ReleaseCapture();
	}
	Window::setCaptureView(view);
}

void PlatformWindow::setTitle(const std::wstring& title) {
	SetWindowText(_hWnd, title.c_str());
}

RefPtr<render::RenderSource> PlatformWindow::renderLayer() {
	RefPtr<EGLLayer> eglLayer = _layer.as<EGLLayer>();
	if (eglLayer) return eglLayer->layer();

	if (_renderLayer == nullptr) _renderLayer = new render::RGBAVideoSource;
	return _renderLayer;
}

void PlatformWindow::setNeedsDisplay() {
	::InvalidateRect((HWND)handle(), nullptr, true);
}

void PlatformWindow::layoutSubviews() {
	if (_windowController) _windowController->layoutWindow();
}

std::optional<LRESULT> PlatformWindow::handleMouseEvent(const MouseEvent& event) {
	if (_windowController) {
		std::optional<LRESULT> result = _windowController->handleMouseEvent(event);
		if (result != std::nullopt) return result.value();
	}
	if (event.native.message == WM_MOUSEMOVE && !_isMouseTracking) {
		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.hwndTrack = (HWND)handle();
		tme.dwHoverTime = HOVER_DEFAULT;
		_isMouseTracking = _TrackMouseEvent(&tme);
	} else if (event.native.message == WM_MOUSEHOVER || event.native.message == WM_MOUSELEAVE) {
		_isMouseTracking = false;
	}
	View* view = hitTest(event.pt.x, event.pt.y);
	if (view == nullptr) view = this;
	switch (event.native.message) {
	case WM_MOUSEMOVE:
		if (view != _enteredView) {
			if (_enteredView) _enteredView->onMouseLeave(event);
			_enteredView = view;
			if (_enteredView) _enteredView->onMouseEnter(event);
		}
		view->onMouseMove(event);
		break;
	case WM_MOUSELEAVE:
		if (_enteredView) {
			_enteredView->onMouseLeave(event);
			_enteredView = nullptr;
		}
		break;
	case WM_MOUSEHOVER:
		view->onMouseHover(event);
		break;
	case WM_MOUSEWHEEL:
		view->onMouseWheel(event);
		break;
	//case WM_NCLBUTTONDOWN:
	case WM_LBUTTONDOWN:
	//case WM_NCRBUTTONDOWN:
	case WM_RBUTTONDOWN:
	//case WM_NCMBUTTONDOWN:
	case WM_MBUTTONDOWN:
	//case WM_NCXBUTTONDOWN:
	case WM_XBUTTONDOWN:
		view->onMouseDown(event);
		break;
	//case WM_NCLBUTTONUP:
	case WM_LBUTTONUP:
	//case WM_NCRBUTTONUP:
	case WM_RBUTTONUP:
	//case WM_NCMBUTTONUP:
	case WM_MBUTTONUP:
	//case WM_NCXBUTTONUP:
	case WM_XBUTTONUP:
		view->onMouseUp(event);
		break;
	//case WM_NCLBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:
	//case WM_NCRBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	//case WM_NCMBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	//case WM_NCXBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		view->onDoubleClick(event);
		break;
	default:
		break;
	}
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::handleKeyboardEvent(const KeyboardEvent& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::handleNativeEvent(const MSG& event) {
	bool isMouseEvent = event.message >= WM_MOUSEFIRST && event.message <= WM_MOUSELAST || event.message == WM_MOUSEHOVER || event.message == WM_MOUSELEAVE;
	bool isNonClientMouseEvent = false;
	switch (event.message) {
	case WM_NCMOUSEMOVE:
	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
	case WM_NCMOUSEHOVER:
	case WM_NCMOUSELEAVE:
		isNonClientMouseEvent = true;
		break;
	}
	if (isMouseEvent || isNonClientMouseEvent) {
		POINT pt = { GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) };
		if (isNonClientMouseEvent) {
			// 非客户区的坐标是屏幕坐标
			ScreenToClient(_hWnd, &pt);
		}
		MouseEvent mouse(event, pt.x, pt.y);
		return handleMouseEvent(mouse);
	} else if (event.message >= WM_KEYFIRST && event.message <= WM_KEYLAST) {
		KeyboardEvent keyboard(event);
		return handleKeyboardEvent(keyboard);
	} else {
		switch (event.message) {
		case WM_CREATE: return onCreate(event);
		case WM_DESTROY: return onDestroy(event);
		case WM_NCACTIVATE: return onNcActivate(event);
		case WM_NCCALCSIZE: return onNcCalcSize(event);
		case WM_NCHITTEST: return onNcHitTest(event);
		case WM_GETMINMAXINFO: return onGetMinMaxInfo(event);
		case WM_SYSCOMMAND: return onSysCommand(event);
		case WM_SIZE: return onSize(event);
		case WM_DPICHANGED: return onDpiChanged(event);
		case WM_PAINT: return onPaint(event);
		default: break;
		}
	}
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onCreate(const MSG& event) {
	HWND hWnd = (HWND)handle();
	::SetWindowLong(hWnd, GWL_STYLE, _style->style());
	if (_style->dwmShadow()) {
		DWORD dwValue = 2;
		DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &dwValue, sizeof(dwValue));
		MARGINS margins = { 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea(hWnd, &margins);
	}
	if (_eglWindow) {
		_eglContext = new EGLRenderContextImpl(hWnd);
		_eglContext->load();
		_eglContext->setTarget(_eglWindow);
	}
	if (_windowController) _windowController->onInitWindow();
	setNeedsLayout();
	return 0;
}

std::optional<LRESULT> PlatformWindow::onDestroy(const MSG& event) {
	if (_windowController) _windowController->onDestroyWindow();
	if (_eglContext) {
		_eglContext->setTarget(nullptr);
		_eglContext->unload();
	}
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onNcActivate(const MSG& event) {
	if (::IsIconic(_hWnd)) return std::nullopt;
	return (event.wParam == 0) ? TRUE : FALSE;
}

std::optional<LRESULT> PlatformWindow::onNcCalcSize(const MSG& event) {
	return 0;
}

std::optional<LRESULT> PlatformWindow::onNcHitTest(const MSG& event) {
	POINT pt = { GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) };
	ScreenToClient((HWND)handle(), &pt);

	Size scale = this->scale();
	RefPtr<View> view1 = hitTest(pt.x / scale.width, pt.y / scale.height);
	RefPtr<NCHitTestView> view2 = view1.as<NCHitTestView>();
	while (view2 == nullptr && view1->superview() != nullptr) {
		view1 = view1->superview();
		view2 = view1.as<NCHitTestView>();
	}
	if (view2 == nullptr) return HTCLIENT;

	LRESULT code = view2->code();
	switch (code) {
	case HTLEFT:
	case HTRIGHT:
	case HTTOP:
	case HTTOPLEFT:
	case HTTOPRIGHT:
	case HTBOTTOM:
	case HTBOTTOMLEFT:
	case HTBOTTOMRIGHT:
		return IsZoomed((HWND)handle()) ? HTCLIENT : code;
	default:
		return code;
	}
}

std::optional<LRESULT> PlatformWindow::onGetMinMaxInfo(const MSG& event) {
	MONITORINFO oMonitor = { sizeof(oMonitor) };
	::GetMonitorInfo(::MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);

	RECT rcWork = oMonitor.rcWork;
	RECT rcMonitor = oMonitor.rcMonitor;
	if (!_style->fullscreen()) {
		rcWork.left -= rcMonitor.left;
		rcWork.right -= rcMonitor.left;
		rcWork.top -= rcMonitor.top;
		rcWork.bottom -= rcMonitor.top;
	} else {
		rcWork = rcMonitor;
	}

	LPMINMAXINFO lpMMI = (LPMINMAXINFO)event.lParam;

	lpMMI->ptMaxPosition.x = rcWork.left;
	lpMMI->ptMaxPosition.y = rcWork.top;
	lpMMI->ptMaxSize.x = rcWork.right - rcWork.left;
	lpMMI->ptMaxSize.y = rcWork.bottom - rcWork.top;

	lpMMI->ptMaxTrackSize.x = lpMMI->ptMaxSize.x;
	lpMMI->ptMaxTrackSize.y = lpMMI->ptMaxSize.y;
	lpMMI->ptMinTrackSize.x = (LONG)_minTrackSize.width;
	lpMMI->ptMinTrackSize.y = (LONG)_minTrackSize.height;

	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onSysCommand(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onDpiChanged(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onSize(const MSG& event) {
	_layer->setBounds(Rect(0, 0, LOWORD(event.lParam), HIWORD(event.lParam)));
	setNeedsLayout();
	render([](auto) {});
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onPaint(const MSG& event) {
	layoutIfNeeded();

	PAINTSTRUCT ps;
	if (_eglContext) {
		BeginPaint(_hWnd, &ps);
		EndPaint(_hWnd, &ps);
		if (_layer && _renderLayer) {
			RefPtr<gdiplus::Layer> gdiLayer = _layer.as<gdiplus::Layer>();
			if (gdiLayer != nullptr) {
				gdiLayer->paint(_renderLayer);
			}
		}

		_eglContext->render();
	} else {
		HDC hdc = BeginPaint(_hWnd, &ps);
		if (_layer) _layer.cast<gdiplus::Layer>()->paint(hdc);
		EndPaint(_hWnd, &ps);
	}
	return 0;
}

std::optional<LRESULT> PlatformWindow::onMouseMove(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onMouseHover(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onMouseLeave(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onMouseWheel(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onMouseDown(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onMouseUp(const MSG& event) {
	return std::nullopt;
}

std::optional<LRESULT> PlatformWindow::onDoubleClick(const MSG& event) {
	return std::nullopt;
}

PlatformWindow::EGLWindowImpl::EGLWindowImpl(PlatformWindow* window) : _window(window) {
}

HWND PlatformWindow::EGLWindowImpl::handle() const {
	return (HWND)_window->handle();
}

render::vec2 PlatformWindow::EGLWindowImpl::size() const {
	assert(handle() != nullptr);
	RECT rect = { 0 };
	if (GetClientRect(handle(), &rect)) {
		return render::vec2(rect.right - rect.left, rect.bottom - rect.top);
	}
	return render::vec2(0.0f, 0.0f);
}

PlatformWindow::EGLRenderContextImpl::EGLRenderContextImpl(HWND hWnd) : _hWnd(hWnd), _hDC(GetDC(_hWnd)) {
}

PlatformWindow::EGLRenderContextImpl::~EGLRenderContextImpl() {
	ReleaseDC(_hWnd, _hDC);
}
