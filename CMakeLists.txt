cmake_minimum_required(VERSION 3.16)

if(WIN32)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
        elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
                CACHE STRING "Vcpkg toolchain file")
        endif()
    endif()
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
endif()

project(osu-beatmap-parser VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/native/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/native/*.hpp")

find_package(SndFile CONFIG REQUIRED)

execute_process(
  COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".node"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (MSVC)
    set(MSVC_RUNTIME_LIBRARY OFF)
    add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${NODE_ADDON_API_DIR}
    ${CMAKE_JS_INC}
)

target_link_libraries(
    ${PROJECT_NAME} 
    ${CMAKE_JS_LIB}
    SndFile::sndfile
)