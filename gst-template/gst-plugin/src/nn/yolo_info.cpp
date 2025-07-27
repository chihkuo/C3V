#include "yolo_info.h"
#include <random>

namespace {
    std::vector<std::vector<std::string>> g_className = {
        { "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
          "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
          "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
          "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
          "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
          "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
          "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
          "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
          "hair drier", "toothbrush" },
        { "person", "bike", "car", "motor", "bus", "train", "truck", "light", "dog", "scooter", "other vehicle" }
    };

    std::vector<cv::Scalar> g_color;

    std::vector<cv::Scalar> generateRandomRGB(int count) {
        std::vector<cv::Scalar> scalarGroups;
        scalarGroups.reserve(count);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        for (int i = 0; i < count; ++i) {
            int r = dis(gen);
            int g = dis(gen);
            int b = dis(gen);
            scalarGroups.emplace_back(r, g, b);
        }

        return scalarGroups;
    }
}

namespace YoloInfo {

    void initColors() {
        std::cout << "YoloInfo::initColors" << std::endl;
        g_color = generateRandomRGB(80);
    }

    const std::string& getClassName(int class_type_index, int class_id) {
        return g_className[class_type_index][class_id];
    }

    const cv::Scalar& getColor(int class_id) {
        return g_color[class_id];
    }

    std::size_t getClassSize(int class_type_index) {
        return g_className[class_type_index].size();
    }

}