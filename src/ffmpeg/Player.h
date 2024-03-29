﻿//
// Created by 熊朝伟 on 2023-07-07.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MoviePlayer : public Object {
    const uint32_t _maxCacheFrames = 3;
    RefPtr<MovieThread> _thread;
    RefPtr<MovieSource> _source;
    RefPtr<MovieSourceStream> _audioSource;
    RefPtr<MovieSourceStream> _videoSource;
    RefPtr<AudioRenderer> _audioRenderer;
    RefPtr<VideoRenderer> _videoRenderer;
    RefPtr<MoviePlayer> _self;
    bool _isPlaying = false;
public:
    static bool supported(const std::string& path);
    static RefPtr<MoviePlayer> file(const std::string& path, RefPtr<MovieThread> thread = nullptr);

    ~MoviePlayer();

    bool open(RefPtr<MovieFile> file, RefPtr<MovieThread> thread = nullptr);
    bool open(const std::string& path, RefPtr<MovieThread> thread = nullptr);
    void asyncOpen(const std::string& path, const std::function<void(bool)>& callback);
    void asyncOpen(const std::string& path, RefPtr<MovieThread> thread, const std::function<void(bool)>& callback);
    void close();

    bool ready() const;
    bool isPlaying() const;

    int32_t width() const;
    int32_t height() const;

    /**
     * @brief 绑定UI渲染，按照format指定的格式输出所有视频帧。使用者应在render回调中通知画面刷新。
     * @param format 
     * @param render 
    */
    void bind(AVPixelFormat format, std::function<void(RefPtr<Frame>)> render);

    /**
     * @brief 播放视频
     * @param state 
    */
    void play(bool state);

    /**
     * @brief 跳到指定位置播放
     * @param time 需要seek的目标时间，单位为秒。
     * @param callback 完成后第一时间的回调。
    */
    void seek(double time, std::function<void(bool)> callback = std::function<void(bool)>());

    /**
     * @brief 当前播放时间
     * @return 
    */
    double time() const;

    /**
     * @brief 播放总时长
     * @return 
    */
    double duration() const;

    /**
     * @brief 当前音量
    */
    double volume() const;

    /**
     * @brief 设置音量
    */
    void setVolume(double volume);
};

OMP_FFMPEG_NAMESPACE_END
