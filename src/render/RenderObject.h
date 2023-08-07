//
//  RenderObject.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct Event {};

template <typename Event = void, typename T = void>
struct EventHandler;

template <>
struct EventHandler<void, void> : public Object {
    virtual bool equals(const std::type_index &type) = 0;
};

template <typename Event>
struct EventHandler<Event, void> : public EventHandler<> {
    virtual void call(const Event &event) = 0;
};

template <typename Event, typename T>
struct EventHandler : public EventHandler<Event> {
public:
    EventHandler(T *instance, void (T::*callback)(const Event &))
    : _instance(instance), _callback(callback) {}

    void call(const Event &event) override {
        (_instance->*_callback)(event);
    }

    bool equals(const std::type_index &type) override {
        return std::type_index(typeid(T)) == type;
    }

    bool equals(T *instance, void (T::*callback)(const Event &)) {
        if (_instance != instance) return false;
        if (_callback != callback) return false;
        return true;
    }
private:
    T *_instance;
    void (T::*_callback)(const Event &);
};

class EventController {
    std::unordered_map<std::type_index, std::vector<RefPtr<EventHandler<>>>> subscribers;
public:
    template <typename Event, typename T>
    void subscribe(T *instance, void(T::*callback)(const Event &event)) {
        subscribers[typeid(Event)].push_back(std::make_shared<EventHandler<Event, T>>(instance, callback));
    }

    template <typename Event, typename T>
    void unsubscribe(T *instance, void(T::*callback)(const Event &event)) {
        std::vector<RefPtr<EventHandler<>>> &handlers = subscribers[typeid(Event)];
        for (auto it = handlers.begin(); it != handlers.end(); it++) {
            RefPtr<EventHandler<>> handler = *it;
            if (handler == nullptr) {
                continue;
            }
            if (!handler->equals(typeid(T))) {
                continue;
            }
            if (std::static_pointer_cast<EventHandler<Event, T>>(handler)->equals(instance, callback)) {
                handlers.erase(it);
                break;
            }
        }
    }

    template<typename Event>
    void publish(const Event &event) {
        for (RefPtr<EventHandler<>> handler : subscribers[typeid(Event)]) {
            if (handler != nullptr) {
                handler.cast<EventHandler<Event>>()->call(event);
            }
        }
    }
};

struct TouchEvent : public Event {
    float x;
    float y;
};

struct TapGestureEvent : TouchEvent {};
struct LongTapGestureEvent : TouchEvent {};
struct PanGestureBeganEvent : TouchEvent {};
struct PanGestureMovedEvent : TouchEvent {};
struct PanGestureEndedEvent : TouchEvent {};

class RenderObject : public RenderSource {
public:
    vec4 backgroundColor() const { return vec4(0, 0, 0, 0); }
    vec2 size() const override;
    bool visible() const override { return true; }

    void load(RefPtr<RenderContext> context) override = 0;
    void unload(RefPtr<RenderContext> context) override = 0;
    void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) override = 0;

    // 注意：此方法在主线程执行
    virtual bool hitTest(float x, float y);
    virtual void resize(float width, float height);

    static bool init();
    // static std::string name() = 0;
    // static std::shared_ptr<RenderObject> make(const std::string &json) = 0;
    
    EventController &events();
protected:
    vec2 _size = vec2(0, 0);
    EventController _events;
};

OMP_RENDER_NAMESPACE_END
