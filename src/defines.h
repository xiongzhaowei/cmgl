//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

#ifndef DEFINE_OMP_NAMESPACE
    #define DEFINE_OMP_NAMESPACE    1
#endif

#if DEFINE_OMP_NAMESPACE && !defined(OMP_NAMESPACE_NAME)
    #define OMP_NAMESPACE_NAME      wheel
#endif

#ifdef OMP_NAMESPACE_NAME
    #define OMP_NAMESPACE_BEGIN     namespace OMP_NAMESPACE_NAME {
    #define OMP_NAMESPACE_END       }
    #define OMP_NAMESPACE_PREFIX    OMP_NAMESPACE_NAME ::
    #define OMP_USING_NAMESPACE     using namespace OMP_NAMESPACE_NAME;
#else
    #define OMP_NAMESPACE_BEGIN
    #define OMP_NAMESPACE_END
    #define OMP_NAMESPACE_PREFIX
    #define OMP_USING_NAMESPACE
#endif

#ifdef __ANDROID__
    #include <jni.h>
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #include <objc/objc.h>
#endif

#if defined(__cplusplus)
#include <stdint.h>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <typeindex>
#include <functional>
#include <unordered_map>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "RefPtr.h"
#include "Object.h"
#include "WeakPtr.h"
#include "WaitableEvent.h"
#include "Thread.h"
#include "Future.h"
#endif
