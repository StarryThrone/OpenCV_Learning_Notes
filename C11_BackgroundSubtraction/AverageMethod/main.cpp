//
//  main.cpp
//  AverageMethod
//
//  Created by chenjie on 2020/7/31.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

// Global storage
// Float, 3-channel images needed by learning model.
cv::Mat previousFrame;
cv::Mat averageFrame, differenceFrame;
cv::Mat upperLimitFrame, lowerLimitFrame;
cv::Mat forgroundFrame;
cv::Mat tmpFrame, tmpFrame2;

// Float, 1-channel images needed by learning model.
// Present each channel respectively.
std::vector<cv::Mat> channelFrames(3);
std::vector<cv::Mat> channelLowFrames(3);
std::vector<cv::Mat> channelHighFrames(3);

// Thresholds
// scaling the thesholds in backgroundDiff()
float highThreshFactor = 20.0;
float lowThreshFactor = 28.0;

// Counts number of images learned for averaging later
float learnedImageCount = 0;

/// Learn the background statistics for one more frame
/// @param backgroundFrame backgroundFrame is a color sample of the background, 3-channel, 8u
void accumulateBackground(cv::Mat & backgroundFrame) {
    // nb. Not thread safe
    static int first = 1;
    // convert to float
    backgroundFrame.convertTo(tmpFrame, CV_32F);
    if (!first) {
        // 因为需要计算差分，因此首帧不处理
        averageFrame += tmpFrame;
        cv::absdiff(tmpFrame, previousFrame, tmpFrame2);
        differenceFrame += tmpFrame2;
        learnedImageCount += 1.0;
    }
    first = 0;
    previousFrame = tmpFrame;
}

void updateHighThreshold(float scale) {
    upperLimitFrame = averageFrame + (differenceFrame * scale);
    cv::split(upperLimitFrame, channelHighFrames);
}

void updateLowThreshold(float scale) {
    lowerLimitFrame = averageFrame - (differenceFrame * scale);
    cv::split(lowerLimitFrame, channelLowFrames);
}

void createModelsfromStats() {
    averageFrame *= (1.0 / learnedImageCount);
    differenceFrame *= (1.0 / learnedImageCount);
    
    // Make sure diff is always something
    differenceFrame += cv::Scalar(1.0, 1.0, 1.0);
    updateHighThreshold(highThreshFactor);
    updateLowThreshold(lowThreshFactor);
}

// Byte, 1-channel image
cv::Mat backgroundTempFrame;
/// Create a binary: 0,255 mask where 255 (red) means foreground pixel
/// @param sourceFrame Input image, 3-channel, 8u
/// @param forgroundFrame Foreground image to be created, 1-channel 8u
void backgroundDiff(cv::Mat& sourceFrame, cv::Mat& forgroundFrame) {
    cv::Mat backgroundFrame = forgroundFrame;
    sourceFrame.convertTo(tmpFrame, CV_32F); // To float
    cv::split(tmpFrame, channelFrames);
    
    // Channel 1
    cv::inRange(channelFrames[0], channelLowFrames[0], channelHighFrames[0], backgroundFrame);
    // Channel 2
    cv::inRange(channelFrames[1], channelLowFrames[1], channelHighFrames[1], backgroundTempFrame);
    backgroundFrame = cv::min(backgroundFrame, backgroundTempFrame);
    // Channel 3
    cv::inRange(channelFrames[2], channelLowFrames[2], channelHighFrames[2], backgroundTempFrame);
    backgroundFrame = cv::min(backgroundFrame, backgroundTempFrame);

    // Finally, invert the results
    forgroundFrame = 255 - backgroundFrame;
}

void showForgroundInRed(const std::string & winname, const cv::Mat &img) {
    cv::Mat rawImage;
    cv::split(img, channelFrames);
    // Mark red channel of forground parts with maxValue.
    channelFrames[2] = cv::max(forgroundFrame, channelFrames[2]);
    cv::merge(channelFrames, rawImage);
    cv::imshow(winname, rawImage);
    cv::imshow("Segmentation", forgroundFrame);
}

