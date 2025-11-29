cmake_minimum_required(VERSION 3.16)
project(osu-beatmap-parser VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

add_compile_definitions(NAPI_VERSION=7)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/native/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/native/*.hpp")

add_subdirectory(vendor/libsndfile)

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libsndfile/include
    ${NODE_ADDON_API_DIR}
    ${CMAKE_JS_INC}
)

target_link_libraries(
    ${PROJECT_NAME} 
    ${CMAKE_JS_LIB}
    sndfile
)

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
)