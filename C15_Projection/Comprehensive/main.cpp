//
//  main.cpp
//  Comprehensive
//
//  Created by 陈杰 on 2021/7/13.
//

#include <iostream>
#include <math.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>



void help(const char * argv[]) {
  std::cout << "\n\nExample Comprehensive. Stereo calibration, rectification, and correspondence"
            << "\n    Reads in list of locations of a sequence of checkerboard calibration"
            << "\n    objects from a left,right stereo camera pair. Calibrates, rectifies and then"
            << "\n    does stereo correspondence."
            << "\n"
            << "\n    This program will run on default parameters assuming you created a build directory"
            << "\n    directly below the Learning-OpenCV-3 directory and are running programs there."
            << "\nNOTE: The list_of_stereo_pairs> must give the full path name to the left right images, in alternating"
            << "\n    lines: left image, right image, one path/filename per line, see"
            << "\n    stereoData/example_comprehensive_list.txt file, you can comment out lines"
            << "\n    there by starting them with #."
            << "\n"
            << "\nDefault Call (with parameters: board_w = 9, board_h = 6, list = ../stereoData_list.txt):"
            << "\n" << argv[0] << "\n"
            << "\nManual call:"
            << "\n" << argv[0] << " [<board_w> <board_h> <path/list_of_stereo_pairs>]"
            << "\n\n PRESS ANY KEY TO STEP THROUGH RESULTS AT EACH STAGE."
            << "\n" << std::endl;
}


