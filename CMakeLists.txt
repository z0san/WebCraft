cmake_minimum_required(VERSION 3.10)

# set the project name
project(WebCraft VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/web)
  
if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    Message( STATUS "EMCC ENABLED")
#    -s PTHREAD_POOL_SIZE=4
    set(USE_FLAGS "-O3 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -pthread -s PTHREAD_POOL_SIZE=8 --preload-file shader -o index.js")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
#    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
#    Message( STATUS "EMCC DISABLED")
    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    find_package(SDL2 REQUIRED)
    find_package(OpenGLES2 REQUIRED)
endif()

# add the executable
add_executable(WebCraft
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        src/MyGL.cpp
        src/MyGL.h
        src/Drawable.cpp
        src/Drawable.h
        src/glm_includes.h
        src/ShaderProgram.cpp
        src/ShaderProgram.h
        src/WorldAxes.cpp
        src/WorldAxes.h
        src/Entity.cpp
        src/Entity.h
        src/Camera.cpp
        src/Camera.h
        src/Player.cpp
        src/Player.h
        src/smartpointerhelp.h
        src/Terrain.cpp
        src/Terrain.h
        src/noise.h
        src/noise.cpp
        src/chunk.cpp
        src/chunk.h
        src/Texture.cpp src/Texture.h
        src/Quad.cpp src/Quad.h src/FrameBuffer.cpp src/FrameBuffer.h src/Reticle.cpp src/Reticle.h)

# glm
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

include_directories(${CMAKE_SOURCE_DIR}/lib ${SDL2_INCLUDE_DIRS} ${OPENGLES_INCLUDE_DIRS})
target_link_libraries(WebCraft ${SDL2_LIBRARIES} ${OPENGLES_LIBRARIES} glm)
