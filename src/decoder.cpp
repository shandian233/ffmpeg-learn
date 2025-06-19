#include "decoder.h"
#include <iostream>
#include <fstream>


//整体的设计框架，包括参考的老师给的流程图，类，方法，成员变量是我完成的
//但是由于设计到很多没有接触过的api接口调用，所以有些函数的细节是ai完成的
//如果是ai写的我会注释



Decoder::Decoder() : 
formatContext(nullptr), 
codecContext(nullptr), 
    frame(nullptr), 
    packet(nullptr), 
    videoStreamIndex(-1) {
}

Decoder::~Decoder() {
    cleanup();
}

bool Decoder::initialize() {
    return true;
}

bool Decoder::decode(const char* input_filename, const char* output_filename) {
    // 打开视频文件
    if (avformat_open_input(&formatContext, input_filename, nullptr, nullptr) < 0) {
        std::cerr << "无法打开文件: " << input_filename << std::endl;
        return false;
    }

    // 获取流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        return false;
    }

    // 找到视频流
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "找不到视频流" << std::endl;
        return false;
    }

    // 获取解码器
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        std::cerr << "找不到解码器" << std::endl;
        return false;
    }

    // 创建解码器上下文
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "无法创建解码器上下文" << std::endl;
        return false;
    }

    // 设置解码器参数
    if (avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) < 0) {
        std::cerr << "无法设置解码器参数" << std::endl;
        return false;
    }

    // 打开解码器
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "无法打开解码器" << std::endl;
        return false;
    }

    // 分配帧和包
    frame = av_frame_alloc();
    packet = av_packet_alloc();

    // 打开输出文件
    //无缓冲写入​​：
    //直接使用std::ofstream写入文件，避免缓冲延迟。
    //这里ai用的是二进制流
   std::ofstream outFile(output_filename, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "无法创建输出文件: " << output_filename << std::endl;
        return false;
    }
    // 读取视频包并解码
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecContext, packet) < 0) {
                std::cerr << "发送包失败" << std::endl;
                return false;
            }

            while (true) {
                int ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "解码错误" << std::endl;
                    return false;
                }

                // 写入YUV的数据

//按平面（Y、U、V）写入文件，注意UV分量宽高是Y的一半（YUV420格式）。
//这里自然而然就想到这样处理：
//frame->data[0]存储Y分量，frame->data[1]和frame->data[2]存储UV分量。

                for (int i = 0; i < frame->height; i++) {
                    outFile.write((const char*)frame->data[0] + i * frame->linesize[0], frame->width);
                }
                for (int i = 0; i < frame->height/2; i++) {
                    outFile.write((const char*)frame->data[1] + i * frame->linesize[1], frame->width/2);
                }
                for (int i = 0; i < frame->height/2; i++) {
                    outFile.write((const char*)frame->data[2] + i * frame->linesize[2], frame->width/2);
                }
            }
        }
        av_packet_unref(packet);
    }

    outFile.close();
    return true;
}

void Decoder::cleanup() {
    if (codecContext) avcodec_free_context(&codecContext);
    if (formatContext) avformat_close_input(&formatContext);
    if (frame) av_frame_free(&frame);
    if (packet) av_packet_free(&packet);
}


//ai写的传参都会注意用const，是好习惯
void Decoder::dumpVideoInfo(const char* filename) {
    if (avformat_open_input(&formatContext, filename, nullptr, nullptr) < 0) {
        std::cerr << "无法打开文件进行信息dump" << std::endl;
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        avformat_close_input(&formatContext);
        return;
    }

    // 查找视频流
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "未找到视频流" << std::endl;
        avformat_close_input(&formatContext);
        return;
    }

    // 获取编解码器参数
    AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;
   

    //这里的输出格式是ai美化的
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
std::cout << "\n====== Video File Information ======\n";
    std::cout << "Filename: " << filename << "\n";
    std::cout << "Format: " << formatContext->iformat->name << "\n";
    std::cout << "Duration: " << formatContext->duration / AV_TIME_BASE << " seconds\n";
    std::cout << "Bitrate: " << formatContext->bit_rate / 1000 << " Kbit/s\n";
    std::cout << "Video Codec: " << codec->name << "\n";
    std::cout << "Resolution: " << codecParams->width << "x" << codecParams->height << "\n";
    std::cout << "Frame Rate: " << av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate) << " fps\n";
    std::cout << "Video Stream Index: " << videoStreamIndex << "\n";
    std::cout << "==============================\n\n";

    avformat_close_input(&formatContext);
}



