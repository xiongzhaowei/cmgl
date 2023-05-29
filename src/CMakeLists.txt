
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
target_link_libraries(${PROJECT_NAME} PUBLIC dwmapi)
target_link_libraries(${PROJECT_NAME} PUBLIC EGL GLESv2 SDL2)
target_link_libraries(${PROJECT_NAME} PUBLIC avcodec avfilter avformat avutil swresample swscale)
