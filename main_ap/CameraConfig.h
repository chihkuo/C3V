#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>

#ifdef C3V_RELEASE
#define RGB_CONFIG  "/home/sunplus/demo/rgb_config.json"
#define THERMAL_CONFIG  "/home/sunplus/demo/thermal_config.json"
#else
#define RGB_CONFIG  "rgb_config.json"
#define THERMAL_CONFIG  "thermal_config.json"
#endif

using json = nlohmann::json;

struct CameraParams {
    std::string name;
    std::string device;
    int width;
    int height;
    int zoom_ratio;
    int fps;
    int bitrate;
    int encoder;
    int color_map;
    std::string record_path;
    std::string snapshot_path;
    int brightness;
    int contrast;
    int saturation;
    int hue;
    int sharpness;
    int denoise;
    int display_mode;
    int ai_mode;
    int tracker_mode;
    int tele_zoom_speed;
    int wide_zoom_speed;

    json toJson() const {
        return {
            {"name", name},
            {"device", device},
            {"width", width},
            {"height", height},
            {"zoom_ratio", zoom_ratio},
            {"fps", fps},
            {"bitrate", bitrate},
            {"encoder", encoder},
            {"color_map", color_map},
            {"record_path", record_path},
            {"snapshot_path", snapshot_path},
            {"brightness", brightness},
            {"contrast", contrast},
            {"saturation", saturation},
            {"hue", hue},
            {"sharpness", sharpness},
            {"denoise", denoise},
            {"display_mode", display_mode},
            {"ai_mode", ai_mode},
            {"tracker_mode", tracker_mode},
            {"tele_zoom_speed", tele_zoom_speed},
            {"wide_zoom_speed", wide_zoom_speed},
        };
    }

    static CameraParams fromJson(const json& j) {
        CameraParams params;
        j.at("name").get_to(params.name);
        j.at("device").get_to(params.device);
        j.at("width").get_to(params.width);
        j.at("height").get_to(params.height);
        j.at("zoom_ratio").get_to(params.zoom_ratio);
        j.at("fps").get_to(params.fps);
        j.at("bitrate").get_to(params.bitrate);
        j.at("encoder").get_to(params.encoder);
        j.at("color_map").get_to(params.color_map);
        j.at("record_path").get_to(params.record_path);
        j.at("snapshot_path").get_to(params.snapshot_path);
        j.at("brightness").get_to(params.brightness);
        j.at("contrast").get_to(params.contrast);
        j.at("saturation").get_to(params.saturation);
        j.at("hue").get_to(params.hue);
        j.at("sharpness").get_to(params.sharpness);
        j.at("denoise").get_to(params.denoise);
        j.at("display_mode").get_to(params.display_mode);
        j.at("ai_mode").get_to(params.ai_mode);
        j.at("tracker_mode").get_to(params.tracker_mode);
        j.at("tele_zoom_speed").get_to(params.tele_zoom_speed);
        j.at("wide_zoom_speed").get_to(params.wide_zoom_speed);
        return params;
    }
};

class CameraConfig {
public:
    CameraParams camera;

    void loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (file.is_open()) {
            _filePath = filePath;
            json j;
            file >> j;
            camera = CameraParams::fromJson(j);
        } else {
            std::cerr << "Failed to open " << filePath << " for reading.\n";
        }
    }

    void saveToFile(const std::string& filePath) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << camera.toJson().dump(4);
        } else {
            std::cerr << "Failed to open " << filePath << " for writing.\n";
        }
    }
    
    void save() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string tempFilePath = _filePath + ".tmp";
        std::ofstream file(tempFilePath);

        if (!file.is_open()) {
            std::cerr << "Failed to open " << tempFilePath << " for writing.\n";
            return;
        }

        file << camera.toJson().dump(4);
        file.close();

        if (std::rename(tempFilePath.c_str(), _filePath.c_str()) != 0) {
            std::cerr << "Failed to rename " << tempFilePath << " to " << _filePath << "\n";
        }
    }
    
    void dumpConfig() {
        std::cout << camera.toJson().dump(4) << std::endl;
    }
    
private:
    std::string _filePath;
    mutable std::mutex mutex_;
};