//ai辅助完成的
//考虑到终端输出的信息太多就注释掉了
//这里是实现的存储每一帧的信息，有：
//Y（亮度）分量
//U、V（色度）分量：
/*
void Decoder::analyzeYUVFile(const char* yuv_filename, int width, int height) {
    FILE* fp = fopen(yuv_filename, "rb");
    if (!fp) {
        std::cerr << "Cannot open YUV file: " << yuv_filename << std::endl;
        return;
    }

    // 计算一帧的大小 (YUV420格式)
    int frame_size = width * height * 3 / 2;  // Y分量 + U分量 + V分量
    unsigned char* buffer = new unsigned char[frame_size];
    int frame_count = 0;

    std::cout << "\n====== YUV File Analysis ======\n";
    std::cout << "Format: YUV420P\n";
    std::cout << "Resolution: " << width << "x" << height << "\n";

    // 读取并分析每一帧
    while (fread(buffer, 1, frame_size, fp) == frame_size) {
        frame_count++;
        
        // 分析Y分量
        std::cout << "Frame " << frame_count << " Y component (first 16 bytes): ";
        for (int i = 0; i < 16; i++) {
            printf("%02x ", buffer[i]);
        }
        std::cout << "\n";

        // 分析U分量
        std::cout << "Frame " << frame_count << " U component (first 8 bytes): ";
        int u_offset = width * height;
        for (int i = 0; i < 8; i++) {
            printf("%02x ", buffer[u_offset + i]);
        }
        std::cout << "\n";

        // 分析V分量
        std::cout << "Frame " << frame_count << " V component (first 8 bytes): ";
        int v_offset = width * height * 5 / 4;
        for (int i = 0; i < 8; i++) {
            printf("%02x ", buffer[v_offset + i]);
        }
        std::cout << "\n\n";
    }

    std::cout << "Total Frames: " << frame_count << "\n";
    std::cout << "============================\n";

    delete[] buffer;
    fclose(fp);
}*/

void Decoder::analyzeFrameTypes(const char* filename) {
    if (avformat_open_input(&formatContext, filename, nullptr, nullptr) < 0) {
        std::cout << "Cannot open file" << std::endl;
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cout << "Cannot get stream info" << std::endl;
        avformat_close_input(&formatContext);
        return;
    }
//这里的输出格式是ai美化的
    std::cout << "\n===== Frame Type Analysis =====\n";
    
    AVPacket* packet = av_packet_alloc();
    
    //增加到100帧以确保能看到音视频帧
    int count = 0;
    while (av_read_frame(formatContext, packet) >= 0 && count < 100) {
        AVStream* stream = formatContext->streams[packet->stream_index];
        
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "Video Frame " << count + 1 
                      << " - PTS: " << packet->pts 
                      << ", Size: " << packet->size << " bytes"
                      << ", Key Frame: " << ((packet->flags & AV_PKT_FLAG_KEY) ? "Yes" : "No")
                      << std::endl;
        }
        else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "Audio Frame " << count + 1 
                      << " - PTS: " << packet->pts 
                      << ", Size: " << packet->size << " bytes"
                      << ", Duration: " << packet->duration
                      << std::endl;
        }
        
        count++;
        av_packet_unref(packet);
    }
    
    std::cout << "\nTotal Frames Analyzed: " << count << std::endl;
    std::cout << "===================\n";
//这里的输出格式是ai美化的
    av_packet_free(&packet);
    avformat_close_input(&formatContext);
}




//有了前面的经验，写这个就没那么痛苦了
//我参考了FFmpeg API的典型使用流程，
//从解封装、解码到编码的完整处理链


