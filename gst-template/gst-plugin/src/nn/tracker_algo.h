#ifndef __Tracker_Algo_H__
#define __Tracker_Algo_H__

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>
#include <chrono>

enum class TrackerMode {
    CSRT,
    KCF,
    MIL,
};

class Tracker_Algo {
public:
    Tracker_Algo(int margin = 20, bool useGray = false);

    bool init(TrackerMode mode, const cv::Mat& frame, const cv::Rect& roi);
    bool update(const cv::Mat& frame, cv::Rect& out_bbox);

private:
    int margin_;
    bool useGray_;
    cv::Ptr<cv::Tracker> tracker_;
    cv::Rect bbox_;
    cv::Point offset_;
    cv::Rect cropRegion_;

    static cv::Rect expandRect(const cv::Rect& rect, int margin);
    static cv::Rect sanitizeRect(const cv::Rect& rect, const cv::Size& imageSize);

    bool prepareCrop(const cv::Mat& frame, const cv::Rect& targetROI, cv::Mat& cropped);
    cv::Ptr<cv::Tracker> createTracker(TrackerMode mode);
};

#endif // __Tracker_Algo_H__
