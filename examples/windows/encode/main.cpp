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

template <typename T>
class AudioSplitter : public Transformer<Frame> {
    RefPtr<Consumer<Frame>> _output;
    AVSampleFormat _format;
    AVChannelLayout _ch_layout;
    bool _planar;
    int32_t _nb_samples;
    int32_t _pts = 0;
    size_t _buffer_size = 0;
    std::vector<std::vector<T>> _buffers;
public:
    AudioSplitter(RefPtr<Consumer<Frame>> output, AVSampleFormat format, AVChannelLayout ch_layout, bool planar, int32_t nb_samples) : _output(output), _format(format), _ch_layout(ch_layout), _planar(planar), _nb_samples(nb_samples) {
        assert(output != nullptr);
        assert(format != AV_SAMPLE_FMT_NONE);
        assert(ch_layout.nb_channels > 0);
        assert(nb_samples > 0);
        if (planar) {
            _buffers.resize(ch_layout.nb_channels);
            for (std::vector<T>& buffer : _buffers) {
                buffer.reserve(_nb_samples);
            }
        } else {
            _buffers.resize(1);
            _buffers.back().reserve(nb_samples * ch_layout.nb_channels);
        }
    }
    void add(RefPtr<Frame> frame) override {
        assert(av_channel_layout_compare(&_ch_layout, &frame->frame()->ch_layout) == 0);
        assert(_format == (AVSampleFormat)frame->frame()->format);

        size_t nb_samples = frame->frame()->nb_samples;
        if (_buffer_size == 0 && nb_samples == _nb_samples) {
            _output->add(frame);
            return;
        }

        int32_t nb_channels = _ch_layout.nb_channels;
        if (_buffer_size + nb_samples >= _nb_samples) {
            RefPtr<Frame> output = Frame::alloc(_format, _ch_layout, _nb_samples);

            size_t offset = _nb_samples - _buffer_size;
            if (_planar) {
                for (int channel = 0; channel < nb_channels; channel++) {
                    assert(_buffers[channel].size() == _buffer_size);

                    memcpy(output->frame()->extended_data[channel], _buffers[channel].data(), _buffer_size * sizeof(T));
                    memcpy(output->frame()->extended_data[channel] + _buffer_size, frame->frame()->extended_data[channel], offset * sizeof(T));
                }
            } else {
                memcpy(output->frame()->extended_data[0], _buffers[0].data(), _buffer_size * nb_channels * sizeof(T));
                memcpy(output->frame()->extended_data[0] + _buffer_size * nb_channels, frame->frame()->extended_data[0], offset * nb_channels * sizeof(T));
            }
            _output->add(output);
            while (nb_samples - offset > _nb_samples) {
                output = Frame::alloc(_format, _ch_layout, _nb_samples);
                if (_planar) {
                    for (int channel = 0; channel < nb_channels; channel++) {
                        memcpy(output->frame()->extended_data[channel], frame->frame()->extended_data[channel] + offset, _nb_samples * sizeof(T));
                    }
                } else {
                    memcpy(output->frame()->extended_data[0], frame->frame()->extended_data[0] + offset * nb_channels, _nb_samples * nb_channels * sizeof(T));
                }
                _output->add(output);
                offset += _nb_samples;
            }
            if (nb_samples - offset > 0) {
                _buffer_size = nb_samples - offset;
                if (_planar) {
                    for (int channel = 0; channel < nb_channels; channel++) {
                        _buffers[channel].resize(_buffer_size);
                        memcpy(_buffers[channel].data(), frame->frame()->extended_data[channel] + offset, _buffer_size * sizeof(T));
                    }
                } else {
                    _buffers[0].resize(_buffer_size * nb_channels);
                    memcpy(_buffers[0].data(), frame->frame()->extended_data[0] + offset * nb_channels, _buffer_size * nb_channels * sizeof(T));
                }
            }
        } else {
            if (_planar) {
                for (int channel = 0; channel < nb_channels; channel++) {
                    _buffers[channel].resize(_buffer_size + nb_samples);
                    memcpy(_buffers[channel].data() + _buffer_size, frame->frame()->extended_data[channel], nb_samples * sizeof(T));
                }
            } else {
                size_t size = _buffer_size * nb_channels;
                _buffers[0].resize(size);
                memcpy(_buffers[0].data() + size, frame->frame()->extended_data[0], nb_samples * nb_channels * sizeof(T));
            }
            _buffer_size += nb_samples;
        }
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
        })->transform<AudioSplitter<int16_t>>(
            audioTarget->context()->sample_fmt,
            audioTarget->context()->ch_layout,
            false,
            audioTarget->context()->frame_size
        );
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
