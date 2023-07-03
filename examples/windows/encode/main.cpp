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

class AudioSplitter : public Transformer<Frame> {
    RefPtr<Consumer<Frame>> _output;
    RefPtr<Frame> _frame;
    int64_t _timestamp;
    int32_t _sampleCount;
    int32_t _sampleTotal;
    int32_t _bytesPerSample;
    bool _planar;
public:
    AudioSplitter(RefPtr<Consumer<Frame>> output, int32_t nb_samples) : _output(output), _sampleTotal(nb_samples), _timestamp(0) {
        assert(output != nullptr);
        assert(nb_samples > 0);
    }
    void copyData(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
        assert(_frame != nullptr);
        if (_sampleCount + count >= _sampleTotal) {
            int32_t copyCount = _sampleTotal - _sampleCount;
            copy(data, offset, copyCount, nb_channels);
            offset += copyCount;
            count -= copyCount;

            _frame->frame()->pts = _timestamp;
            _output->add(_frame);

            if (count > 0) {
                _frame = Frame::alloc((AVSampleFormat)_frame->frame()->format, _frame->frame()->ch_layout, _sampleTotal, _frame->frame()->sample_rate);
                _sampleCount = 0;
                copyData(data, offset, count, nb_channels);
            } else {
                _frame = nullptr;
            }
        } else {
            copy(data, offset, count, nb_channels);
        }
    }
    void copy(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
        int32_t bytesPerSample = _bytesPerSample;
        if (!_planar) {
            bytesPerSample *= nb_channels;
            nb_channels = 1;
        }
        for (int32_t channel = 0; channel < nb_channels; channel++) {
            memcpy(_frame->frame()->extended_data[channel] + _sampleCount * bytesPerSample, data[channel] + offset * bytesPerSample, count * bytesPerSample);
        }
        _timestamp += count;
        _sampleCount += count;
    }
    void add(RefPtr<Frame> frame) override {
        if (frame == nullptr) return;
        if (frame->frame() == nullptr) return;

        if (_frame == nullptr) {
            AVSampleFormat format = (AVSampleFormat)frame->frame()->format;
            _frame = Frame::alloc(format, frame->frame()->ch_layout, _sampleTotal, frame->frame()->sample_rate);
            _planar = av_sample_fmt_is_planar(format);
            _bytesPerSample = av_get_bytes_per_sample(format);
            _sampleCount = 0;
        } else {
            assert(av_channel_layout_compare(&_frame->frame()->ch_layout, &frame->frame()->ch_layout) == 0);
            assert(_frame->frame()->format == frame->frame()->format);
            assert(_frame->frame()->sample_rate == frame->frame()->sample_rate);
        }
        copyData(frame->frame()->extended_data, 0, frame->frame()->nb_samples, frame->frame()->ch_layout.nb_channels);
    }
    void addError() override {
        _output->addError();
    }
    void close() override {
        _output->close();
    }
};

#ifdef main
#undef main
#endif
int main() {
//int APIENTRY wWinMain(
//    _In_ HINSTANCE instance,
//    _In_opt_ HINSTANCE prev,
//    _In_ wchar_t *command_line,
//    _In_ int show_command
//) {
    RefPtr<Thread> thread = new Thread;
    std::string url = wcstombs(L"D:\\迅雷下载\\Guardian Of The Galaxy Volume 3 (2023) ENG HDTC 1080p x264 AAC - HushRips.mp4", CP_UTF8);
    std::string output = wcstombs(L"D:\\迅雷下载\\test.asf", CP_UTF8);

    RefPtr<ffmpeg::MovieSource> source = new ffmpeg::MovieSource();
    RefPtr<ffmpeg::MovieTarget> target = MovieTarget::from(nullptr, output.c_str());

    RefPtr<ffmpeg::MovieEncoder> audioTarget = target->audio(160616, AV_SAMPLE_FMT_FLTP, 44100, { AV_CHANNEL_ORDER_NATIVE, 2, AV_CH_LAYOUT_STEREO });
    RefPtr<ffmpeg::MovieEncoder> videoTarget = target->video(4000000, AV_PIX_FMT_YUV420P, 25, 1920, 1080, 25, 0);

    target->openFile(output);
    target->writeHeader();

    if (source->open(url)) {
        RefPtr<Stream<Frame>> videoSource = source->stream(AVMEDIA_TYPE_VIDEO)->convert(videoTarget->context()->pix_fmt);
        RefPtr<Stream<Frame>> audioSource = source->stream(AVMEDIA_TYPE_AUDIO)->convert(
            audioTarget->context()->sample_fmt,
            audioTarget->context()->ch_layout,
            audioTarget->context()->sample_rate
        )->transform<AudioSplitter>(audioTarget->context()->frame_size);
        videoSource = videoSource->convert<ffmpeg::Frame>([](RefPtr<ffmpeg::Frame> frame) {
            frame->frame()->pts /= 3600;
            printf("video pts: %lld\n", frame->frame()->pts);
            return frame;
        });
        audioSource->listen(audioTarget);
        videoSource->listen(videoTarget);

        for (int i = 0; i < 1000; i++) {
            source->read();
        }

    }

    target->writeTrailer();
    target->closeFile();

    return 0;
}
