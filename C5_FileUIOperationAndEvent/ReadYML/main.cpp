//
//  main.cpp
//  ReadYML
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

//NOTE:
// 1.在运行该程序之前，请先运行WriteYML生成test.yml文件，直接运行本程序将无法找到正确文件
// 2.由于本程序使用的test.yml文件都是WriteYML生成的，已经知道其数据结构，因此代码中没有判断具体的数据类型，在实际的工程中应该先判断数据类型，再以适当的方式读取数据

#include <iostream>
#include <opencv2/core.hpp>

int main(int argc, const char * argv[]) {
    std::cout << "\nExample. Using cv::FileStorage to read a .yml file"
    << " \nCall: \n"
    << argv[0] << std::endl;

    cv::FileStorage fs2("test.yml", cv::FileStorage::READ);
    // 使用重载运算符[]读取cv::FileStorage的数据，返回值是cv::FileNode实例
    // 第一种读取cv::FileNode数据的方式，使用重载的类型转换运算符
    int frameCount = (int)fs2["frameCount"];
    // 第二种读取cv::FileNode数据的方式，使用重载的输出运算符>>
    std::string date;
    fs2["calibrationDate"] >> date;

    cv::Mat cameraMatrix2, distCoeffs2;
    fs2["cameraMatrix"] >> cameraMatrix2;
    fs2["distCoeffs"] >> distCoeffs2;
    std::cout << "frameCount: "        << frameCount    << std::endl
              << "calibration date: "  << date          << std::endl
              << "camera matrix: "     << cameraMatrix2 << std::endl
              << "distortion coeffs: " << distCoeffs2   << std::endl;

    cv::FileNode features = fs2["features"];
    // 第三种读取cv::FileNode数据的方式，使用迭代器
    cv::FileNodeIterator it = features.begin();
    cv::FileNodeIterator it_end = features.end();
    int idx = 0;
    std::vector<uchar> lbpval;
    for(; it != it_end; ++it, idx++) {
        std::cout << "feature #" << idx << ": ";
        // 使用重载运算符*获取cv::FileNode实例，使用重载运算符[]获取这个映射关系值的cv::FileNode实例，再使用重载的类型强转运算符(int)获取真实数据
        std::cout << "x=" << (int)(*it)["x"]
                  << ", y=" << (int)(*it)["y"]
                  << ", lbp: (";
        // 读取由数字组成的序列数据时可以使用如下小技巧，通过重载的输出运算符>>将值直接写入到std::vector实例中
        (*it)["lbp"] >> lbpval;
        for(int i = 0; i < (int)lbpval.size(); i++) {
            std::cout << " " << (int)lbpval[i];
        }
        std::cout << ")" << std::endl;
    }

    fs2.release();
    
    return 0;
}
