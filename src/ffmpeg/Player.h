//
// Created by 熊朝伟 on 2023-07-07.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MoviePlayer : public Object {
    RefPtr<MovieThread> _thread;
    RefPtr<MovieSource> _source;
    RefPtr<MovieSourceStream> _audioSource;
    RefPtr<MovieSourceStream> _videoSource;
    RefPtr<AudioRenderer> _audioRenderer;
    RefPtr<VideoRenderer> _videoRenderer;
public:
    static RefPtr<MoviePlayer> file(const std::string& path, RefPtr<MovieThread> thread = nullptr);

    void bind(AVPixelFormat format, std::function<void(RefPtr<Frame>)> render);
    void play(bool state);
    void seek(double time, std::function<void()> callback);
    double duration() const;

    double volume() const;
    void setVolume(double volume);
};

OMP_FFMPEG_NAMESPACE_END
