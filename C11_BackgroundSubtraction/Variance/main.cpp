//
//  main.cpp
//  Variance
//
//  Created by chenjie on 2020/8/3.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// MARK:- Average value and variance value.
// Store accumalative sum and square of correspoding pixels intensity in each video frame
cv::Mat sum, sqsum;
int trainedImageCount = 0;

/// Accumulate the information we need for our variance computation.
/// @param mat Image mat needed to accumulate into result.
void accumulateVariance(cv::Mat& mat) {
    if (sum.empty()) {
        sum = cv::Mat::zeros(mat.size(), CV_32FC(mat.channels()));
        sqsum = cv::Mat::zeros(mat.size(), CV_32FC(mat.channels()));
        trainedImageCount = 0;
    }
    cv::accumulate(mat, sum);
    cv::accumulateSquare(mat, sqsum);
    trainedImageCount++;
}

/// Compute standard deviation.
/// @param stDev Standard deviation that store the computed result.
void computeStdev(cv::Mat& stDev) {
    double scaleFactor = 1.0 / trainedImageCount;
    cv::sqrt(((scaleFactor * sqsum) - ((scaleFactor * scaleFactor) * sum.mul(sum))), stDev);
}

// Compute average value.
void computeAvg(cv::Mat& av) {
    double scaleFactor = 1.0 / trainedImageCount;
    av = scaleFactor * sum;
}

// Global storage
// Float, 3-channel images
cv::Mat averageFrame;
// Standard deviation frame.
cv::Mat stdDevFrame;
// Threshold
cv::Mat upperLimitFrame, lowerLimitFrame;
//scratch and our mask
cv::Mat forgroundFrame;

// Float, 1-channel images
// Store each channel data of a frame.
std::vector<cv::Mat> channelFrames(3);
// Store lower limit intensity of pixels in background model frame
std::vector<cv::Mat> channelLowFrames(3);
// Store upper limit intensity of pixels in background model frame
std::vector<cv::Mat> channelHighFrames(3);

// Thresholds
float high_thresh = 21.0;
float low_thresh = 2.0;

void setHighThreshold(float scale) {
    upperLimitFrame = averageFrame + (stdDevFrame * scale);
    cv::split(upperLimitFrame, channelHighFrames);
}

void setLowThreshold(float scale) {
    lowerLimitFrame = averageFrame - (stdDevFrame * scale);
    cv::split(lowerLimitFrame, channelLowFrames);
}

void createModelsfromStats() {
    // Make sure diff is always something
    stdDevFrame += cv::Scalar(0.1, 0.1, 0.1);
    setHighThreshold(high_thresh);
    setLowThreshold(low_thresh);
}

// Byte, 1-channel image
cv::Mat backgroundTempFrame;
cv::Mat tmpFrame;
/// Create a binary: 0,255 mask where 255 (red) means foreground pixel
/// @param sourceFrame Input image, 3-channel, 8u
/// @param forgroundFrame Mask image to be created, 1-channel 8u
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

void showForgroundInRed(const char * argv[], const cv::Mat &img) {
    cv::Mat rawImage;
    cv::split(img, channelFrames);
    channelFrames[2] = cv::max(forgroundFrame, channelFrames[2]);
    cv::merge(channelFrames, rawImage);
    cv::imshow(argv[0], rawImage);
    cv::imshow("Segmentation", forgroundFrame);
}

void adjustThresholds(const char * argv[], cv::Mat &img) {
    int key = cv::waitKey();
    while (key != 27 && key != 'Q' && key != 'q') {
        // Esc or Q or q to exit
        switch (key) {
            case 'L':
                low_thresh += 0.2;
                break;
            case 'l':
                low_thresh -= 0.2;
                break;
            case 'H':
                high_thresh += 0.2;
                break;
            case 'h':
                high_thresh -= 0.2;
                break;
            default:
                break;
        }
        std::cout << "H or h, L or l, esq or q to quit;  high_thresh = "
                  << high_thresh << ", " << "low_thresh = " << low_thresh << std::endl;
        
        setHighThreshold(high_thresh);
        setLowThreshold(low_thresh);
        
        backgroundDiff(img, forgroundFrame);
        showForgroundInRed(argv, img);
    }
}

