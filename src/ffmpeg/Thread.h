//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MovieThread : public Thread {
public:
    MovieThread() = default;

    void run() override;
    void add(RefPtr<MovieSource> movie);
    void remove(RefPtr<MovieSource> movie);
private:
    bool available(std::list<RefPtr<MovieSource>>& list) const;

    std::list<RefPtr<MovieSource>> _movies;
};

OMP_FFMPEG_NAMESPACE_END
