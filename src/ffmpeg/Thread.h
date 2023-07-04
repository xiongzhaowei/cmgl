//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class DecodeThread : public Thread {
public:
    DecodeThread() = default;

    void run() override;
    void add(RefPtr<Decoder> decoder);
    void remove(RefPtr<Decoder> decoder);

    RefPtr<Decoder> decode(const std::string& url);
private:
    bool available(std::list<RefPtr<Decoder>>& list);

    std::list<RefPtr<Decoder>> _decoders;
};

class MovieThread : public Thread {
public:
    MovieThread() = default;

    void run() override;
    void add(RefPtr<MovieSource> movie);
    void remove(RefPtr<MovieSource> movie);
private:
    bool available(std::list<RefPtr<MovieSource>>& list);

    std::list<RefPtr<MovieSource>> _movies;
};

OMP_FFMPEG_NAMESPACE_END
