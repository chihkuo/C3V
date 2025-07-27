#include "tracker_algo.h"


Tracker_Algo::Tracker_Algo(int margin, bool useGray)
    : margin_(margin), useGray_(useGray) {}

cv::Rect Tracker_Algo::expandRect(const cv::Rect& rect, int margin) {
    return cv::Rect(rect.x - margin, rect.y - margin, rect.width + 2 * margin, rect.height + 2 * margin);
}

cv::Rect Tracker_Algo::sanitizeRect(const cv::Rect& rect, const cv::Size& imageSize) {
    int x = std::clamp(rect.x, 0, imageSize.width);
    int y = std::clamp(rect.y, 0, imageSize.height);
    int width = std::min(rect.width + rect.x, imageSize.width) - x;
    int height = std::min(rect.height + rect.y, imageSize.height) - y;
    return (width > 0 && height > 0) ? cv::Rect(x, y, width, height) : cv::Rect();
}

bool Tracker_Algo::prepareCrop(const cv::Mat& frame, const cv::Rect& targetROI, cv::Mat& cropped) {
    cropRegion_ = sanitizeRect(expandRect(targetROI, margin_), frame.size());
    if (cropRegion_.empty()) return false;

    offset_ = cropRegion_.tl();
    cropped = frame(cropRegion_);

    if (useGray_ && cropped.channels() == 3) {
        cv::cvtColor(cropped, cropped, cv::COLOR_BGR2GRAY);
    }

    return true;
}

cv::Ptr<cv::Tracker> Tracker_Algo::createTracker(TrackerMode mode)
{
    switch (mode) {
        case TrackerMode::CSRT:
            std::cout << "Tracker Mode: CSRT" << std::endl;
            return cv::TrackerCSRT::create();
        case TrackerMode::KCF:
            std::cout << "Tracker Mode: KCF" << std::endl;
            return cv::TrackerKCF::create();
        case TrackerMode::MIL:
            std::cout << "Tracker Mode: MIL" << std::endl;
            return cv::TrackerMIL::create();
        default:
            throw std::invalid_argument("Unknown tracker mode");
    }
}

bool Tracker_Algo::init(TrackerMode mode, const cv::Mat& frame, const cv::Rect& roi) {
    if (frame.empty()) {
        std::cerr << "Error: Frame is empty.\n";
        return false;
    }

    cv::Rect validROI = roi & cv::Rect(0, 0, frame.cols, frame.rows);
    if (validROI.empty()) {
        std::cerr << "Error: ROI out of bounds.\n";
        return false;
    }

    cv::Mat cropped;
    if (!prepareCrop(frame, validROI, cropped)) {
        std::cerr << "Error: Failed to prepare cropped image.\n";
        return false;
    }

    bbox_ = validROI - offset_;
    tracker_ = createTracker(mode);

    tracker_->init(cropped.clone(), bbox_);
    return true;
}

bool Tracker_Algo::update(const cv::Mat& frame, cv::Rect& out_bbox) {
    if (frame.empty()) return false;

    cv::Mat cropped;
    if (!prepareCrop(frame, bbox_ + offset_, cropped)) {
        std::cerr << "Error: Invalid cropRegion in update.\n";
        return false;
    }

#if defined(__aarch64__) || defined(__arm__)
    cv::Rect2d bbox2d;
    bool ok = tracker_->update(cropped.clone(), bbox2d);
    bbox_ = bbox2d;
    //bool ok = tracker_->update(cropped.clone(), bbox_);
#elif defined(__x86_64__) || defined(_M_X64)
    bool ok = tracker_->update(cropped.clone(), bbox_);
#endif

    if (!ok) return false;

    out_bbox = bbox_ + offset_;
    return true;
}