// I is just a sample image for allocation purposes
void AllocateImages(const cv::Size& size) {
    averageFrame = cv::Mat::zeros(size, CV_32FC3);
    stdDevFrame = cv::Mat::zeros(size, CV_32FC3);
    upperLimitFrame = cv::Mat::zeros(size, CV_32FC3);
    lowerLimitFrame = cv::Mat::zeros(size, CV_32FC3);
    tmpFrame = cv::Mat::zeros(size, CV_32FC3);
    backgroundTempFrame = cv::Mat(size, CV_32FC1);
}

void help(const char * argv[]) {
    std::cout << "\n"
              << "Compute mean and std on <#frames to train on> frames of an incoming video, then run the model\n"
              << argv[0] <<" <#frames to train on> <avi_path/filename>\n"
              << "For example:\n"
              << argv[0] << " 50 ../tree.avi\n"
              << "'a' to adjust thresholds, esc, 'q' or 'Q' to quit"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    // 1. Validate parameters count.
    if (argc < 3) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }

    // 2. Read video file.
    cv::VideoCapture cap = cv::VideoCapture(argv[2]);
    if (!cap.isOpened()) {
        std::cerr << "Couldn't run the program" << std::endl;
        help(argv);
        return -1;
    }
    
    // 3. Train background model
    // 3.1 Prepare needed variable
    cv::namedWindow(argv[0], cv::WINDOW_AUTOSIZE);
    int number_to_train_on = atoi(argv[1]);
    int frameIndex = 0;
    int key;
    cv::Mat currentFrame;
    std::cout << "Total frames to train on = " << number_to_train_on << std::endl;
    // FIRST PROCESSING LOOP (TRAINING):
    while (1) {
        std::cout << "frame#: " << frameIndex << std::endl;
        // 3.2 Read video frame
        cap >> currentFrame;
        if (!currentFrame.data) {
            // Something went wrong, abort
            exit(1);
        }
        if (frameIndex == 0) {
            AllocateImages(currentFrame.size());
        }
        
        // 3.3 Accumalate pixel data from learned video frames
        accumulateVariance(currentFrame);
        
        cv::imshow(argv[0], currentFrame);
        frameIndex++;
        // Suspend application for seven millisecond.
        key = cv::waitKey(7);
        if (key == 27 || key == 'q' || key == 'Q' || frameIndex >= number_to_train_on) {
            // Allow early exit on space, esc, q
            break;
        }
    }

    // 4. We have accumulated our training, now create the models
    std::cout << "Creating the background model" << std::endl;
    computeAvg(averageFrame);
    computeStdev(stdDevFrame);
    createModelsfromStats();
    std::cout << "Done!  Hit any key to continue into single step. Hit 'a' "
              << "or 'A' to adjust thresholds, esq, 'q' or 'Q' to quit\n" << std::endl;

    // 5. SECOND PROCESSING LOOP (TESTING):
    // For the mask image
    cv::namedWindow("Segmentation", cv::WINDOW_AUTOSIZE);
    while (1) {
        // 5.1 Suspend allication until user trigger any key.
        key = cv::waitKey();
        
        // 5.2 Read current video frame.
        cap >> currentFrame;
        if (!currentFrame.data) {
            exit(0);
        }
        // 5.3 Calculate foreground image
        std::cout << frameIndex++ << std::endl;
        backgroundDiff(currentFrame, forgroundFrame);
        cv::imshow("Segmentation", forgroundFrame);
        // A simple visualization is to write to the red channel
        showForgroundInRed(argv, currentFrame);
        
        if (key == 'a') {
            // Adjust treshold used to calculate foreground image
            std::cout << "In adjust thresholds, 'H' or 'h' == high thresh up or down; "
                      << "'L' or 'l' for low thresh up or down." << std::endl;
            std::cout << " esq, 'q' or 'Q' to quit " << std::endl;
            adjustThresholds(argv, currentFrame);
            std::cout << "Done with adjustThreshold, back to frame"
                      << " stepping, esq, q or Q to quit." << std::endl;
        } else if (key == 27 || key == 'q' || key == 'Q') {
            // esc, 'q' or 'Q' to exit
            break;
        }
    }

    return 0;
}
