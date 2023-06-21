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

template <typename T, bool planar>
class AudioSplitterT : public Transformer<Frame> {
    RefPtr<Consumer<Frame>> _output;
    AVSampleFormat _format;
    AVChannelLayout _layout;
    RefPtr<Frame> _frame;
    int32_t _sampleCount;
    int32_t _sampleTotal;
    int64_t _timestamp;
public:
    AudioSplitterT(RefPtr<Consumer<Frame>> output, AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples) : _output(output), _format(format), _layout(ch_layout), _sampleTotal(nb_samples), _timestamp(0) {
        assert(output != nullptr);
        assert(format != AV_SAMPLE_FMT_NONE);
        assert(ch_layout.nb_channels > 0);
        assert(nb_samples > 0);
    }
    void copyData(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
        if (_frame == nullptr) {
            _frame = Frame::alloc(_format, _layout, _sampleTotal);
            _sampleCount = 0;
        }
        if (_sampleCount + count >= _sampleTotal) {
            int32_t copyCount = _sampleTotal - _sampleCount;
            copy(data, offset, copyCount, nb_channels);
            offset += copyCount;
            count -= copyCount;

            _frame->frame()->pts = _timestamp;
            _output->add(_frame);
            _frame = nullptr;

            if (count > 0) {
                copyData(data, offset, count, nb_channels);
            }
        } else {
            copy(data, offset, count, nb_channels);
        }
    }
    void copy(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
        if (planar) {
            for (int channel = 0; channel < nb_channels; channel++) {
                memcpy(_frame->frame()->extended_data[channel] + _sampleCount * sizeof(T), data[channel] + offset * sizeof(T), count * sizeof(T));
            }
        } else {
            memcpy(_frame->frame()->extended_data[0] + _sampleCount * nb_channels * sizeof(T), data[0] + offset * nb_channels * sizeof(T), count * nb_channels * sizeof(T));
        }
        _timestamp += count;
        _sampleCount += count;
    }
    void add(RefPtr<Frame> frame) override {
        assert(av_channel_layout_compare(&_layout, &frame->frame()->ch_layout) == 0);
        assert(_format == (AVSampleFormat)frame->frame()->format);

        copyData(frame->frame()->extended_data, 0, frame->frame()->nb_samples, frame->frame()->ch_layout.nb_channels);
    }
    void addError() override {
        _output->addError();
    }
    void close() override {
        _output->close();
    }
};

class AudioSplitter : public Transformer<Frame> {
    RefPtr<Consumer<Frame>> _output;
public:
    AudioSplitter(RefPtr<Consumer<Frame>> output, AVSampleFormat format, AVChannelLayout ch_layout, int32_t frame_size) {
        _output = build(output, format, ch_layout, frame_size);
    }
    AudioSplitter(RefPtr<Consumer<Frame>> output, AVCodecContext* context) {
        _output = build(output, context->sample_fmt, context->ch_layout, context->frame_size);
    }
    void add(RefPtr<Frame> frame) override {
        _output->add(frame);
    }
    void addError() override {
        _output->addError();
    }
    void close() override {
        _output->close();
    }
    static RefPtr<Transformer<Frame>> build(RefPtr<Consumer<Frame>> output, AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples) {
        switch (format) {
        case AV_SAMPLE_FMT_U8:
            return new AudioSplitterT<uint8_t, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S16:
            return new AudioSplitterT<int16_t, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S32:
            return new AudioSplitterT<int32_t, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_FLT:
            return new AudioSplitterT<float, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_DBL:
            return new AudioSplitterT<double, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_U8P:
            return new AudioSplitterT<uint8_t, true>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S16P:
            return new AudioSplitterT<int16_t, true>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S32P:
            return new AudioSplitterT<int32_t, true>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_FLTP:
            return new AudioSplitterT<float, true>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_DBLP:
            return new AudioSplitterT<double, true>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S64:
            return new AudioSplitterT<int64_t, false>(output, format, ch_layout, nb_samples);
        case AV_SAMPLE_FMT_S64P:
            return new AudioSplitterT<int64_t, true>(output, format, ch_layout, nb_samples);
        default:
            return nullptr;
        }
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
    std::string output = wcstombs(L"D:\\迅雷下载\\test.mpg", CP_UTF8);

    RefPtr<ffmpeg::FileSource> source = new ffmpeg::FileSource(thread);
    RefPtr<ffmpeg::FileTarget> target = ffmpeg::FileTarget::from(nullptr, output.c_str());

    RefPtr<ffmpeg::FileTargetStream> audioTarget = ffmpeg::FileTargetStream::audio(target);
    RefPtr<ffmpeg::FileTargetStream> videoTarget = ffmpeg::FileTargetStream::video(target);

    audioTarget->open(AV_SAMPLE_FMT_FLTP, 160616, 44100, { AV_CHANNEL_ORDER_NATIVE, 2, AV_CH_LAYOUT_STEREO });
    videoTarget->open(AV_PIX_FMT_YUV420P, 4000000, 25, 1920, 1080, 25, 0);

    target->openFile(output);
    target->writeHeader();

    if (source->open(url)) {
        RefPtr<ffmpeg::FileSourceStream> videoSource = ffmpeg::FileSourceStream::video(source);
        RefPtr<ffmpeg::FileSourceStream> audioSource = ffmpeg::FileSourceStream::audio(source);
        source->listen(audioSource);
        source->listen(videoSource);

        //RefPtr<ffmpeg::Stream<ffmpeg::Frame>> audioFilter = audioSource->convert<ffmpeg::Frame>([](RefPtr<ffmpeg::Frame> frame) {
        //    printf("audio pts: %lld\n", frame->frame()->pts);
        //    return frame;
        //});
        RefPtr<Converter> converter = ffmpeg::AudioConverter::create(audioSource->stream(), audioTarget->context()->sample_fmt, audioTarget->context()->ch_layout);
        RefPtr<ffmpeg::Stream<ffmpeg::Frame>> audioFilter = audioSource->convert<ffmpeg::Frame>([converter](RefPtr<ffmpeg::Frame> frame) {
            return converter->convert(frame);
        })->transform<AudioSplitter>(audioTarget->context());
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
