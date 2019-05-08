cmake_minimum_required(VERSION 3.1)
project(Forge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++17")

set ( CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}" )
set ( CMAKE_SHARED_LINKER_FLAGS "-image_base 7fff04c4a000" ${CMAKE_SHARED_LINKER_FLAGS})

set(CMAKE_MACOSX_RPATH 1)
add_subdirectory(Forge/Vendor/luajit)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Forge/Vendor/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(Forge/Vendor/assimp)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(Forge/Vendor/bullet)

add_subdirectory(Forge/Vendor/libnoise)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(Forge/Headers/
                    Forge/Vendor/assimp/include/
                    Forge/Vendor/bullet/src/
                    Forge/Vendor/glad/include/
                    Forge/Vendor/glfw/include/
                    Forge/Vendor/glm/
                    Forge/Vendor/stb/
                    Forge/Vendor/polyvox/include/
                    Forge/Vendor/fastnoise/
                    Forge/Vendor/libnoise/include/
                    Forge/Vendor/gltext/
                    Forge/Vendor/nuklear/
                    Forge/Vendor/sol2/single/
                    Forge/Vendor/luajit/src/
                    Forge/Vendor/kangaru/include/)

file(GLOB VENDORS_SOURCES Forge/Vendor/glad/src/glad.c
                          Forge/Vendor/fastnoise/FastNoise.cpp)
file(GLOB PROJECT_HEADERS Forge/Headers/*.hpp)
file(GLOB PROJECT_SOURCES Forge/Sources/*.cpp)
file(GLOB PROJECT_SHADERS Forge/Shaders/*.comp
                          Forge/Shaders/*.frag
                          Forge/Shaders/*.geom
                          Forge/Shaders/*.vert)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                               ${VENDORS_SOURCES})
target_link_libraries(${PROJECT_NAME} assimp glfw liblua
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
                      BulletDynamics BulletCollision LinearMath 
                      libnoise)
set_target_properties(${PROJECT_NAME} PROPERTIES ${LUAJIT_LINK_FLAGS}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
