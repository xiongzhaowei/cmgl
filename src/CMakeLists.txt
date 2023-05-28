
project(cmgl LANGUAGES C)

add_library(${PROJECT_NAME} STATIC)

add_subdirectory("ffmpeg")
add_subdirectory("render")
add_subdirectory("thread")
add_subdirectory("ui")
target_sources(${PROJECT_NAME} PRIVATE
    CMGL.h
    defines.h
    Object.cpp
    Object.h
    omplayer.h
    RefPtr.h
    WeakPtr.h
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4013")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4057")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4100")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4127")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4152")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4201")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4244")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4245")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4267")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4701")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4702")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4706")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4723")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4996")
target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:>:/MT>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/MTd>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/MT>)
target_compile_definitions(${PROJECT_NAME} PRIVATE "_CRT_SECURE_NO_WARNINGS")
target_compile_definitions(${PROJECT_NAME} PRIVATE "NOMINMAX")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static setupapi)

add_library(EGL SHARED IMPORTED)
set_target_properties(EGL PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/src/angle/out/Release/libEGL.dll")
set_target_properties(EGL PROPERTIES IMPORTED_IMPLIB "${SRCROOT}/build/src/angle/out/Release/libEGL.dll.lib")
target_include_directories(EGL INTERFACE "${SRCROOT}/build/src/angle/include")
target_link_libraries(${PROJECT_NAME} PUBLIC EGL)

add_library(GLESv2 SHARED IMPORTED)
set_target_properties(GLESv2 PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/src/angle/out/Release/libGLESv2.dll")
set_target_properties(GLESv2 PROPERTIES IMPORTED_IMPLIB "${SRCROOT}/build/src/angle/out/Release/libGLESv2.dll.lib")
target_include_directories(GLESv2 INTERFACE "${SRCROOT}/build/src/angle/include")
target_link_libraries(${PROJECT_NAME} PUBLIC GLESv2)

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avcodec.lib")
target_include_directories(avcodec INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC avcodec mfuuid strmiids)

add_library(avfilter STATIC IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avfilter.lib")
target_include_directories(avfilter INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC avfilter)

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avformat.lib")
target_include_directories(avformat INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC avformat secur32)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avutil.lib")
target_include_directories(avutil INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC avutil)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/swresample.lib")
target_include_directories(swresample INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC swresample)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/swscale.lib")
target_include_directories(swscale INTERFACE "${SRCROOT}/build/include")
target_link_libraries(${PROJECT_NAME} PUBLIC swscale)
