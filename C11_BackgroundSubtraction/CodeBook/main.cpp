//
//  main.cpp
//  CodeBook
//
//  Created by chenjie on 2020/9/4.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



//Always 3 because yuv
#define CHANNELS 3
// IF pixel is within this bound outside of codebook, learn it, else form new code
int cbBounds[CHANNELS];
// 通常需要设置背景的误差容许范围，使得提取出的前景物体更明显
// If pixel is lower than a codebook by this amount, it's matched
int minMod[CHANNELS];
// If pixel is high than a codebook by this amount, it's matched
int maxMod[CHANNELS];



//Here’s how the actual codebook elements are described:
class CodeElement {
    public:
        // High side threshold for learning
        unsigned char learnHigh[CHANNELS];
        // Low side threshold for learning
        unsigned char learnLow[CHANNELS];
        // High side of box boundary
        unsigned char max[CHANNELS];
        // Low side of box boundary
        unsigned char min[CHANNELS];
        // Allow us to kill stale entries
        // 最近更新对应的模型训练索引，即视频帧索引
        int t_last_update;
        // max negative run (longest period of inactivity)
        // 最大空闲时间，即在模型训练时最大经历多少帧仍未被更新
        int stale;

        CodeElement() {
            for (int i = 0; i < CHANNELS; i++) {
                learnHigh[i] = learnLow[i] = max[i] = min[i] = 0;
            }
            t_last_update = stale = 0;
        }
        
        CodeElement& operator = (const CodeElement& ce) {
            for (int i = 0; i < CHANNELS; i++) {
                learnHigh[i] = ce.learnHigh[i];
                learnLow[i] = ce.learnLow[i];
                min[i] = ce.min[i];
                max[i] = ce.max[i];
            }
            t_last_update = ce.t_last_update;
            stale = ce.stale;
            return *this;
        }
        
        CodeElement(const CodeElement& ce) {
            *this = ce;
        }
};



// The variable t counts the number of points we’ve accumulated since the start or the last clear operation.
// You need one of these for each pixel in the video image (rowXcol)
class CodeBook : public std::vector<CodeElement> {
    public:
    //Count of every image learned on
    int t;
    
    // count every access
    CodeBook() {
        t = 0;
    }
    
    // Default is an empty book
    // Construct book of size n
    CodeBook(int n) : std::vector<CodeElement>(n) {
        t = 0;
    }
};



/// Updates the codebook entry with a new data point
/// Note: cbBounds must be of length equal to numChannels
/// Return CodeBook index
/// @param p incoming YUV pixel
/// @param c CodeBook for the pixel
/// @param cbBounds Bounds for codebook (usually: {10,10,10})
/// @param numChannels Number of color channels we're learning
int updateCodebook(const cv::Vec3b& p, CodeBook& c, int* cbBounds, int numChannels) {
    if (c.size() == 0) {
        c.t = 0;
    }
    //Record learning event
    c.t += 1;
    //SET HIGH AND LOW BOUNDS
    unsigned int high[3], low[3];
    for (int i = 0; i < numChannels; i++) {
        high[i] = p[i] + *(cbBounds + i);
        if (high[i] > 255) {
            high[i] = 255;
        }
        low[i] = p[i] - *(cbBounds + i);
        if (low[i] < 0) {
            low[i] = 0;
        }
    }

    // SEE IF THIS FITS AN EXISTING CODEWORD
    int matchChannel;
    bool elementIsExist = false;
    for (int i = 0; i < c.size(); i++) {
        matchChannel = 0;
        for (int j = 0; j < numChannels; j++) {
            // Found an entry for this channel
            if ((c[i].learnLow[j] <= p[j]) && (p[j] <= c[i].learnHigh[j])) {
                matchChannel++;
            }
        }

        // If an entry was found
        if (matchChannel == numChannels) {
            elementIsExist = true;
            c[i].t_last_update = c.t;

            // adjust this codeword for the first channel
            for (int j = 0; j < numChannels; j++) {
                if (c[i].max[j] < p[j]) {
                    c[i].max[j] = p[j];
                } else if (c[i].min[j] > p[j]) {
                    c[i].min[j] = p[j];
                }

                // SLOWLY ADJUST LEARNING BOUNDS
                if (c[i].learnHigh[j] < high[j]) {
                    c[i].learnHigh[j] += 1;
                }
                if (c[i].learnLow[j] > low[j]) {
                    c[i].learnLow[j] -= 1;
                }
            }
            break;
        }
    }

    // OVERHEAD TO TRACK POTENTIAL STALE ENTRIES
    for (int i = 0; i < c.size(); i++) {
        // Track which codebook entries are going stale:
        int negRun = c.t - c[i].t_last_update;
        if (c[i].stale < negRun) {
            c[i].stale = negRun;
        }
    }

    // ENTER A NEW CODEWORD IF NEEDED
    if (!elementIsExist) {
        // if no existing codeword found, make one
        CodeElement ce;
        for (int i = 0; i < numChannels; i++) {
            ce.learnHigh[i] = high[i];
            ce.learnLow[i] = low[i];
            ce.max[i] = p[i];
            ce.min[i] = p[i];
        }

        ce.t_last_update = c.t;
        ce.stale = 0;
        c.push_back(ce);
    }

    return int(c.size());
}



