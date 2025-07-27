#ifndef __ALGO_MANAGER_H__
#define __ALGO_MANAGER_H__

#include "SafeQueue.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include "definition.h"
#include "vnn_manager.h"

#include "BoTSORT.h"
#include "DataType.h"
#include "track.h"
#include "tracker_algo.h"
#include "opencv_utility.h"
#include "WorkerThread.h"


using namespace cv;
using namespace std;

class AlgoManager {
public:
    AlgoManager(VNN_Mode mode);
    ~AlgoManager();

    void start();
    void stop();
    void pushParams(const AlgoParams& params);
    bool getResult(std::vector<RESULT>& result, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

private:
    struct TaskData {
        int64_t id;
        std::shared_ptr<AlgoParams> params;
        cv::Mat frame;
        std::vector<RESULT> result;
        std::vector<unsigned char> image;

        TaskData() = default;
        TaskData(int64_t _id, std::shared_ptr<AlgoParams> _params, const cv::Mat& _frame)
            : id(_id), params(_params), frame(_frame) {}
    };

    VNN_Mode mode_;

    using TaskDataPtr = std::shared_ptr<TaskData>;
    SafeQueue<TaskDataPtr> preprocessQueue_;
    SafeQueue<TaskDataPtr> inferenceQueue_;
    SafeQueue<TaskDataPtr> postprocessQueue_;
    SafeQueue<TaskDataPtr> resultQueue_;

    WorkerThread preprocessWorker_;
    WorkerThread inferenceWorker_;
    WorkerThread postprocessWorker_;

    std::unique_ptr<VNN_Manager> vnn_mgr_;
    std::atomic<bool> is_running_;
    std::mutex running_mutex_;

    int64_t getTimestampMs();
    void preprocess(TaskDataPtr& task);
    void inference(TaskDataPtr& task);
    void postprocess(TaskDataPtr& task);
    void fakeDetection(TaskDataPtr& task);
    void tracking(TaskDataPtr& task);
};

#endif
