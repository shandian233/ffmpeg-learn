cmake_minimum_required(VERSION 3.10)
project(decoder)

#设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#添加头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include/ffmpeg)

#添加库文件路径
link_directories(${CMAKE_SOURCE_DIR}/lib)

#添加源文件
add_executable(decoder 
    src/main.cpp
    src/decoder.cpp
)

#链接FFmpeg库
target_link_libraries(decoder
    avcodec
    avformat
    avutil
    swscale
    swresample
)

#复制DLL文件到输出目录
add_custom_command(TARGET decoder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/bin/avcodec-61.dll"
        "${CMAKE_SOURCE_DIR}/bin/avformat-61.dll"
        "${CMAKE_SOURCE_DIR}/bin/avutil-59.dll"
        "${CMAKE_SOURCE_DIR}/bin/swscale-8.dll"
        "${CMAKE_SOURCE_DIR}/bin/swresample-5.dll"
        "$<TARGET_FILE_DIR:decoder>"
)