// During learning, after you've learned for some period of time,
// periodically call this to clear out stale codebook entries
// 长时间未被更新的码书元素可以认为是背景中偶尔出现的前景物体，这些元素需要删除
int foo = 0;
/// return number of entries cleared
/// @param c Codebook to clean up
int clearStaleEntries(CodeBook &c) {
    // 设置一个不活跃的时间阈值
    int staleThresh = c.t >> 1;
    // 标记数组
    int *keep = new int[c.size()];
    int keepCnt = 0;
    
    // SEE WHICH CODEBOOK ENTRIES ARE TOO STALE
    for (int i = 0; i < c.size(); i++) {
        if (c[i].stale > staleThresh) {
            // Mark for destruction
            keep[i] = 0;
        } else {
            // Mark to keep
            keep[i] = 1;
            keepCnt += 1;
        }
    }
    
    // move the entries we want to keep to the front of the vector and then
    // truncate to the correct length once all of the good stuff is saved.
    int k = 0;
    int numCleared = 0;
    for (int i = 0; i < c.size(); i++) {
        if (keep[i]) {
            c[k] = c[i];
            // We have to refresh these entries for next clearStale
            c[k].t_last_update = 0;
            k++;
        } else {
            numCleared++;
        }
    }
    c.resize(keepCnt);
    delete[] keep;
    
    return numCleared;
}



/// Given a pixel and a codebook, determine whether the pixel is covered by the codebook
/// NOTES:
/// minMod and maxMod must have length numChannels,
/// e.g. 3 channels => minMod[3], maxMod[3]. There is one min and one max threshold per channel.
///
/// return 0 => background, 255 => foreground
/// @param p Pixel (YUV)
/// @param c Codebook
/// @param numChannels Number of channels we are testing
/// @param minMod_ Add this (possibly negative) number onto max level when determining whether new pixel is foreground
/// @param maxMod_ Subtract this (possibly negative) number from min level when determining whether new pixel is foreground
uchar backgroundDiff(const cv::Vec3b& p, CodeBook& c, int numChannels, int* minMod_, int* maxMod_) {
    int matchChannel;
    
    // SEE IF THIS FITS AN EXISTING CODEWORD
    int i;
    for (i = 0; i < c.size(); i++) {
        matchChannel = 0;
        for (int n = 0; n < numChannels; n++) {
            if((c[i].min[n] - minMod_[n] <= p[n]) && (p[n] <= c[i].max[n] + maxMod_[n])) {
                matchChannel++; // Found an entry for this channel
            } else {
                break;
            }
        }
        // Found an entry that matched all channels
        if (matchChannel == numChannels) {
            break;
        }
    }
    
    // No match with codebook => foreground
    if (i >= c.size()) {
        return 255;
    }
    // Else background
    return 0;
}



