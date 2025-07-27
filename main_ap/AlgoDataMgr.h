#ifndef ALGO_DATA_MGR_H
#define ALGO_DATA_MGR_H

#include "DataDefinition.h"
#include <nlohmann/json.hpp>
#include <mutex>
#include <map>


void to_json(nlohmann::json& j, const Rect& r);
void to_json(nlohmann::json& j, const AlgoData& data);
void from_json(const nlohmann::json& j, Rect& r);
void from_json(const nlohmann::json& j, AlgoData& data);

class AlgoDataMgr {
public:
    AlgoDataMgr();
    ~AlgoDataMgr();

    void setData(AlgoEventType type, std::vector<AlgoData> &data);
    std::vector<AlgoData> getData(AlgoEventType type);
    std::string getJson(AlgoEventType type);
    void reset();

private:
    std::mutex mutex_;
    std::map<AlgoEventType, std::vector<AlgoData>> algoData_;
};


#endif  // ALGO_DATA_MGR_H
