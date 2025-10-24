cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

find_package(Python 3.13 COMPONENTS Interpreter Development.Module REQUIRED)

add_subdirectory(_ThirdParty/nanobind)
add_subdirectory(_ThirdParty/zlib)

nanobind_add_module(
    tools_boost
    src/main.cpp
    src/catsystem2_crypto.cpp
    src/hca_decryptor.cpp
    src/xpcm_extractor.cpp
)

target_compile_features(tools_boost PRIVATE cxx_std_17)

target_link_libraries(tools_boost PRIVATE zlibstatic)

target_include_directories(tools_boost PRIVATE 
    ${CMAKE_SOURCE_DIR}/_ThirdParty/zlib
    ${CMAKE_BINARY_DIR}/_ThirdParty/zlib
)

install(TARGETS tools_boost LIBRARY DESTINATION .)
install(FILES "${CMAKE_SOURCE_DIR}/tools_boost.pyi" DESTINATION .)
