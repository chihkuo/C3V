#include "AlgoDataMgr.h"
#include <iostream>

void to_json(nlohmann::json& j, const Rect& r) {
    j = nlohmann::json{{"x", r.x}, {"y", r.y}, {"width", r.width}, {"height", r.height}};
}

void to_json(nlohmann::json& j, const AlgoData& data) {
    j = nlohmann::json{
        {"track_id", data.track_id},
        {"class_id", data.class_id},
        {"confidence", data.confidence},
        {"rect", data.rect}
    };
}

void from_json(const nlohmann::json& j, Rect& r) {
    j.at("x").get_to(r.x);
    j.at("y").get_to(r.y);
    j.at("width").get_to(r.width);
    j.at("height").get_to(r.height);
}

void from_json(const nlohmann::json& j, AlgoData& data) {
    j.at("track_id").get_to(data.track_id);
    j.at("class_id").get_to(data.class_id);
    j.at("confidence").get_to(data.confidence);
    j.at("rect").get_to(data.rect);
}

AlgoDataMgr::AlgoDataMgr()
 : algoData_({
        {AlgoEventType::Algo_Results, {}},
        {AlgoEventType::Algo_Detect, {}},
        {AlgoEventType::Algo_BotSort_Track, {}},
        {AlgoEventType::Algo_OpenCV_Track, {}}}) {

}

AlgoDataMgr::~AlgoDataMgr() {

}

void AlgoDataMgr::setData(AlgoEventType type, std::vector<AlgoData> &data) {
    std::lock_guard<std::mutex> lock(mutex_);
    algoData_[type].swap(data);
}

std::vector<AlgoData> AlgoDataMgr::getData(AlgoEventType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    return algoData_[type];
}

std::string AlgoDataMgr::getJson(AlgoEventType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json json_data = algoData_[type];
    return json_data.dump(4);
}

void AlgoDataMgr::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : algoData_) {
        std::vector<AlgoData>().swap(pair.second);
    }
}
