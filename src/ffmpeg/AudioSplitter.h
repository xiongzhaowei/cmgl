//
// Created by 熊朝伟 on 2023-07-04.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class AudioSplitter : public StreamConsumer<Frame> {
    RefPtr<StreamConsumer<Frame>> _output;
    RefPtr<Frame> _frame;
    int64_t _timestamp = 0;
    int32_t _sampleCount;
    int32_t _sampleTotal;
    int32_t _bytesPerSample = 0;
    bool _planar = false;
public:
    typedef Frame Target;

    AudioSplitter(RefPtr<StreamConsumer<Frame>> output, int32_t nb_samples);
    void copyData(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels);
    void copy(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels);
    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;
    bool available() const override;
};

OMP_FFMPEG_NAMESPACE_END