bool Decoder::saveFirstIFrame(const char* input_filename, const char* output_jpg) {
   //这里其实应该使用智能指针，可以更好地资源管理
   //但是期末周了，就偷懒了
    AVFormatContext* fmt_ctx = nullptr; // 格式上下文，存储音视频封装格式相关信息
    AVCodecContext* dec_ctx = nullptr;   // 解码器上下文，存储编解码器相关参数
    const AVCodec* dec = nullptr;     // 解码器，用于实际的解码工作
    AVFrame* frame = nullptr;     // 存储解码后的原始帧数据
    int video_stream_idx = -1;    // 视频流索引，初始化为-1表示未找到
    
   // 打开输入文件并创建格式上下文
    // avformat_open_input会自动分配fmt_ctx内存
    if (avformat_open_input(&fmt_ctx, input_filename, nullptr, nullptr) < 0) {
        std::cerr << "Cannot open input file" << std::endl;
        //仿照ai使用cerr
        return false;
    }

   //遍历所有流，查找视频流
    //nb_streams表示视频文件中的流数量
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx < 0) {
        std::cerr << "No video stream found" << std::endl;
        avformat_close_input(&fmt_ctx);
        return false;
    }

    //查找并配置解码器
    //根据编解码器ID获取对应的解码器
    dec = avcodec_find_decoder(fmt_ctx->streams[video_stream_idx]->codecpar->codec_id);
    if (!dec) {
        std::cerr << "Failed to find decoder" << std::endl;
        //std::cerr（标准错误流）​​
//专用于输出​​错误信息、警告或诊断消息​​，通常与程序异常或调试相关。
//其设计目的是在紧急情况下（如程序崩溃）确保错误信息能立即显示
        avformat_close_input(&fmt_ctx);
        return false;
    }

     //创建解码器上下文
    //使用找到的解码器分配上下文内存
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
        std::cerr << "Failed to allocate decoder context" << std::endl;
        avformat_close_input(&fmt_ctx);
        return false;
    }

     //将编解码器参数从流复制到上下文
    //确保解码器上下文包含正确的参数
    if (avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_idx]->codecpar) < 0) {
        std::cerr << "Failed to copy decoder params" << std::endl;
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return false;
    }

    //这里就是打开解码器
    if (avcodec_open2(dec_ctx, dec, nullptr) < 0) {
        std::cerr << "Failed to open decoder" << std::endl;
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return false;
    }
//分配Frame和Packet对象
    frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    
    bool found_keyframe = false;
    // 寻找第一个I帧
    while (av_read_frame(fmt_ctx, packet) >= 0 && !found_keyframe) {
        //循环读取帧直到找到I帧
        if (packet->stream_index == video_stream_idx) {
            if (packet->flags & AV_PKT_FLAG_KEY) {
                // 检查是否为关键帧(I帧)
                
                
                //解码I帧
                //发送数据包到解码器
                if (avcodec_send_packet(dec_ctx, packet) >= 0) {
                    // 接收解码后的帧
                    if (avcodec_receive_frame(dec_ctx, frame) >= 0) {
                        found_keyframe = true;
                        std::cout << "Found first I-Frame, saving to JPEG..." << std::endl;
                        
                        // 将帧转换为JPEG
                        FILE* jpeg_file = fopen(output_jpg, "wb");
                        if (!jpeg_file) {
                            std::cerr << "Cannot open output JPEG file" << std::endl;
                            break;
                        }
                        
                        //配置JPEG编码器，由于我没接触过这方面，这里是询问ai的
                        AVCodec* jpeg_codec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_MJPEG));
                        AVCodecContext* jpeg_ctx = avcodec_alloc_context3(jpeg_codec);
                        
                        //设置JPEG编码参数
                        //这些都是ai配的，我不懂
                        jpeg_ctx->width = frame->width;
                        jpeg_ctx->height = frame->height;
                        jpeg_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;   //使用JPEG优化的YUV420格式
                        jpeg_ctx->time_base.num = 1;
                        jpeg_ctx->time_base.den = 25;   //这里是设置时间基准，也是ai做的
                        
                        if (avcodec_open2(jpeg_ctx, jpeg_codec, nullptr) < 0) {
                            std::cerr << "Could not open JPEG codec" << std::endl;
                            fclose(jpeg_file);
                            break;
                        }
                        
                        //然后就是写入文件就可以了
                        AVPacket* jpeg_packet = av_packet_alloc();
                        
                        if (avcodec_send_frame(jpeg_ctx, frame) >= 0) {
                            if (avcodec_receive_packet(jpeg_ctx, jpeg_packet) >= 0) {
                                fwrite(jpeg_packet->data, 1, jpeg_packet->size, jpeg_file);
                            }
                        }
                        
                        fclose(jpeg_file);
                        av_packet_free(&jpeg_packet);
                        avcodec_free_context(&jpeg_ctx);
                    }
                }
            }
        }
        av_packet_unref(packet);
    }

    // 清理资源
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

    return found_keyframe;
}




