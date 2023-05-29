#include <sdkddkver.h>
#include <Windows.h>
#include "CMGL.h"


const std::wstring mbstowcs(const std::string& str, uint32_t codePage) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), wstr.data(), size);
    return wstr;
}

const std::string wcstombs(const std::wstring& wstr, uint32_t codePage) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), str.data(), size, NULL, NULL);
    return str;
}

int APIENTRY wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE prev,
    _In_ wchar_t *command_line,
    _In_ int show_command
) {
    wheel::RefPtr<wheel::ui::windows::PlatformWindow> w = new wheel::ui::windows::PlatformWindow(wheel::ui::windows::PlatformWindow::Style::create(TEXT("Player"), TEXT(""), true, true, true, true, false, false));
    if (w->create(1024, 768, nullptr, true)) {
        wheel::RefPtr<wheel::render::YUV420PVideoSource> source = new wheel::render::YUV420PVideoSource;
        w->render([w, source](wheel::render::egl::EGLRenderContext* context) {
            context->addSource(source);
        });
        wheel::RefPtr<wheel::ui::windows::NCHitTestView> caption = new wheel::ui::windows::NCHitTestView;
        caption->setFrame(wheel::ui::Rect(0, 0, 1024, 32));
        caption->setBackgroundColor(wheel::ui::Color(0xFFFFFFFF));
        caption->setCode(HTCAPTION);
        w->addSubview(caption);
        w->showWindow(SW_SHOW);

        std::wstring url = L"D:\\迅雷下载\\Guardian Of The Galaxy Volume 3 (2023) ENG HDTC 1080p x264 AAC - HushRips.mp4";
        wheel::RefPtr<wheel::ffmpeg::Movie> movie = wheel::ffmpeg::Movie::from(wcstombs(url, CP_UTF8), source, [w]() {
            PostMessage(w->handle(), WM_USER, 0, 0); // 通知刷新画面
        });
        movie->start();
        movie->play(true);
        movie->seek(600, []() {
            printf("done");
        });
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_USER) {
            w->render([](wheel::render::egl::EGLRenderContext* context) {});
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
