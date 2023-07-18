//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MovieThread : public Thread {
public:
    class ScheduledTask;

    MovieThread() = default;

    void run() override;
    void add(RefPtr<MovieSource> movie);
    void remove(RefPtr<MovieSource> movie);
    RefPtr<ScheduledTask> schedule(double timeInterval, const std::function<bool()>& callback);
    void cancel(RefPtr<ScheduledTask> task);
private:
    bool available(std::list<RefPtr<MovieSource>>& list) const;

    std::list<RefPtr<MovieSource>> _movies;
    std::list<RefPtr<ScheduledTask>> _schedules;
};

class MovieThread::ScheduledTask : public Object {
    std::chrono::steady_clock::time_point _schedule;
    std::chrono::nanoseconds _timeInterval;
    std::function<bool()> _callback;
public:
    ScheduledTask(double timeInterval, const std::function<bool()>& callback);
    std::chrono::steady_clock::time_point next();
    bool available() const;
    bool exec();
};

OMP_FFMPEG_NAMESPACE_END
