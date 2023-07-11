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
using namespace wheel::ffmpeg;

int main() {
//int APIENTRY wWinMain(
//    _In_ HINSTANCE instance,
//    _In_opt_ HINSTANCE prev,
//    _In_ wchar_t *command_line,
//    _In_ int show_command
//) {
    RefPtr<MovieThread> thread = new MovieThread;
    thread->start();
    std::string url = wcstombs(L"D:\\迅雷下载\\Guardian Of The Galaxy Volume 3 (2023) ENG HDTC 1080p x264 AAC - HushRips.mp4", CP_UTF8);
    std::string output = wcstombs(L"D:\\迅雷下载\\test.mp4", CP_UTF8);

    RefPtr<ffmpeg::MovieSource> source = new ffmpeg::MovieSource();
    RefPtr<ffmpeg::MovieTarget> target = MovieTarget::from(nullptr, output.c_str());

    RefPtr<ffmpeg::MovieEncoder> audioTarget = target->encoder(160616, AV_SAMPLE_FMT_FLTP, 44100, { AV_CHANNEL_ORDER_NATIVE, 2, AV_CH_LAYOUT_STEREO });
    RefPtr<ffmpeg::MovieEncoder> videoTarget = target->encoder(4000000, AV_PIX_FMT_YUV420P, 25, 1920, 1080, 25, 0);

    target->openFile(output);
    target->writeHeader();

    if (source->open(url)) {
        RefPtr<Stream<Frame>> videoSource = MovieSourceStream::video(source)->convert(videoTarget->context()->pix_fmt);
        RefPtr<Stream<Frame>> audioSource = MovieSourceStream::audio(source)->convert(
            audioTarget->context()->sample_fmt,
            audioTarget->context()->ch_layout,
            audioTarget->context()->sample_rate
        )->transform<AudioSplitter>(audioTarget->context()->frame_size);
        videoSource = videoSource->convert<ffmpeg::Frame>([](RefPtr<ffmpeg::Frame> frame) {
            frame->frame()->pts /= 3600;
            printf("video pts: %lld\n", frame->frame()->pts);
            return frame;
        });
        //audioSource->listen(audioTarget);
        //videoSource->listen(videoTarget);
        audioSource->listen(new EmptyTarget);
        videoSource->listen(new EmptyTarget);

        //for (int i = 0; i < 1000; i++) {
        //    if (source->available()) {
        //        source->read();
        //    }
        //}

    }

    thread->add(source);
    Sleep(60000);
    thread->remove(source);
    Sleep(500);

    target->writeTrailer();
    target->closeFile();

    return 0;
}
