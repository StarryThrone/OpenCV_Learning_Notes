//
//  main.cpp
//  FourCCToVideoCodec
//
//  Created by chenjie on 2020/6/12.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

void help(const char * argv[]) {
    std::cout << "\n"
    << "Unpacking a four-character code to identify a video codec"
    << "\nCall:\n"
    << argv[0] <<" <path/video>\n"
    << "For example:\n"
    << argv[0] << " ../tree.avi\n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help(argv);
        return -1;
    }

    cv::VideoCapture cap = cv::VideoCapture(argv[1]);
    unsigned f = (unsigned)cap.get( cv::CAP_PROP_FOURCC );
    char fourcc[] = {(char)f, (char)(f >> 8), (char)(f >> 16), (char)(f >> 24), '\0'};
    std::cout << "FourCC for this video was: " << fourcc << std::endl;

    return 0;
}