// Just make a convienience class (assumes image will not change size in video);
class CbBackgroudDiff {
    public:
    // Will hold the yuv converted image
    cv::Mat Iyuv;
    // Will hold the background difference mask
    // 背景分割的结果
    cv::Mat mask;
    // Will hold a CodeBook for each pixel
    std::vector<CodeBook> codebooks;
    // How many pixels are in the image
    int row, col, image_length;
    
    //Constructor
    void init(cv::Mat &I_from_video) {
        // 设置全局变量的值
        std::vector<int> v(3,10);
        set_global_vecs(cbBounds, v);
        // Just some decent defaults for low side
        v[0] = 6; v[1] = 20; v[2] = 8;
        set_global_vecs(minMod, v);
        // Decent defaults for high side
        v[0] = 70; v[1] = 20; v[2] = 6;
        set_global_vecs(maxMod, v);
        
        // 创建图像
        Iyuv.create(I_from_video.size(), I_from_video.type());
        mask.create(I_from_video.size(), CV_8UC1);
        
        // 为属性赋值
        row = I_from_video.rows;
        col = I_from_video.cols;
        image_length = row * col;
        std::cout << "(row,col,len) = (" << row << ", " << col << ", " << image_length << ")" << std::endl;
        
        // 根据单幅图片的像素数，为码书数组分配空间
        codebooks.resize(image_length);
    }
    
    // 构造函数
    CbBackgroudDiff(cv::Mat &I_from_video) {
        init(I_from_video);
    }
    CbBackgroudDiff(){};
    
    // Convert to YUV
    void convert_to_yuv(cv::Mat &Irgb) {
        cvtColor(Irgb, Iyuv, cv::COLOR_BGR2YUV);
    }
    
    // Check that image doesn't change size, return -1 if size doesn't match, else 0
    int size_check(cv::Mat &I) {
        int ret = 0;
        if ((row != I.rows) || (col != I.cols)) {
            std::cerr << "ERROR: Size changed! old[" << row << ", " << col << "], now [" << I.rows << ", " << I.cols << "]!" << std::endl;
            ret = -1;
        }
        return ret;
    }
    