void adjustThresholds(const char * argv[], cv::Mat &img) {
    int key = 1;
    // Esc or Q or q to exit
    while ((key = cv::waitKey()) != 27 && key != 'Q' && key != 'q') {
        if (key == 'L') {
            lowThreshFactor += 0.2;
        }
        if (key == 'l') {
            lowThreshFactor -= 0.2;
        }
        if (key == 'H') {
            highThreshFactor += 0.2;
        }
        if (key == 'h') {
            highThreshFactor -= 0.2;
        }
        std::cout << "H or h, L or l, esq or q to quit;  high_thresh = "
                  << highThreshFactor << ", " << "low_thresh = " << lowThreshFactor << std::endl;
        updateHighThreshold(highThreshFactor);
        updateLowThreshold(lowThreshFactor);
    }
}

void AllocateImages(const cv::Size & size) {
    averageFrame = cv::Mat::zeros(size, CV_32FC3);
    differenceFrame = cv::Mat::zeros(size, CV_32FC3);
    previousFrame = cv::Mat::zeros(size, CV_32FC3);
    upperLimitFrame = cv::Mat::zeros(size, CV_32FC3);
    lowerLimitFrame = cv::Mat::zeros(size, CV_32FC3);
    tmpFrame = cv::Mat::zeros(size, CV_32FC3);
    tmpFrame2 = cv::Mat::zeros(size, CV_32FC3);
    backgroundTempFrame = cv::Mat(size, CV_32FC1);
}

void help(const char * argv[]) {
    std::cout << "\n"
              << "Train a background model on  the first <#frames to train on> frames of an incoming video, then run the model\n"
              << argv[0] << " <#frames to train on> <avi_path/filename>\n"
              << "For example:\n"
              << argv[0] << " 50 ../tree.avi\n"
              << "'A' or 'a' to adjust thresholds, esc, 'q' or 'Q' to quit"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }

    cv::VideoCapture cap = cv::VideoCapture(argv[2]);
    if (!cap.isOpened()) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }
    
    cv::namedWindow(argv[0], cv::WINDOW_AUTOSIZE);
    cv::Mat currentFrame;
    int frameIndex = 0;
    int key;
    int number_to_train_on = atoi(argv[1]);
    // Train model until max iteration count or quite event triggered by user
    while (1) {
        std::cout << "Learning frame#: " << frameIndex << std::endl;
        cap >> currentFrame;
        // Something went wrong, abort
        if (!currentFrame.data) {
            exit(1);
        }
        if (frameIndex == 0) {
            AllocateImages(currentFrame.size());
        }
        
        // Train Model
        accumulateBackground(currentFrame);
        // Play video by showing video frame
        cv::imshow(argv[0], currentFrame);
        frameIndex++;
        
        key = cv::waitKey(7);
        if (key == 27 || key == 'q' || key == 'Q' || frameIndex >= number_to_train_on) {
            // Allow early exit on space, esc, q
            break;
        }
    }
    
    // FIRST PROCESSING LOOP (TRAINING):
    std::cout << "Total frames to train on = " << frameIndex + 1 << std::endl;
    // We have accumulated our training, now create the models
    std::cout << "Creating the background model" << std::endl;
    createModelsfromStats();
    std::cout << "Done!  Hit any key to continue into single step. "
              << "Hit 'a' or 'A' to adjust thresholds, esq, 'q' or 'Q' to quit\n"
              << std::endl;

    // SECOND PROCESSING LOOP (TESTING):
    cv::namedWindow("Segmentation", cv::WINDOW_AUTOSIZE);
    while (1) {
        key = cv::waitKey(0);

        // Read image data
        cap >> currentFrame;
        if (!currentFrame.data) {
            exit(0);
        }
        
        if (key == 'a') {
            // Adjust threshold if needed
            std::cout << "In adjust thresholds, 'H' or 'h' == high thresh up or down;"
                      << " 'L' or 'l' for low thresh up or down."
                      << std::endl;
            std::cout << " esq, 'q' or 'Q' to quit " << std::endl;
            adjustThresholds(argv, currentFrame);
            std::cout << "Done with adjustThreshold, back to frame stepping, esq, q or Q to quit." << std::endl;
        }
        
        
        std::cout << "Processing frame: " << frameIndex++ << std::endl;
        backgroundDiff(currentFrame, forgroundFrame);
        // A simple visualization is to write to the red channel
        showForgroundInRed(argv[0], currentFrame);
        
        if (key == 27 || key == 'q' || key == 'Q') {
            // esc, 'q' or 'Q' to exit
            break;
        }
    }

    return 0;
}


//#include <opencv2/opencv.hpp>
//#include <cstdlib>
//#include <fstream>
