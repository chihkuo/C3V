#ifndef __OPENCV_UTILITY_H__
#define __OPENCV_UTILITY_H__

#include <vector>
#include <random>
#include <opencv2/opencv.hpp>

extern std::vector<cv::Mat> custom_colormap;

int initializeColormap();
void bgr2uyvy(const cv::Mat& bgrImage, cv::Mat& uyvyImage);
void bgr2yuy2(const cv::Mat& bgrImage, cv::Mat& yuy2Image);
void bgr2nv12(const cv::Mat& bgrImage, cv::Mat& nv12Image);
bool convert_to_bgr(unsigned char* src, int width, int height, const char* format, cv::Mat &outputMat);
bool convert_from_bgr(cv::Mat& inputMat, cv::Mat& outputMat, const char* format);
std::vector<cv::Scalar> generateRandomRGB(int count);

#endif
