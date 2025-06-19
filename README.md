# FFmpeg 解码的项目

## 项目简介
该项目是一个使用 FFmpeg API 的 C++ 程序，旨在解码视频文件并处理视频流。项目包含了必要的类和函数，以便于初始化 FFmpeg 库、执行解码操作以及处理错误。

然后本项目主要实现了上课时老师给的流程图中的​​视频处理通道​​部分（黄色和橙色模块），包括：
​​1.解复用（Demuxer）​​：通过avformat_open_input和
avformat_find_stream_info实现。
​
2.​视频解码​​：通过avcodec_send_packet和avcodec_receive_frame完成。
3.​​数据输出​​：YUV数据写入文件（对应流程图中的"图像处理"和"显示屏"）。




## 文件结构
       
ffmpeg-decode-project/
├── CMakeLists.txt           # CMake构建配置文件
├── README.md                # 项目说明文档
├── src/                     # 源代码目录
│   ├── main.cpp            # 主程序入口
│   ├── decoder.h           # 解码器类头文件
│   └── decoder.cpp         # 解码器类实现
├── include/                 # 头文件目录
│   └── ffmpeg/             # FFmpeg头文件
├── lib/                    # 库文件目录
│   ├── avcodec.lib
│   ├── avformat.lib
│   ├── avutil.lib
│   ├── swresample.lib
│   └── swscale.lib
├── bin/                    # DLL文件目录
│   ├── avcodec-61.dll
│   ├── avformat-61.dll
│   ├── avutil-59.dll
│   ├── swresample-5.dll
│   └── swscale-8.dll
└── build/                  # 构建输出目录
    ├── bin/
    │   └── Release/       # 生成的可执行文件目录
    └── ...



decoder.h/cpp:
功能如下：
实现视频解码功能
视频信息分析
帧类型分析
I帧提取和保存



main.cpp:
程序入口
参数解析
调用解码器功能


依赖的库：
avcodec
avformat
avutil
swscale
swresample




## 构建和运行
1. 确保已安装 CMake 和 FFmpeg。
2. 在项目根目录下创建构建目录并进入：
   ```bash
   mkdir build
   cd build
   ```
3. 运行 CMake 配置， 生成VS项目文件
   ```bash
   cmake ..
   ```
4. 构建Release版本
   ```bash
  cmake --build . --config Release
   ```
5. 运行程序：
   ```bash
   .\decoder.exe wukunyang.mp4 output.yuv
   ```

## 依赖
- C++11 或更高版本
- FFmpeg 库




## 思路以及项目逻辑：

首先是：
1.命令行参数​​：
输入MP4文件路径和输出YUV文件路径，通过argv传递。
​2.错误处理​​：
使用std::cerr输出错误（如解码器初始化失败），确保错误信息即时显示。







然后是：
视频信息分析（dumpVideoInfo）​​
​​实现步骤​​：
1.打开文件并获取流信息（avformat_open_input + avformat_find_stream_info）。
2.遍历流找到视频流（AVMEDIA_TYPE_VIDEO）。
提取关键参数（分辨率、帧率、编解码器类型等）并打印。

dumpVideoInfo()：视频头信息解析​​
在流程图的第一阶段（解复用），必须确认媒体文件的视频流参数（如分辨率、帧率），才能正确初始化后续解码器。
​​实现逻辑​​：
通过avformat_open_input打开文件（对应流程图中的"媒体文件"输入）。
使用avformat_find_stream_info获取流信息（对应"解复用器"阶段）。
提取视频流的AVCodecParameters（如codecpar->width/height），打印关键参数。
然后ai给出建议：
分离信息解析和解码过程，避免解码时重复打开文件。







接着是：
帧类型分析（analyzeFrameTypes）​​
​​关键​​：
通过AVPacket的flags & AV_PKT_FLAG_KEY判断是否为关键帧（I帧）。
​流程​​：
读取前100个包（av_read_frame）。
分类输出视频帧和音频帧的PTS、大小等信息。

analyzeFrameTypes()：帧类型分析​​
这是老师给的流程图中的"包队列"需要区分帧类型（I/P/B帧），可以看到视频GOP结构和调试。
​​实现逻辑​​：
读取前100个AVPacket（对应"音频包队列/视频包队列"）。
通过packet->flags & AV_PKT_FLAG_KEY判断关键帧（I帧）。
输出PTS、帧大小等元数据（类似流程图中队列的监控功能）。



接着：
保存首帧为JPEG（saveFirstIFrame）​​
​​关键​​：
寻找第一个关键帧（AV_PKT_FLAG_KEY）。
解码该帧（avcodec_send_packet + avcodec_receive_frame）。
使用JPEG编码器（AV_CODEC_ID_MJPEG）将帧转为JPEG格式。
​​询问ai后了解到​​：
JPEG编码器需要设置像素格式为AV_PIX_FMT_YUVJ420P（兼容YUV420P）

saveFirstIFrame
对应流程图中"图像帧队列"的特殊处理需求（如生成缩略图）。首I帧是视频解码的起点。
实现逻辑​​：
遍历包队列寻找第一个AV_PKT_FLAG_KEY包（流程图中的"视频包队列"筛选）。
解码该包为AVFrame（"视频解码"模块）。
用JPEG编码器转换帧数据（自定义的"图像处理"分支）。



最后：
视频解码（decode）​​
​这是​核心流程​​（遵循老师给的流程图）：
​​1。解复用​​：打开文件并分离视频流。
​2.​初始化解码器​​：根据编解码器ID（如H.264）创建解码上下文。
​3.​解码循环​​：
4.读取包（av_read_frame）→ 然后就发送到解码器（avcodec_send_packet）。
5.接收解码后的帧（avcodec_receive_frame）。
​6.​YUV写入​​：
按平面（Y、U、V）写入文件，注意UV分量宽高是Y的一半（YUV420格式）。
这里是frame->data[0]存储Y分量，frame->data[1]和frame->data[2]存储UV分量。
​7.​缓冲处理​​：
av_packet_unref确保每个包及时释放内存。





过程中的一些思考：
上课时说到 ffmpeg API 的使用规范​​
​​1.上下文管理​​：
每个模块（解复用、解码）有独立的上下文（如AVFormatContext、AVCodecContext）。
​2.​错误处理​​：
所有FFmpeg函数调用后检查返回值（如< 0表示失败）。
3.注意​​内存安全​​：
使用av_frame_free、av_packet_free等专用释放函数。
