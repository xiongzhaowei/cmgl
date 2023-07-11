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
    void seek(double time, std::function<void(bool)> callback);

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