    // Utilities for setting gloabals
    void set_global_vecs(int *globalvec, std::vector<int> &vec) {
        if (vec.size() != CHANNELS) {
            std::cerr << "Input vec[" << vec.size() << "] should equal CHANNELS [" << CHANNELS << "]" << std::endl;
            vec.resize(CHANNELS, 10);
        }
        int i = 0;
        for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it, ++i) {
             globalvec[i] = *it;
         }
     }

    // Background operations
    // Learn codebook, -1 if error, else total # of codes
    int updateCodebookBackground(cv::Mat &Irgb) {
        convert_to_yuv(Irgb);
        if (size_check(Irgb)) {
            return -1;
        }
        
        // 码书元素的总数量
        int total_codebooks = 0;
        // 获取图片像素迭代器，和码书迭代器，它们的数量是相等的
        cv::Mat_<cv::Vec3b>::iterator Iit = Iyuv.begin<cv::Vec3b>(), IitEnd = Iyuv.end<cv::Vec3b>();
        std::vector<CodeBook>::iterator Cit = codebooks.begin(), CitEnd = codebooks.end();
        for (; Iit != IitEnd; ++Iit,++Cit) {
            // 为每个像素训练码书
            total_codebooks += updateCodebook(*Iit, *Cit, cbBounds, CHANNELS);
        }
        
        if (Cit != CitEnd) {
            std::cerr << "ERROR: Cit != CitEnd in updateCodeBackground(...) " << std::endl;
        }
        return(total_codebooks);
    }

    // Clean out stuff that hasn't been updated for a long time
    int clearStaleEntriesBackground() {
        // 总共被清理的码书元素数量
        int total_cleared = 0;
        std::vector<CodeBook>::iterator Cit = codebooks.begin(), CitEnd = codebooks.end();
        for(; Cit != CitEnd; ++Cit) {
            total_cleared += clearStaleEntries(*Cit);
        }
        return(total_cleared);
    }

    // Take the background difference of the image
    // 提取前景图像
    int backgroundDiffBackground(cv::Mat &Irgb) {
        convert_to_yuv(Irgb);
        if (size_check(Irgb)) {
            return -1;
        }
        
        // 获取图片像素迭代器，和码书迭代器，和前景模版迭代器，它们的数量是相等的
        cv::Mat_<cv::Vec3b>::iterator Iit = Iyuv.begin<cv::Vec3b>(), IitEnd = Iyuv.end<cv::Vec3b>();
        std::vector<CodeBook>::iterator Cit = codebooks.begin(), CitEnd = codebooks.end();
        cv::Mat_<uchar>::iterator Mit = mask.begin<uchar>(), MitEnd = mask.end<uchar>();
        
        // 计算前景模版
        for (; Iit != IitEnd; ++Iit,++Cit,++Mit) {
            *Mit = backgroundDiff(*Iit, *Cit, CHANNELS, minMod, maxMod);
        }
        if ((Cit != CitEnd) || (Mit != MitEnd)) {
            std::cerr << "ERROR: Cit != CitEnd and, or Mit != MitEnd in updateCodeBackground(...) " << std::endl;
            return -1;
        }
        return 0;
    }
};



void help(const char * argv[]) {
    std::cout << "\n"
    << "Train a codebook background model on the first <#frames to train on> frames of an incoming video, then run the model\n"
    << argv[0] <<" <#frames to train on> <avi_path/filename>\n"
    << "For example:\n"
    << argv[0] << " 50 ../tree.avi\n"
    << "'A' or 'a' to adjust thresholds, esc, 'q' or 'Q' to quit"
    << std::endl;
}



// Adjusting the distance you have to be on the low side (minMod) or high side (maxMod) of a codebook
// to be considered as recognized/background
void adjustThresholds(char* charstr, cv::Mat &Irgb, CbBackgroudDiff &bgd) {
    int key = 1;
    // 判定调节的通道和通道索引
    int y = 1, u = 0, v = 0, index = 0;
    // 该值为1时调整的是阈值maxMod，否则调整的是minMod
    int thresh = 0;
    if (thresh) {
        std::cout << "yuv[" << y << "][" << u << "][" << v << "] maxMod active" << std::endl;
    } else {
        std::cout << "yuv[" << y << "][" << u << "][" << v << "] minMod active" << std::endl;
    }
    std::cout << "minMod[" << minMod[0] << "][" << minMod[1] << "][" << minMod[2] << "]" << std::endl;
    std::cout << "maxMod[" << maxMod[0] << "][" << maxMod[1] << "][" << maxMod[2] << "]" << std::endl;
    
    // Esc or Q or q to exit
    while ((key = cv::waitKey()) != 27 && key != 'Q' && key != 'q') {
        if (thresh) {
            std::cout << "yuv[" << y << "][" << u << "][" << v << "] maxMod active" << std::endl;
        } else {
            std::cout << "yuv[" << y << "][" << u << "][" << v << "] minMod active" << std::endl;
        }
        std::cout << "minMod[" << minMod[0] << "][" << minMod[1] << "][" << minMod[2] << "]" <<std::endl;
        std::cout << "maxMod[" << maxMod[0] << "][" << maxMod[1] << "][" << maxMod[2] << "]" << std::endl;
        
        if (key == 'y') {
            y = 1; u = 0; v = 0; index = 0;
        }
        if (key == 'u') {
            y = 0; u = 1; v = 0; index = 1;
        }
        if (key == 'v') {
            y = 0; u = 0; v = 1; index = 2;
        }
        // minMod
        if (key == 'l') {
            thresh = 0;
        }
        // maxMod
        if (key == 'h') {
            thresh = 1;
        }
        // Up
        if (key == '.') {
            if (thresh == 0) {
                minMod[index] += 4;
            }
            if (thresh == 1) {
                maxMod[index] += 4;
            }
        }
        //Down
        if (key == ',') {
            if (thresh == 0) {
                minMod[index] -= 4;
            }
            if (thresh == 1) {
                maxMod[index] -= 4;
            }
        }
        
        std::cout << "y,u,v for channel; l for minMod, h for maxMod threshold; , for down, . for up; esq or q to quit;" << std::endl;
        bgd.backgroundDiffBackground(Irgb);
        cv::imshow(charstr, bgd.mask);
    }
}



