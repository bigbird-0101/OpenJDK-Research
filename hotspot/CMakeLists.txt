cmake_minimum_required(VERSION 3.15)
project(JDK8_HotSpot_Cygwin)

# 设置Cygwin特定选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# Cygwin编译器和链接器设置
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Cygwin编译标志
add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-deprecated-declarations
        -fno-rtti
        -fno-exceptions
)

# JDK8 HotSpot特定定义
add_definitions(
        -D_WINDOWS
        -D_AMD64_
        -Dx86_64
        -D_LP64=1
        -D_JNI_IMPLEMENTATION_
        -DDEBUG
        -DASSERT
        -DVM_LITTLE_ENDIAN
        -DHOTSPOT_BUILD_USER="JDK8_Cygwin_User"
        -DHOTSPOT_RELEASE_VERSION="1.8"
        -DINCLUDE_TRACE
        # 添加JNI必要的定义
        -DTARGET_ARCH_x86
)

# 调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()

# HotSpot JDK8包含目录
include_directories(
        # 根目录
        ${CMAKE_SOURCE_DIR}

        # 共享VM代码
        ${CMAKE_SOURCE_DIR}/src/share/vm
        ${CMAKE_SOURCE_DIR}/src/share/vm/precompiled
        ${CMAKE_SOURCE_DIR}/src/share/vm/prims
        ${CMAKE_SOURCE_DIR}/src/share/vm/runtime
        ${CMAKE_SOURCE_DIR}/src/share/vm/memory
        ${CMAKE_SOURCE_DIR}/src/share/vm/oops
        ${CMAKE_SOURCE_DIR}/src/share/vm/utilities
        ${CMAKE_SOURCE_DIR}/src/share/vm/code
        ${CMAKE_SOURCE_DIR}/src/share/vm/compiler
        ${CMAKE_SOURCE_DIR}/src/share/vm/interpreter
        ${CMAKE_SOURCE_DIR}/src/share/vm/ci
        ${CMAKE_SOURCE_DIR}/src/share/vm/gc_implementation/shared
        ${CMAKE_SOURCE_DIR}/src/share/vm/gc_interface
        ${CMAKE_SOURCE_DIR}/src/share/vm/classfile
        ${CMAKE_SOURCE_DIR}/src/share/vm/gc_implementation/parallelScavenge
        ${CMAKE_SOURCE_DIR}/src/share/vm/gc_implementation/parNew
        ${CMAKE_SOURCE_DIR}/src/share/vm/gc_implementation/concurrentMarkSweep
        ${CMAKE_SOURCE_DIR}/src/share/vm/trace

        # 架构特定
        ${CMAKE_SOURCE_DIR}/src/cpu/x86/vm
        ${CMAKE_SOURCE_DIR}/src/os/windows/vm
        ${CMAKE_SOURCE_DIR}/src/os_cpu/windows_x86/vm
        
        # 生成的文件目录
        ${CMAKE_BINARY_DIR}/generated
)

# 创建生成文件的目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated/jvmtifiles)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated/tracefiles)

# 设置文件夹组织（在CLion中显示）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 查找源文件的函数
function(find_hotspot_sources DIR VARIABLE_NAME)
    file(GLOB_RECURSE ${VARIABLE_NAME}
            "${DIR}/*.cpp"
            "${DIR}/*.c"
            "${DIR}/*.cc"
    )
    set(${VARIABLE_NAME} ${${VARIABLE_NAME}} PARENT_SCOPE)
endfunction()

# JVMTI文件生成
add_custom_command(
    OUTPUT 
        ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmti.h
        ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnv.hpp
        ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnter.cpp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated/jvmtifiles
    COMMAND ${CMAKE_COMMAND} -E echo "Generating JVMTI files..."
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmti.h ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmti.h
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmtiEnv.hpp ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnv.hpp
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmtiEnter.hpp ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnter.hpp
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmti.h
        ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmtiEnv.hpp
        ${CMAKE_SOURCE_DIR}/src/share/vm/prims/jvmtiEnter.hpp
    COMMENT "Generating JVMTI files"
)

add_custom_target(jvmti_files DEPENDS 
    ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmti.h
    ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnv.hpp
    ${CMAKE_BINARY_DIR}/generated/jvmtifiles/jvmtiEnter.hpp
)

# Trace文件生成
add_custom_command(
    OUTPUT 
        ${CMAKE_BINARY_DIR}/generated/tracefiles/traceTypes.hpp
        ${CMAKE_BINARY_DIR}/generated/tracefiles/traceEventIds.hpp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated/tracefiles
    COMMAND ${CMAKE_COMMAND} -E echo "Generating Trace files..."
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/share/vm/trace/traceDataTypes.hpp ${CMAKE_BINARY_DIR}/generated/tracefiles/traceTypes.hpp
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/share/vm/trace/traceEvent.hpp ${CMAKE_BINARY_DIR}/generated/tracefiles/traceEventIds.hpp
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/src/share/vm/trace/traceDataTypes.hpp
        ${CMAKE_SOURCE_DIR}/src/share/vm/trace/traceEvent.hpp
    COMMENT "Generating Trace files"
)

add_custom_target(trace_files DEPENDS 
    ${CMAKE_BINARY_DIR}/generated/tracefiles/traceTypes.hpp
    ${CMAKE_BINARY_DIR}/generated/tracefiles/traceEventIds.hpp
)

# 收集各个模块的源文件
find_hotspot_sources("src/share/vm" SHARE_SOURCES)
find_hotspot_sources("src/cpu/x86/vm" CPU_SOURCES)
find_hotspot_sources("src/os/windows/vm" OS_SOURCES)
find_hotspot_sources("src/os_cpu/windows_x86/vm" OS_CPU_SOURCES)

# 过滤掉可能不兼容的文件（根据需要调整）
list(FILTER SHARE_SOURCES EXCLUDE REGEX ".*/adlc/.*")
list(FILTER SHARE_SOURCES EXCLUDE REGEX ".*/jvmtifiles/.*")

# 在CLion中显示文件分组
source_group("Share VM" FILES ${SHARE_SOURCES})
source_group("CPU x86" FILES ${CPU_SOURCES})
source_group("OS Windows" FILES ${OS_SOURCES})
source_group("OS_CPU Windows x86" FILES ${OS_CPU_SOURCES})

# 合并所有源文件
set(ALL_SOURCES
        ${SHARE_SOURCES}
        ${CPU_SOURCES}
        ${OS_SOURCES}
        ${OS_CPU_SOURCES}
)

# 创建jvm.dll目标（Cygwin中为libjvm.dll）
add_library(jvm SHARED ${ALL_SOURCES})
add_dependencies(jvm jvmti_files trace_files)

# 设置目标属性
set_target_properties(jvm PROPERTIES
        OUTPUT_NAME "jvm"
        SUFFIX ".dll"
        PREFIX ""
        FOLDER "JDK8 HotSpot"
)

# Cygwin链接库
target_link_libraries(jvm
        kernel32
        user32
        advapi32
        ws2_32
        dbghelp
        shell32
)

add_executable(jvmMain main.cpp)

# 链接hotspot库到可执行文件
target_link_libraries(jvmMain jvm)
# 输出信息
message(STATUS "JDK8 HotSpot配置完成")
message(STATUS "编译器: ${CMAKE_C_COMPILER}")
message(STATUS "C++编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "共享VM文件数量: ${SHARE_SOURCES}")
message(STATUS "CPU文件数量: ${CPU_SOURCES}")
message(STATUS "总源文件数量: ${ALL_SOURCES}")