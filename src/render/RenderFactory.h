//
//  RenderFactory.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

class RenderFactory : public Object {
public:
    virtual RefPtr<RenderObject> make(const std::string &json) = 0;
    
    template <typename T>
    class Factory;
    
    template <typename T>
    static bool init();
    static RefPtr<RenderObject> make(const std::string &name, const std::string &json);
protected:
    static std::unordered_map<std::string, RefPtr<RenderFactory>> &factories();
};

template <typename T>
class RenderFactory::Factory : public RenderFactory {
public:
    RefPtr<RenderObject> make(const std::string &json) override;
    
    static const bool registed;
    static bool load() {
        static_assert(std::is_base_of<RenderObject, T>::value, "");
        RenderFactory::factories()[T::name()] = new RenderFactory::Factory<T>();
        return true;
    }
    static bool init();
};

template <typename T>
bool RenderFactory::init() { return Factory<T>::init(); }

template <typename T>
const bool RenderFactory::Factory<T>::registed = false;

OMP_RENDER_NAMESPACE_END

#define OMPDefineRenderObject(RenderObjectClass) \
OMP_RENDER_NAMESPACE_BEGIN \
template <> \
const bool RenderFactory::Factory<RenderObjectClass>::registed = \
RenderFactory::Factory<RenderObjectClass>::load(); \
template <> \
RefPtr<RenderObject> RenderFactory::Factory<RenderObjectClass>::make(const std::string &json) { \
    return RenderObjectClass::make(json); \
} \
template <> \
bool RenderFactory::Factory<RenderObjectClass>::init() { \
    return RenderObjectClass::init(); \
} \
OMP_RENDER_NAMESPACE_END
