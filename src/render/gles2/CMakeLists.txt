add_subdirectory("renderer")
target_sources(${PROJECT_NAME} PRIVATE
    defines.h
    Program.cpp
    Program.h
    TestRenderObject.cpp
    TestRenderObject.h
    Texture.cpp
    Texture.h
    TextureFramebuffer.cpp
    TextureFramebuffer.h
    Window.cpp
    Window.h
)
