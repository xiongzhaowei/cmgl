//
//  defines.h
//  omplayer
//
//  Created by 熊朝伟 on 2023/5/16.
//

#pragma once

#include "../defines.h"
#include "../render/defines.h"

#define OMP_FFMPEG_NAMESPACE_BEGIN    OMP_NAMESPACE_BEGIN namespace ffmpeg {
#define OMP_FFMPEG_NAMESPACE_END      } OMP_NAMESPACE_END
#define OMP_FFMPEG_NAMESPACE_PREFIX   OMP_NAMESPACE_PREFIX ffmpeg ::
#define OMP_FFMPEG_USING_NAMESPACE    OMP_USING_NAMESPACE using namespace OMP_NAMESPACE_PREFIX ffmpeg;

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#include "Frame.h"
#include "Thread.h"
#include "Movie.h"
