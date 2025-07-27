#include <iostream>
#include "opencv_utility.h"
#include "algo_manager.h"
#include "yolo_info.h"

using namespace YoloInfo;

extern "C"
{

#include "nn_api.h"
#include "cJSON.h"

    std::once_flag g_init_flag;
    static std::map<VNN_Mode, std::unique_ptr<AlgoManager>> g_algoManager;
    static std::map<VNN_Mode, std::vector<RESULT>> g_preResult = {
        { evm_YOLOv7_RGB, {} }, { evm_YOLOv7_THERMAL, {} }
    };

    static std::array<std::mutex, 2> g_modeMutex;

    void initialize_func() {
        std::call_once(g_init_flag, [] {
            std::cout << __func__ << std::endl;
            initializeColormap();
            YoloInfo::initColors();
        });
    }

    void draw_results_and_colormap(const AlgoParams *params, const char *detection_result_json) {
        if (params->display_mode == DisplayMode::DISPLAY_NONE && params->colormap_type < 0) {
            return;
        }

        int width = params->width;
        int height = params->height;
        const char *format = params->format;
        int algo_model = params->algo_model;
        int colormap_type = params->colormap_type;
        DisplayMode display_mode = params->display_mode;
        ProcessingMode process_mode = params->process_mode;

        cv::Mat frame;
        if (!convert_to_bgr(params->img, width, height, format, frame)) {
            return;
        }        

        cv::Mat image;
        if (colormap_type >= 0 && static_cast<size_t>(colormap_type) < custom_colormap.size()) {
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::Mat gray_3ch;
            cv::cvtColor(gray, gray_3ch, cv::COLOR_GRAY2BGR);
            cv::LUT(gray_3ch, custom_colormap[colormap_type], image);
        } else {
            image = frame;
        }

        cJSON* detection = cJSON_Parse(detection_result_json);
        if (detection) {
            cJSON* results = cJSON_GetObjectItem(detection, "results");
            if (results && cJSON_IsArray(results)) {
                cJSON* result = nullptr;
                cJSON_ArrayForEach(result, results) {
                    if (!cJSON_IsObject(result)) continue;

                    int track_id = cJSON_GetObjectItem(result, "track_id")->valueint;
                    int class_id = cJSON_GetObjectItem(result, "class_id")->valueint;
                    int x = cJSON_GetObjectItem(result, "x")->valueint;
                    int y = cJSON_GetObjectItem(result, "y")->valueint;
                    int w = cJSON_GetObjectItem(result, "width")->valueint;
                    int h = cJSON_GetObjectItem(result, "height")->valueint;
                    double confidence = cJSON_GetObjectItem(result, "confidence")->valuedouble;

                    //Bert -
                    //cv::Scalar boxColor = process_mode == ProcessingMode::PROCESSING_DETECT ? getColor(class_id) : getColor(0);
                    cv::Scalar boxColor =  (class_id <= static_cast<int>(getClassSize(algo_model))) ? getColor(class_id) : getColor(0);

                    if (display_mode & DisplayMode::DISPLAY_BOX) {
                        cv::rectangle(image, cv::Rect(x, y, w, h), boxColor, 2);
                    }

                    if (display_mode & DisplayMode::DISPLAY_TEXT) {
                        char str[32] = {0};
                        if (process_mode == ProcessingMode::PROCESSING_DETECT) {
                            sprintf(str, "%s %.2f", getClassName(algo_model, class_id).c_str(), confidence / 100);
                        } else if (process_mode == ProcessingMode::PROCESSING_TRACK_BOTSORT) {
                            if (class_id > static_cast<int>(getClassSize(algo_model)))
                                sprintf(str, "Tracking %d", track_id);
                            else
                                sprintf(str, "%d : %s", track_id, getClassName(algo_model, class_id).c_str());
                        } else if (process_mode == ProcessingMode::PROCESSING_TRACK_OPENCV) {
                            sprintf(str, "Tracking %d", track_id);
                        }

                        std::string label = str;
                        cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
                        cv::Rect textBox(x, y - 40, textSize.width + 10, textSize.height + 20);
                        cv::rectangle(image, textBox, boxColor, cv::FILLED);
                        cv::putText(image, label, cv::Point(x + 5, y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
                    }
                }
            }

            cJSON_Delete(detection);
        }

        cv::Mat output_frame;
        convert_from_bgr(image, output_frame, format);
        memcpy(params->img, output_frame.data, output_frame.total() * output_frame.elemSize());
    }

    int start_image_process(const AlgoParams *params, RESULT **result)
    {
        thread_local size_t no_result_count = 0;
        VNN_Mode algoMode = static_cast<VNN_Mode>(params->algo_model);

        std::lock_guard<std::mutex> lock(g_modeMutex[algoMode]);
        auto it = g_algoManager.find(algoMode);
        if (it == g_algoManager.end()) {
            auto algo = std::make_unique<AlgoManager>(algoMode);
            algo->start();
            g_algoManager[algoMode] = std::move(algo);
        } else {
            it->second->start();
        }

        g_algoManager[algoMode]->pushParams(*params);

        std::vector<RESULT> ret;
        g_algoManager[algoMode]->getResult(ret, std::chrono::milliseconds(5));

        if (ret.size()) {
            g_preResult[algoMode] = ret;
            no_result_count = 0;
        } else {
            ret = g_preResult[algoMode];
            if (++no_result_count > 5) {
                g_preResult[algoMode].clear();
                no_result_count = 0;
            }
        }

        std::size_t result_size = ret.size();
        if (result_size) {
            *result = (RESULT*)malloc(result_size * sizeof(RESULT));
            if (*result == nullptr) {
                std::cerr << "Memory allocation failed\n";
                return 0;
            }

            memcpy(*result, ret.data(), result_size * sizeof(RESULT));
        }

        return static_cast<int>(result_size);
    }

    int stop_image_process(int algo_model)
    {
        VNN_Mode mode = static_cast<VNN_Mode>(algo_model);
        std::lock_guard<std::mutex> lock(g_modeMutex[mode]);
        auto it = g_algoManager.find(mode);
        if (it != g_algoManager.end()) {
            it->second->stop();
        }

        g_preResult[mode].clear();
        return 0;
    }

    void release_results(RESULT **result) {
        if (result && *result) {
            free(*result);
            *result = nullptr;
        }
    }
}
