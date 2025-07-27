#include "algo_manager.h"
#include "yolo_info.h"

#if defined(__aarch64__) || defined(__arm__)
#define C3V_PLATFORM 1
#elif defined(__x86_64__) || defined(_M_X64)
#define C3V_PLATFORM 0
#endif

using namespace YoloInfo;

AlgoManager::AlgoManager(VNN_Mode mode)
: mode_(mode)
, preprocessQueue_(1)
, inferenceQueue_(1)
, postprocessQueue_(1)
, is_running_(false)
#if C3V_PLATFORM
, vnn_mgr_(std::make_unique<VNN_Manager>(static_cast<VNN_Mode>(mode)))
#endif
{
    preprocessQueue_.setName("Preprocess");
    inferenceQueue_.setName("Inference");
    postprocessQueue_.setName("Postprocess");
    resultQueue_.setName("Result");
}

AlgoManager::~AlgoManager()
{
    stop();
}

void AlgoManager::start() {
    std::lock_guard<std::mutex> lock(running_mutex_);
    if (is_running_) return;

    is_running_ = true;

    preprocessQueue_.reset();
    inferenceQueue_.reset();
    postprocessQueue_.reset();
    resultQueue_.reset();

    preprocessWorker_.start([this]() {
        worker_loop(preprocessQueue_, &inferenceQueue_, is_running_,
            [this](TaskDataPtr task) { this->preprocess(task); });
    });

    inferenceWorker_.start([this]() {
        worker_loop(inferenceQueue_, &postprocessQueue_, is_running_,
            [this](TaskDataPtr task) { this->inference(task); });
    });

    postprocessWorker_.start([this]() {
        worker_loop(postprocessQueue_, &resultQueue_, is_running_,
            [this](TaskDataPtr task) { this->postprocess(task); });
    });    
}

void AlgoManager::stop()
{
    std::lock_guard<std::mutex> lock(running_mutex_);
    if (!is_running_) return;

    is_running_ = false;

    preprocessQueue_.shutdown();
    inferenceQueue_.shutdown();
    postprocessQueue_.shutdown();
    resultQueue_.shutdown();

    preprocessWorker_.stop();
    inferenceWorker_.stop();
    postprocessWorker_.stop();

    preprocessQueue_.clear();
    inferenceQueue_.clear();
    postprocessQueue_.clear();
    resultQueue_.clear();
}

void AlgoManager::pushParams(const AlgoParams& params)
{
    auto task = std::make_shared<TaskData>();
    task->id = getTimestampMs();
    task->params = std::make_shared<AlgoParams>(params);
    task->image.assign(params.img, params.img + params.data_size);
    preprocessQueue_.push(task);
}

int64_t AlgoManager::getTimestampMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void AlgoManager::preprocess(TaskDataPtr &task)
{
    //std::cout << __func__ << std::endl;
    unsigned char *img = task->image.data();
    int width = task->params->width;
    int height = task->params->height;
    const char *format = task->params->format;

    cv::Mat frame;
    if (!convert_to_bgr(img, width, height, format, frame)) {
        std::cout << "convert_to_bgr, failed." << std::endl;
        return;
    }

    if (frame.empty()) {
        std::cout << __func__ << ", frame.empty()" << std::endl;
        return;
    }

    #if C3V_PLATFORM
    ProcessingMode processing_mode = task->params->process_mode;
    if (processing_mode == PROCESSING_DETECT || processing_mode == PROCESSING_TRACK_BOTSORT) {
        vnn_mgr_->vnn_PreProcessNeuralNetwork(frame);
    }
    #endif

    task->image.clear();
    task->frame = frame;
}

void AlgoManager::inference(TaskDataPtr &task)
{
    ProcessingMode processing_mode = task->params->process_mode;
    if (processing_mode == PROCESSING_DETECT || 
        processing_mode == PROCESSING_TRACK_BOTSORT) {
        #if C3V_PLATFORM
        vnn_mgr_->vnn_ProcessGraph();
        #else
        fakeDetection(task);
        #endif
    } else if (processing_mode == PROCESSING_TRACK_OPENCV) {
        tracking(task);
    }
}

