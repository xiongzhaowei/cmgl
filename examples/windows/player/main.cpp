#include <sdkddkver.h>
#include <Windows.h>
#include "CMGL.h"

using namespace wheel;
#include "player.h"

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
    SetProcessDPIAware();

    std::wstring http = L"http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
    std::wstring url = L"D:\\迅雷下载\\Guardian Of The Galaxy Volume 3 (2023) ENG HDTC 1080p x264 AAC - HushRips.mp4";
    std::wstring dsv = L"C:\\Users\\SHAREit\\Downloads\\(whatsapp)09977c28a0414b9ba61aee2ae05928ce.mp4.dsv";

    RefPtr<PlayerWindowController> playerWindowController = new PlayerWindowController(url);
    playerWindowController->open(wcstombs(url, CP_UTF8));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
