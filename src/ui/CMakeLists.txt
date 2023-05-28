add_subdirectory("windows")
target_sources(${PROJECT_NAME} PRIVATE
    defines.cpp
    defines.h
    Type.cpp
    Type.h
    View.cpp
    View.h
)
