#include <sdkddkver.h>
#include <Windows.h>
#include "CMGL.h"

using namespace wheel;

struct TestConsumer : public ffmpeg::Consumer<Object> {
    std::vector<RefPtr<Object>> list;
    int32_t error = 0;
    bool closed = false;
    void add(RefPtr<Object> object) {
        list.push_back(object);
    }
    void addError() {
        error++;
    }
    void close() {
        closed = true;
    }
};

int APIENTRY wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE prev,
    _In_ wchar_t *command_line,
    _In_ int show_command
) {
    RefPtr<Thread> thread = new Thread;
    RefPtr<ffmpeg::StreamController<Object>> streamController = ffmpeg::StreamController<Object>::sync(thread);
    RefPtr<TestConsumer> consumer = new TestConsumer;
    WeakPtr<ffmpeg::StreamSubscription<Object>> subscription = streamController->stream()->listen(consumer);

    RefPtr<Object> obj1 = new Object;
    RefPtr<Object> obj2 = new Object;
    streamController->add(obj1);
    streamController->add(obj2);
    assert(consumer->list.size() == 0);

    thread->runOnThread([=]() {
        assert(consumer->list.size() == 2);
        streamController->addError();
        streamController->addError();
        assert(consumer->error == 0);
        thread->runOnThread([=]() {
            assert(consumer->error == 2);
            streamController->close();
            assert(consumer->closed == false);
            thread->runOnThread([=]() {
                assert(consumer->closed == true);
                assert(streamController->isClosed());
                thread->stop();
            });
        });
    });
    thread->run();
    assert(subscription == nullptr);

    return 0;
}
