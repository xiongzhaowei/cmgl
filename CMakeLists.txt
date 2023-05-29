
cmake_minimum_required(VERSION 3.15.0)

project(cmgl LANGUAGES CXX)

get_filename_component(SRCROOT "${CMAKE_SOURCE_DIR}" ABSOLUTE)

add_library(EGL SHARED IMPORTED)
set_target_properties(EGL PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/src/angle/out/Release/libEGL.dll")
set_target_properties(EGL PROPERTIES IMPORTED_IMPLIB "${SRCROOT}/build/src/angle/out/Release/libEGL.dll.lib")
target_include_directories(EGL INTERFACE "${SRCROOT}/build/src/angle/include")
install(FILES "${SRCROOT}/build/src/angle/out/Release/libEGL.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}")

add_library(GLESv2 SHARED IMPORTED)
set_target_properties(GLESv2 PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/src/angle/out/Release/libGLESv2.dll")
set_target_properties(GLESv2 PROPERTIES IMPORTED_IMPLIB "${SRCROOT}/build/src/angle/out/Release/libGLESv2.dll.lib")
target_include_directories(GLESv2 INTERFACE "${SRCROOT}/build/src/angle/include")
install(FILES "${SRCROOT}/build/src/angle/out/Release/libGLESv2.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}")

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avcodec.lib")
target_include_directories(avcodec INTERFACE "${SRCROOT}/build/include")
target_link_libraries(avcodec INTERFACE mfuuid strmiids)

add_library(avfilter STATIC IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avfilter.lib")
target_include_directories(avfilter INTERFACE "${SRCROOT}/build/include")

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avformat.lib")
target_include_directories(avformat INTERFACE "${SRCROOT}/build/include")
target_link_libraries(avformat INTERFACE secur32 ws2_32)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/avutil.lib")
target_include_directories(avutil INTERFACE "${SRCROOT}/build/include")
target_link_libraries(avutil INTERFACE bcrypt)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/swresample.lib")
target_include_directories(swresample INTERFACE "${SRCROOT}/build/include")

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/swscale.lib")
target_include_directories(swscale INTERFACE "${SRCROOT}/build/include")

add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION "${SRCROOT}/build/lib/SDL2-static.lib")
target_include_directories(SDL2 INTERFACE "${SRCROOT}/build/include")
target_link_libraries(SDL2 INTERFACE setupapi version winmm)

add_subdirectory("examples")
add_subdirectory("src")
