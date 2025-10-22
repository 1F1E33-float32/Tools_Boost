cmake_minimum_required(VERSION 3.15...3.27)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

# 查找 Python 和 nanobind
find_package(Python 3.13 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind CONFIG REQUIRED)

# 创建 Python 扩展模块
nanobind_add_module(
    tools_boost
    src/main.cpp
    src/cat_system_crypto.cpp
    src/hca_decryptor.cpp
)

# 设置 C++ 标准
target_compile_features(tools_boost PRIVATE cxx_std_17)

# 安装模块
install(TARGETS tools_boost LIBRARY DESTINATION .)

# 安装类型存根文件 (.pyi)
install(FILES "${CMAKE_SOURCE_DIR}/tools_boost.pyi" DESTINATION .)
