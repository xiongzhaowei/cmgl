
project(encode LANGUAGES CXX)

add_executable(${PROJECT_NAME} WIN32
  "main.cpp"
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4100")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4189")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4201")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4310")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4312")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4456")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4457")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4458")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"4819")
target_compile_options(${PROJECT_NAME} PRIVATE /wd"5033")
target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:>:/MT>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/MTd>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/MT>)
target_compile_definitions(${PROJECT_NAME} PRIVATE "_HAS_EXCEPTIONS=0")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")
target_link_libraries(${PROJECT_NAME} PRIVATE comctl32 gdiplus imm32 msimg32 shlwapi)
target_link_libraries(${PROJECT_NAME} PRIVATE cmgl EGL GLESv2)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
add_dependencies(${PROJECT_NAME} EGL GLESv2)

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
  COMPONENT Runtime
)
