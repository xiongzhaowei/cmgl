//
//  defines.h
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#include "../defines.h"
#include <queue>

#define OMP_THREAD_NAMESPACE_BEGIN    OMP_NAMESPACE_BEGIN namespace thread {
#define OMP_THREAD_NAMESPACE_END      } OMP_NAMESPACE_END
#define OMP_THREAD_USING_NAMESPACE    OMP_USING_NAMESPACE using namespace OMP_NAMESPACE_PREFIX thread;

#include "WaitableEvent.h"
#include "Thread.h"
#include "Future.h"
