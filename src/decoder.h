#ifndef DECODER_H
#define DECODER_H

#pragma once

#include <string>
#include <iostream>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}
class Decoder {
public:
    Decoder();
    ~Decoder();
    bool initialize();
   bool decode(const char* input_filename, const char* output_filename);
    void dumpVideoInfo(const char* filename);   //这个就是Dump视频信息的功能
     void analyzeFrameTypes(const char* filename);  //这个是yuv的分析
    bool saveFirstIFrame(const char* input_filename, const char* output_jpg); //获取并保存第一个I帧

     void cleanup();

private:
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVFrame* frame;
    AVPacket* packet;
    int videoStreamIndex;
};

#endif