int main(int argc, const char * argv[]) {
    // 验证参数合法
    if (argc < 3) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }

    // 验证视频文件有效
    cv::VideoCapture cap = cv::VideoCapture(argv[2]);
    if (!cap.isOpened()) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }
    
    // 1 训练背景模型
    // 需要训练的视频帧数
    int number_to_train_on = atoi(argv[1]);
    std::cout << "Total frames to train on = " << number_to_train_on << std::endl;
    // 创建视频播放窗口
    cv::namedWindow(argv[0], cv::WINDOW_AUTOSIZE);
    
    // FIRST PROCESSING LOOP (TRAINING):
    int frameIndex = 0;
    cv::Mat image;
    CbBackgroudDiff bgd;

    while (1) {
        std::cout << "frame#: " << frameIndex;
        cap >> image;
        if (!image.data) {
            // Something went wrong, abort
            exit(1);
        }
        
        if (frameIndex == 0) {
            // 1.1 首帧时创建背景模型
            bgd.init(image);
        }
        
        // 1.2 更新背景模型
        std::cout << ", Codebooks: " << bgd.updateCodebookBackground(image) << std::endl;
        
        // 1。3 显示原图
        cv::imshow(argv[0], image);
        frameIndex++;
        
        int key = cv::waitKey(7);
        // Allow early exit on space, esc, q
        if (key == 27 || key == 'q' || key == 'Q' || frameIndex >= number_to_train_on) {
            break;
        }
    }
    
    // 2 使用模型提取前景物体
    // We have accumulated our training, now create the models
    std::cout << "Created the background model" << std::endl;
    std::cout << "Total entries cleared = " << bgd.clearStaleEntriesBackground() << std::endl;
    std::cout << "Press a key to start background differencing, 'a' to set thresholds, esc or q or Q to quit" << std::endl;

    char seg[] = "Segmentation";
    cv::namedWindow(seg, cv::WINDOW_AUTOSIZE);
    // SECOND PROCESSING LOOP (TESTING):
    int key = cv::waitKey();
    // esc, 'q' or 'Q' to exit
    while (key != 27 || key == 'q' || key == 'Q') {
        cap >> image;
        if (!image.data) {
            exit(0);
        }
        
        // 2.1 提取前景对象
        std::cout << frameIndex++ << " 'a' to adjust threholds" << std::endl;
        if (key == 'a') {
            // 调整阈值并提取前景对象
            std::cout << "Adjusting thresholds" << std::endl;
            std::cout << "y,u,v for channel; l for minMod, h for maxMod threshold; , for down, . for up; esq or q to quit;" << std::endl;
            adjustThresholds(seg, image, bgd);
        } else {
            // 提取前景对下
            if (bgd.backgroundDiffBackground(image)) {
                std::cerr << "ERROR, bdg.backgroundDiffBackground(...) failed" << std::endl;
                exit(-1);
            }
        }
        // 2.2 显示前景对象
        cv::imshow("Segmentation", bgd.mask);
        
        // 挂起程序
        key = cv::waitKey();
    }
    return 0;
}
