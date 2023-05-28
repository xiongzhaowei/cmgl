//
//  RenderFactory.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

RefPtr<RenderObject> RenderFactory::make(const std::string &name, const std::string &json) {
    auto iterator = factories().find(name);
    if (iterator == factories().end()) {
        return nullptr;
    }
    RefPtr<RenderFactory> factory = iterator->second;
    if (!factory) {
        return nullptr;
    }
    return factory->make(json);
}

std::unordered_map<std::string, RefPtr<RenderFactory>> &RenderFactory::factories() {
    static std::unordered_map<std::string, RefPtr<RenderFactory>> factories;
    return factories;
}
