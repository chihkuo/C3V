#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace YoloInfo {
    void initColors();
    const std::string& getClassName(int class_type_index, int class_id);
    const cv::Scalar& getColor(int class_id);
    std::size_t getClassSize(int class_type_index);
}
