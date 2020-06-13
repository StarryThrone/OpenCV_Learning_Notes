//
//  main.cpp
//  Trackbar
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

// 使用一个滚动条模拟开关，控制视频的播放和暂停
int g_switch_value = 1;
void switch_off_function() {
    std::cout << "Pause \n";
};

void switch_on_function() {
    std::cout << "Run \n";
};

// 定义滚动条的回调函数
void switch_callback(int position, void *) {
  if (position == 0) {
    switch_off_function();
  } else {
    switch_on_function();
  }
}

void help(const char * argv[]) {
    std::cout << "Example. Using a trackbar to create a “switch” that the user can turn on and off"
              << "\n this program plays a video and uses the switch to create a pause functionality."
              << "\n\n Call: \n" << argv[0] << " <path/video_file>"
              << "\n\n Shows putting a pause button in a video; Esc to quit \n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);

    // 定义图片实例持有视频帧
    cv::Mat frame;
    cv::VideoCapture g_capture;
    
    if (argc < 2 || !g_capture.open(argv[1])) {
        std::cout << "Failed to open " << argv[1] << " video file\n" << std::endl;
        return -1;
    }

    // 创建窗口
    cv::namedWindow("Example", 1);
    // 在指定窗口中创建滑动条
    cv::createTrackbar("Switch", "Example", &g_switch_value, 1, switch_callback);
    
    // 挂起程序，播放视频，直至用户输入ESC键
    for (;;) {
        if (g_switch_value) {
            // 如果开关是开启状态才播放视频，否则视为暂停播放
            g_capture >> frame;
            if (frame.empty()) {
                break;
            }
            cv::imshow("Example", frame);
        }
        if (cv::waitKey(1000/60) == 27) {
            break;
        }
    }

    return 0;
}
