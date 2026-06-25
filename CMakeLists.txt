# 设置 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.20)

# 生成 compile_commands.json 文件（用于 clangd、clang-tidy 等工具）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置默认构建类型为Release，如果未指定构建类型
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# 设置 C++ 标准为 C++17，并强制要求支持 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制要求支持指定的 C++ 标准
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁用编译器特定扩展，确保代码更具可移植性

# 设置所有库生成位置无关代码 (Position Independent Code) 对应GCC/Clang的-fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 定义项目名称及支持的语言
project(IPCDemo LANGUAGES C CXX)

# 设置输出目录
# 静态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/lib)
# 动态库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/bin)
# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/bin)

# 仅在 Linux/macOS 上启用 pthread
if (UNIX)
    find_package(Threads REQUIRED)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    link_libraries(Threads::Threads)
endif()

# 添加fmt库
include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.2.0
)

FetchContent_MakeAvailable(fmt)
# 将 fmt 设置为全局链接库。
# 当前项目仅用于 Demo/学习用途，所有 target 都会使用 fmt，
# 因此直接使用 link_libraries() 简化配置，避免在每个
# target_link_libraries() 中重复添加 fmt::fmt。
#
# 注意：
# 对于大型项目或可复用库项目，推荐在具体 target 上使用
# target_link_libraries() 显式声明依赖关系。
link_libraries(fmt::fmt)

add_subdirectory(helloworld)
add_subdirectory(shm)
add_subdirectory(shm_sem)