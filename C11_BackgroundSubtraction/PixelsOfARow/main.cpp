//
//  main.cpp
//  PixelsOfARow
//
//  Created by chenjie on 2020/7/30.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void help(const char * argv[]) {
    std::cout << "\n"
              << "Example: Read out RGB pixel values and store them to disk\nCall:\n"
              << argv[0] << " <path/video_file>\n"
              << "\nExample:\n" << argv[0] << " ../tree.avi"
              << "\n This will store to files blines.csv, glines.csv and rlines.csv\n\n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help(argv);
        return -1;
    }
    
    // 读取视频文件
    cv::VideoCapture cap = cv::VideoCapture(argv[1]);
    if (!cap.isOpened()) {
        std::cerr << "Couldn't open video file" << std::endl;
        help(argv);
        return -1;
    }
    
    // 创建用于播放视频的窗口
    cv::namedWindow(argv[0], cv::WINDOW_AUTOSIZE);
    
    // 准备写入视频帧某条直线所有像素BGR数据的文件
    std::ofstream bfile, gfile, rfile;
    bfile.open("blines.csv");
    gfile.open("glines.csv");
    rfile.open("rlines.csv");
    
    // 定义采样直线的两个端点
    cv::Point ptStart(10, 10), ptEnd(30, 30);
    // 定于接收视频数据的图片
    cv::Mat rawImage;
    cap >> rawImage;
    
    while (rawImage.data) {
        // 沿着指定直线采样
        cv::LineIterator it(rawImage, ptStart, ptEnd, 8);
        // 将数据写入文件
        for (int i = 0; i < it.count; ++i, ++it) {
            bfile << (int)(*it)[0] << ", ";
            gfile << (int)(*it)[1] << ", ";
            rfile << (int)(*it)[2] << ", ";
            // 将红色强度设为最大值
            (*it)[2] = 255;
        }
        
        // 显示图片
        cv::imshow(argv[0], rawImage);
        // 挂起程序等待用户事件，并以25FPS的速度播放视频
        cv::waitKey(1000/25);
        
        // 每帧的数据换行
        bfile << "\n"; gfile << "\n"; rfile << "\n";
        // 读取下一帧
        cap >> rawImage;
    }
    
    // 完成文件写入操作
    bfile << std::endl; gfile << std::endl; rfile << std::endl;
    bfile.close(); gfile.close(); rfile.close();
    std::cout << "\n"
              << "Data stored to files: blines.csv, glines.csv and rlines.csv\n\n"
              << std::endl;

    return 0;
}