void AlgoManager::postprocess(TaskDataPtr &task)
{
    #if C3V_PLATFORM
    ProcessingMode processing_mode = task->params->process_mode;
    if (processing_mode == PROCESSING_DETECT) {
        std::vector<Output> ret;
        vnn_mgr_->vnn_GetResult(ret);

        std::vector<RESULT> result;
        result.reserve(ret.size());

        for (const auto& o : ret) {
            result.emplace_back(RESULT {
                .track_id = 0,
                .class_id = o.id,
                .confidence = static_cast<int>(o.confidence * 100),
                .box = { o.box.x, o.box.y, o.box.width, o.box.height }
            });
        }

        task->result.swap(result);

    } else if (processing_mode == PROCESSING_TRACK_BOTSORT) {
        std::vector<Output> ret;
        vnn_mgr_->vnn_GetResult(ret);

        thread_local std::unique_ptr<BoTSORT> tracker = std::make_unique<BoTSORT>("/home/sunplus/config/tracker.ini");

        std::vector<Detection> detections(ret.size());
        for (size_t i = 0; i < ret.size(); ++i) {
            detections[i].bbox_tlwh = cv::Rect_<float>(ret[i].box.x, ret[i].box.y, ret[i].box.width, ret[i].box.height);
            detections[i].class_id = ret[i].id;
            detections[i].confidence = 0.99;
        }

        std::vector<std::shared_ptr<Track>> tracks = tracker->track(detections, task->frame);

        int track_id = task->params->track_id;
        bool single_tracking = track_id > 0;

        std::vector<RESULT> result;
        result.reserve(tracks.size());
        for (const auto &track : tracks) {
            //if (single_tracking && track->track_id != track_id) { //Bert -- Disable
            //    continue;
            //}

            std::vector<float> bbox_tlwh = track->get_tlwh();
            result.emplace_back(RESULT {
                .track_id = track->track_id,
                .class_id = (single_tracking && (track->track_id == track_id)) ? 99 : track->classid,   //Bert -- Add "&& (track->track_id == track_id)"
                .confidence = -1,
                .box = {
                    static_cast<int>(bbox_tlwh[0]),
                    static_cast<int>(bbox_tlwh[1]),
                    static_cast<int>(bbox_tlwh[2]),
                    static_cast<int>(bbox_tlwh[3])
                 }
            });
        }

        task->result.swap(result);
    }

    #endif
}

void AlgoManager::fakeDetection(TaskDataPtr &task)
{
    std::vector<Output> ret;

    static std::random_device rd;
    static std::mt19937 gen(rd());

    int width = task->params->width;
    int height = task->params->height;

    std::uniform_int_distribution<int> confidenceDist(0, 100);
    std::uniform_int_distribution<int> boxXDist(0, width);
    std::uniform_int_distribution<int> boxYDist(0, height);
    std::uniform_int_distribution<int> boxWidthDist(1, width / 5);
    std::uniform_int_distribution<int> boxHeightDist(1, height / 5);

    for (int i = 0; i < 10; ++i) {
        Output output;
        output.id = i + 1;
        output.confidence = confidenceDist(gen);
        output.box = cv::Rect(boxXDist(gen), boxYDist(gen), boxWidthDist(gen), boxHeightDist(gen));

        ret.push_back(output);
    }

    std::vector<RESULT> result;
    for (const auto& o : ret) {
        RESULT r;
        r.track_id = 0;
        r.class_id = o.id;
        r.confidence = static_cast<int>(o.confidence * 100);
        r.box = { o.box.x, o.box.y, o.box.width, o.box.height };
        result.push_back(r);
    }

    task->result.swap(result);
}

void AlgoManager::tracking(TaskDataPtr &task)
{
    thread_local std::unique_ptr<Tracker_Algo> tracker;
    if (task->frame.empty()) {
        std::cout << "frame is empty()....." << std::endl;
        return;
    }

    cv::Mat frame = task->frame;
    BOX *box = task->params->box;
    cv::Rect rect(box->x, box->y, box->width, box->height);
    if (rect.area() > 0) {
        memset(box, 0, sizeof(BOX));
        tracker = std::make_unique<Tracker_Algo>(30, false);
        TrackerMode mode = static_cast<TrackerMode>(task->params->tracker_mode);
        if (!tracker->init(mode, frame, rect)) {
            std::cout << "Tracker initialization failed." << std::endl;
            tracker.reset();
            return;
        }
    }

    if (!tracker) {
        //std::cerr << "Tracker is not initialized." << std::endl;
        return;
    }
    
    cv::Rect bbox;
    bool ok = tracker->update(frame, bbox);
    if (!ok) {
        tracker.reset();
        std::cout << "tracking failed, -----> reset()" << std::endl;
        return;
    }

    task->result = {
        RESULT {
            .track_id = 1,
            .class_id = 99,
            .confidence = 0,
            .box = { bbox.x, bbox.y, bbox.width, bbox.height }
        }
    };
}

bool AlgoManager::getResult(std::vector<RESULT> &result, std::chrono::milliseconds timeout) {
    TaskDataPtr task;
    //if (resultQueue_.wait_and_pop(task, timeout)) {
    if (resultQueue_.try_pop(task)) {
        task->result.swap(result);
        return true;
    }
    
    return false;
}
