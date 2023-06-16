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

using namespace wheel;

int APIENTRY wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE prev,
    _In_ wchar_t *command_line,
    _In_ int show_command
) {
    RefPtr<Thread> thread = new Thread;
    std::string url = wcstombs(L"D:\\迅雷下载\\Guardian Of The Galaxy Volume 3 (2023) ENG HDTC 1080p x264 AAC - HushRips.mp4", CP_UTF8);
    std::string output = wcstombs(L"D:\\迅雷下载\\test.mp4", CP_UTF8);

    RefPtr<ffmpeg::FileSource> source = new ffmpeg::FileSource(thread);
    RefPtr<ffmpeg::FileTarget> target = ffmpeg::FileTarget::from(nullptr, output.c_str());

    RefPtr<ffmpeg::FileTargetStream> audioTarget = ffmpeg::FileTargetStream::audio(target);
    RefPtr<ffmpeg::FileTargetStream> videoTarget = ffmpeg::FileTargetStream::video(target);

    audioTarget->open(AV_SAMPLE_FMT_S16P, 160616, 44100, { AV_CHANNEL_ORDER_NATIVE, 2, AV_CH_LAYOUT_STEREO });
    videoTarget->open(AV_PIX_FMT_YUV420P, 4000000, 25, 1920, 1080, 25, 0);

    target->openFile(output);
    target->writeHeader();

    if (source->open(url)) {
        RefPtr<ffmpeg::FileSourceStream> audioSource = ffmpeg::FileSourceStream::audio(source);
        RefPtr<ffmpeg::FileSourceStream> videoSource = ffmpeg::FileSourceStream::video(source);
        source->listen(audioSource);
        source->listen(videoSource);

        RefPtr<ffmpeg::Stream<ffmpeg::Frame>> audioFilter = audioSource->convert<ffmpeg::Frame>([](RefPtr<ffmpeg::Frame> frame) {
            printf("audio pts: %lld\n", frame->frame()->pts);
            return frame;
        });
        audioFilter->listen(audioTarget);

        RefPtr<ffmpeg::Stream<ffmpeg::Frame>> videoFilter = videoSource->convert<ffmpeg::Frame>([](RefPtr<ffmpeg::Frame> frame) {
            frame->frame()->pts /= 3600;
            printf("video pts: %lld\n", frame->frame()->pts);
            return frame;
        });
        videoFilter->listen(videoTarget);

        for (int i = 0; i < 1000; i++) {
            source->read();
        }

    }

    target->writeTrailer();
    target->closeFile();

    return 0;
}
