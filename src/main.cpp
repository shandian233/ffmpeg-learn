#include <iostream>
#include <libavformat/avformat.h>
#include "decoder.h"

int main(int argc, char* argv[]) {
    // 初始化FFmpeg库
    //av_register_all();
    //查询到说FFmpeg 4.0+ 不再需要显式注册编解码器
    if (argc != 3) {
        std::cout << "用法: decoder <输入MP4文件> <输出YUV文件>" << std::endl;
        return -1;
    }
    
    Decoder decoder;
    if (!decoder.initialize()) {
        std::cerr << "解码器初始化失败" << std::endl;
        return -1;
    }
    //std::cerr（标准错误流）​​
//专用于输出​​错误信息、警告或诊断消息​​，通常与程序异常或调试相关。
//其设计目的是在紧急情况下（如程序崩溃）确保错误信息能立即显示






// 先dump视频信息
    decoder.dumpVideoInfo(argv[1]);

 // 分析输入文件的帧类型
    decoder.analyzeFrameTypes(argv[1]);

const char* input_file = argv[1];
    const char* output_file = argv[2];
    
  // 保存第一个I帧为JPEG
    std::string jpg_filename = "first_iframe.jpg";
    if (decoder.saveFirstIFrame(argv[1], jpg_filename.c_str())) {
        std::cout << "Successfully saved first I-Frame to " << jpg_filename << std::endl;
    }





    if (!decoder.decode(input_file, output_file)) {  // 需要修改decode函数签名
        std::cerr << "Decoding failed" << std::endl;
        //中文会乱码
        decoder.cleanup();
        return -1;
    }
 //decoder.analyzeYUVFile(output_file, 1920, 1080);  // 使用实际的视频分辨率
    decoder.cleanup();
    return 0;
}