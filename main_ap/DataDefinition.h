#ifndef DATA_DEFINITION_H
#define DATA_DEFINITION_H

#include <vector>

enum class GstProcessingMode {
  GST_PROCESSING_DISABLE,
  GST_PROCESSING_DETECT,
  GST_PROCESSING_TRACK_BOTSORT,
  GST_PROCESSING_TRACK_OPENCV,
};

enum class AlgoEventType {
    Algo_Results,
    Algo_Detect,
    Algo_BotSort_Track,
    Algo_OpenCV_Track,
};

struct Rect {
    int x;
    int y;
    int width;
    int height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int x_, int y_, int width_, int height_)
        : x(x_), y(y_), width(width_), height(height_) {}
};

struct AlgoData {
    int track_id;
    int class_id;
    int confidence;
    Rect rect;

    AlgoData() : track_id(0), class_id(0), confidence(0), rect({0, 0, 0, 0}) {}
    AlgoData(int tid, int cid, int conf, const Rect& r)
        : track_id(tid), class_id(cid), confidence(conf), rect(r) {}    
};

struct PipelineAlgoEvent {
    AlgoEventType type;
    std::vector<AlgoData> data;

    PipelineAlgoEvent(AlgoEventType type_) : type(type_) {}
};

#endif
