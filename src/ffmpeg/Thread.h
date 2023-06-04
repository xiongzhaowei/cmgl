//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MovieThread : public Thread {
    friend class Movie;

    std::list<RefPtr<Movie>> _movies;
public:
    MovieThread() = default;

    void run() override;
    void add(RefPtr<Movie> movie);
    void remove(RefPtr<Movie> movie);

    RefPtr<Movie> movie(const std::string& url, AVPixelFormat pixel, AVSampleFormat sample);
private:
    bool detect(std::list<RefPtr<Movie>>& list);
};

class DecodeThread : public Thread {
public:
    DecodeThread() = default;

    void run() override;
    void add(RefPtr<Decoder> decoder);
    void remove(RefPtr<Decoder> decoder);

    RefPtr<Decoder> decode(const std::string& url, AVPixelFormat pixel, AVSampleFormat sample);
private:
    bool available(std::list<RefPtr<Decoder>>& list);

    std::list<RefPtr<Decoder>> _decoders;
};

OMP_FFMPEG_NAMESPACE_END