/// 立体校正
/// @param imageList 存储图像对路径列表的txt格式文件
/// @param nx 棋盘网格的列数
/// @param ny 棋盘网格的行数
/// @param useUncalibrated 立体系统校正时使用未标定的相机，还是使用标定后的相机
static void StereoCalib(const char *imageList, int nx, int ny, bool useUncalibrated) {
    // 1 参数校验
    // 读取圆点网格图片名字的目录文件
    FILE *f = fopen(imageList, "rt");
    if (!f) {
      std::cout << "Cannot open file " << imageList << std::endl;
      return;
    }

    // 一些可以自定义算法处理细节的变量
    // 是否需要显示角点
    bool displayCorners = true;
    
    // 棋盘的网格大小
    int N = nx * ny;
    std::vector<cv::Point3f> boardModel;
    // 棋盘上每个方格的宽度
    const float squareSize = 1.f;
    // 计算所有角点的模型坐标
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            boardModel.push_back(cv::Point3f((float)(i * squareSize), (float)(j * squareSize), 0.f));
        }
    }
    
    // 2 获取角点坐标数据
    // 循环读取所有图像
    // 所有有效图像对的模型坐标向量，其中每个元素都表示单个图片中所有角点位于模型坐标系下的坐标集合
    std::vector<std::vector<cv::Point3f>> objectPoints;
    // 所有有效图像对的图像坐标向量，其中每个元素都表示单个图片中所有角点位于图像坐标系下的坐标
    std::vector<std::vector<cv::Point2f>> points[2];
    // 用于临时存储一组有效图像对的角点位于图像坐标系下的坐标
    std::vector<cv::Point2f> corners[2];
    // 棋盘网格的大小
    cv::Size board_sz = cv::Size(nx, ny);
    // 保存左右图像向量的数组
    // 数组第一个元素保存左侧图像的向量列表
    // 数组第二个元素保存右侧图像的向量列表
    std::vector<std::string> imageNames[2];
    // 左右图像是否寻找到了所有角点
    bool found[2] = {false, false};
    // 图片的实际大小，这里假定所有的图片大小一致
    cv::Size imageSize;
    // 文件名在列表文件中的行索引
    int lineIndex = 0;
    while (true) {
        // 用于存储单次读取结果的数组
        char buf[1024];
        // 读取数据时使用一个较大缓存，确保一次能够读取完整一行
        if (!fgets(buf, sizeof(buf) - 3, f)) {
          break;
        }
        // 将读取到的文件行末尾的空格都替换为字符串结束标志符\0，确保读取图像文件时文件名不会异常
        size_t len = strlen(buf);
        while (len > 0 && isspace(buf[len - 1])) {
            buf[--len] = '\0';
        }
        // 如果文件行以#开头，表示需要忽略该行数据
        if (buf[0] == '#') {
            continue;
        }
        // 如果图片读取失败，直接跳到下一行
        cv::Mat img = cv::imread(buf, 0);
        if (img.empty()) {
            break;
        }

        // 偶数行的文件名表示左侧图像，奇数行的文件名表示右侧图像
        int lr = lineIndex % 2;
        lineIndex++;
        if (lr == 0) {
            // 每次读取左侧图像时都将左右图像的角点搜索状态修改为初始值false
            found[0] = found[1] = false;
        }
        // 在处理右侧图像时，如果左侧图像未找到棋盘角点，那么即使右侧图像找到也没有价值，因此直接跳过
        if (lr == 1 && !found[0]) {
            continue;
        }

        imageSize = img.size();
        imageNames[lr].push_back(buf);

        // 寻找角点和中心时从标准比例向上缩放的最大比例
        const int maxScale = 1;
        for (int s = 1; s <= maxScale; s++) {
            // 从标准比例向上查找，
            cv::Mat timg = img;
            if (s > 1) {
                resize(img, timg, cv::Size(), s, s, cv::INTER_CUBIC);
            }
            // 如果使用的是圆圈网格标定板，则使用如下代码
            // found[lr] = cv::findCirclesGrid(timg, cv::Size(nx, ny), corners[lr],
            //                                 cv::CALIB_CB_ASYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING);
            // 使用棋盘网格标定板，则使用如下代码
            found[lr] = cv::findChessboardCorners(timg, board_sz, corners[lr]);
            // 原代码中的未使用逻辑
            // if (found[lr] || s == maxScale) {
            //    cv::Mat mcorners(corners[lr]);
            //    mcorners *= (1. / s);
            // }
            if (found[lr]) {
                // 找到角点就退出循环
                break;
            }
        }
        // 必要时显示每一张图片的角点查询结果
        if (displayCorners) {
            // 打印文件名
            std::cout << buf << std::endl;
            // 将灰度图转化为RGB图像，实际上还是灰度图，只是变成了三个通道。这是显示图像的接口能够接受的格式。
            cv::Mat cimg;
            cv::cvtColor(img, cimg, cv::COLOR_GRAY2BGR);

            // 在棋盘图像上绘制角点查询结果
            cv::drawChessboardCorners(cimg, cv::Size(nx, ny), corners[lr], found[lr]);
            cv::imshow("Corners", cimg);
            // 阻塞当前线程，输入ESC键退出程序，其他键处理下一张图片
            if ((cv::waitKey(0) & 255) == 27) {
                exit(-1);
            }
        } else {
            std::cout << '.';
        }
        // 对于每个图像对，如果左右图像都成功找到Lee所有角点，则认为是有效图像对，存储模型坐标数据，以及图像对中每个图像内部
        // 基于图像坐标系的坐标数据
        if (lr == 1 && found[0] && found[1]) {
            objectPoints.push_back(boardModel);
            points[0].push_back(corners[0]);
            points[1].push_back(corners[1]);
        }
    }
    // 关闭文件
    fclose(f);

    // 3 标定立体相机
    cv::Mat M1 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat M2 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat D1, D2, R, T, E, F;
    std::cout << "\nRunning stereo calibration ...\n";
    cv::stereoCalibrate(objectPoints, points[0], points[1], M1, D1, M2, D2, imageSize, R, T, E, F,
                        cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST |
                        cv::CALIB_SAME_FOCAL_LENGTH,
                        cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 100, 1e-5));
    std::cout << "Done! Press any key to step through images, ESC to exit\n\n";

    // 标定质量检测
    // 因为基础矩阵隐式包含了所有输出信息，可以使用极线约束【m2^t*F*m1=0】检查标定的质量
    std::vector<cv::Point3f> lines[2];
    double avgErr = 0;
    int nframes = (int)objectPoints.size();
    for (int i = 0; i < nframes; i++) {
        std::vector<cv::Point2f> &pt0 = points[0][i];
        std::vector<cv::Point2f> &pt1 = points[1][i];
        cv::undistortPoints(pt0, pt0, M1, D1, cv::Mat(), M1);
        cv::undistortPoints(pt1, pt1, M2, D2, cv::Mat(), M2);
        cv::computeCorrespondEpilines(pt0, 1, F, lines[0]);
        cv::computeCorrespondEpilines(pt1, 2, F, lines[1]);
        for (int j = 0; j < N; j++) {
            double err = fabs(pt0[j].x * lines[1][j].x + pt0[j].y * lines[1][j].y + lines[1][j].z) +
                         fabs(pt1[j].x * lines[0][j].x + pt1[j].y * lines[0][j].y + lines[0][j].z);
            avgErr += err;
        }
    }
    std::cout << "avg err = " << avgErr / (nframes * N) << std::endl;

    // 4 立体矫正
    // 计算校正后的结果和视差图
    // 是否需要计算未畸变的结果
    bool showUndistorted = true;
    // 立体相机系统的两个摄像机是否是垂直排列的
    bool isVerticalStereo = false;
    if (showUndistorted) {
        // 校正矩阵R1、R2，立体相机校正内参矩阵P1、P2，校正后的映射矩阵map11, map12, map21, map22
        cv::Mat R1, R2, P1, P2, map11, map12, map21, map22;
        if (!useUncalibrated) {
            // 使用经过标定的数据计算立体系统校正参数（BOUGUET的算法）
            // 校正立体系统
            stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, cv::noArray(), 0);
            isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));
            // 计算校正映射表
            initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
            initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);
        } else {
            // 使用未经过标定的数据计算立体系统校正参数（HARTLEY的算法）
            // 使用相机自身的内参矩阵，直接使用基础矩阵计算单应矩阵，从而计算出校正矩阵
            std::vector<cv::Point2f> allpoints[2];
            // 将左右图像的所有帧数据全部集中在一起，从而计算单应矩阵
            for (int i = 0; i < nframes; i++) {
                copy(points[0][i].begin(), points[0][i].end(), back_inserter(allpoints[0]));
                copy(points[1][i].begin(), points[1][i].end(), back_inserter(allpoints[1]));
            }
            // 计算基础矩阵
            cv::Mat F = findFundamentalMat(allpoints[0], allpoints[1], cv::FM_8POINT);
            cv::Mat H1, H2;
            // 计算单应矩阵
            cv::stereoRectifyUncalibrated(allpoints[0], allpoints[1], F, imageSize, H1, H2, 3);
            // 计算矫正矩阵
            R1 = M1.inv() * H1 * M1;
            R2 = M2.inv() * H2 * M2;
            // 计算校正映射表
            cv::initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
            cv::initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);
        }

        // 校正图像并计算视差图
        // 准备一个新的图像能够装下两个相机独立采集数据的立体校正结果，从而形象的演示立体校正后的行对齐现象
        cv::Mat pair;
        if (!isVerticalStereo) {
            pair.create(imageSize.height, imageSize.width * 2, CV_8UC3);
        } else {
            pair.create(imageSize.height * 2, imageSize.width, CV_8UC3);
        }

        // 定义半全局匹配算法视差计算功能类实例
        cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(-64, 128, 11, 100, 1000, 32, 0, 15, 1000, 16, cv::StereoSGBM::MODE_HH);
        for (int i = 0; i < nframes; i++) {
            // 读取原始图像
            cv::Mat img1 = cv::imread(imageNames[0][i].c_str(), 0);
            cv::Mat img2 = cv::imread(imageNames[1][i].c_str(), 0);
            if (img1.empty() || img2.empty()) {
                // 读取失败时处理下一组图像对
                continue;
            }
            
            // 计算立体矫正后的图像
            cv::Mat img1r, img2r, disp, vdisp;
            cv::remap(img1, img1r, map11, map12, cv::INTER_LINEAR);
            cv::remap(img2, img2r, map21, map22, cv::INTER_LINEAR);
            
            // 5 视差图计算
            // 只在水平排列的相机系统中，或者使用已经标定好的相机进行立体校正的情况(非Hartley算法)，才计算深度图
            if (!isVerticalStereo || !useUncalibrated) {
                // 对于垂直排列的相机系统，Hartley不会对对图像应用转置处理，因此在校正后的图像内部极线是垂直的。
                // 立体匹配函数无法处理这种场景
                stereo->compute(img1r, img2r, disp);
                cv::normalize(disp, vdisp, 0, 256, cv::NORM_MINMAX, CV_8U);
                cv::imshow("disparity", vdisp);
            }
            
            if (!isVerticalStereo) {
                // 水平情况下，沿着水平方向绘制扫描线
                // 绘制第一张图像
                cv::Mat part = pair.colRange(0, imageSize.width);
                cvtColor(img1r, part, cv::COLOR_GRAY2BGR);
                // 绘制第二张图像
                part = pair.colRange(imageSize.width, imageSize.width * 2);
                cvtColor(img2r, part, cv::COLOR_GRAY2BGR);
                // 绘制水平扫描线
                for (int j = 0; j < imageSize.height; j += 16) {
                    cv::line(pair, cv::Point(0, j), cv::Point(imageSize.width * 2, j),
                             cv::Scalar(0, 255, 0));
                }
            } else {
                // 垂直情况下，沿着垂直方向绘制扫描线
                // 绘制第一张图像
                cv::Mat part = pair.rowRange(0, imageSize.height);
                cv::cvtColor(img1r, part, cv::COLOR_GRAY2BGR);
                // 绘制第二张图像
                part = pair.rowRange(imageSize.height, imageSize.height * 2);
                cv::cvtColor(img2r, part, cv::COLOR_GRAY2BGR);
                // 绘制垂直扫描线
                for (int j = 0; j < imageSize.width; j += 16) {
                    cv::line(pair, cv::Point(j, 0), cv::Point(j, imageSize.height * 2),
                             cv::Scalar(0, 255, 0));
                }
            }
            // 显示立体校正后的图像对
            cv::imshow("rectified", pair);
            // 阻塞当前线程，等待用户输入事件
            if ((cv::waitKey() & 255) == 27) {
                break;
            }
        }
    }
}



int main(int argc, const char * argv[]) {
    help(argv);
    int board_w = 9, board_h = 6;
    const char *board_list = "../stereoData/stereoData_list.txt";
    if (argc == 4) {
        board_list = argv[1];
        board_w = atoi(argv[2]);
        board_h = atoi(argv[3]);
    }
    StereoCalib(board_list, board_w, board_h, true);
    return 0;
}
