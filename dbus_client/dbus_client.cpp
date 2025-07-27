#include "dbus_client.h"
#include "DBusDefinition.h"
#include <iostream>
#include <unistd.h>  // For usleep
#include <nlohmann/json.hpp>

DBusClient::DBusClient()
 : timeout_ms(-1) {
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, nullptr);
    dbus_connection_flush(conn);    
}

DBusClient::~DBusClient() {
    dbus_connection_unref(conn);
}

int DBusClient::getVideoChannel(int src, int &channel) {
    return getMethod(GET_VIDEO_CHANNEL, src, channel);
}

int DBusClient::getVideoResolution(int src, int &resolution) {
    return getMethod(GET_VIDEO_RESOLUTION, src, resolution);
}

int DBusClient::getVideoFps(int src, int &fps) {
    return getMethod(GET_VIDEO_FPS, src, fps);
}

int DBusClient::getVideoBitrate(int src, int &bitrate) {
    return getMethod(GET_VIDEO_BITRATE, src, bitrate);
}

int DBusClient::getVideoEncode(int src, int &encode) {
    return getMethod(GET_VIDEO_ENCODE, src, encode);
}

int DBusClient::setVideoChannel(int src, int channel) {
    return setMethod(SET_VIDEO_CHANNEL, src, channel);
}

int DBusClient::setVideoResolution(int src, int resolution) {
    return setMethod(SET_VIDEO_RESOLUTION, src, resolution);
}

int DBusClient::setVideoFps(int src, int fps) {
    return setMethod(SET_VIDEO_FPS, src, fps);
}

int DBusClient::setVideoBitrate(int src, int bitrate) {
    return setMethod(SET_VIDEO_BITRATE, src, bitrate);
}

int DBusClient::setVideoEncode(int src, int encode) {
    return setMethod(SET_VIDEO_ENCODE, src, encode);
}

int DBusClient::getVideoZoom(int src, int &ratio) {
    return getMethod(GET_VIDEO_ZOOM, src, ratio);
}

int DBusClient::setVideoZoom(int src, int ratio) {
    return setMethod(SET_VIDEO_ZOOM, src, ratio);
}

int DBusClient::getVideoDisplay(int src, int &mode) {
    return getMethod(GET_VIDEO_DISPLAY, src, mode);
}

int DBusClient::setVideoDisplay(int src, int display) {
    return setMethod(SET_VIDEO_DISPLAY, src, display);
}

int DBusClient::getBrightness(int src, int &value) {
    return getMethod(GET_BRIGHTNESS, src, value);
}

int DBusClient::getContrast(int src, int &value) {
    return getMethod(GET_CONTRAST, src, value);
}

int DBusClient::getSaturation(int src, int &value) {
    return getMethod(GET_SATURATION, src, value);
}

int DBusClient::getHue(int src, int &value) {
    return getMethod(GET_HUE, src, value);
}

int DBusClient::getSharpness(int src, int &value) {
    return getMethod(GET_SHARPNESS, src, value);
}

int DBusClient::getDenoise(int src, int &value) {
    return getMethod(GET_DENOISE, src, value);
}

int DBusClient::setBrightness(int src, int value) {
    return setMethod(SET_BRIGHTNESS, src, value);
}

int DBusClient::setContrast(int src, int value) {
    return setMethod(SET_CONTRAST, src, value);
}

int DBusClient::setSaturation(int src, int value) {
    return setMethod(SET_SATURATION, src, value);
}

int DBusClient::setHue(int src, int value) {
    return setMethod(SET_HUE, src, value);
}

int DBusClient::setSharpness(int src, int value) {
    return setMethod(SET_SHARPNESS, src, value);
}

int DBusClient::setDenoise(int src, int value) {
    return setMethod(SET_DENOISE, src, value);
}

//Thermal
int DBusClient::getColorPalette(int src, int &type) {
    return getMethod(GET_COLOR_PALETTE, src, type);
}

int DBusClient::setColorPalette(int src, int type) {
    return setMethod(SET_COLOR_PALETTE, src, type);
}

//AI
int DBusClient::getAiTrigger(int src, int &mode) {
    return getMethod(GET_AI_TRIGGER, src, mode);
}

int DBusClient::setAiTrigger(int src, int mode) {
    return setMethod(SET_AI_TRIGGER, src, mode);
}

//Tracking
int DBusClient::getTrackerMode(int src, int &id) {
    return getMethod(GET_TRACKER_MODE, src, id);
}

int DBusClient::setTrackerMode(int src, int id) {
    return setMethod(SET_TRACKER_MODE, src, id);
}

int DBusClient::getTrackingId(int src, int &id) {
    return getMethod(GET_TRACKING_ID, src, id);
}

int DBusClient::setTrackingId(int src, int id) {
    return setMethod(SET_TRACKING_ID, src, id);
}

int DBusClient::setTrackingRoi(int src, int x, int y, int width, int height) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_TRACKING_ROI);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INT32, &width, DBUS_TYPE_INT32, &height, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setCsrtRoi(int src, int x, int y, int width, int height) {
    return setTrackingRoi(src, x, y, width, height);
}

int DBusClient::getDetectionData(int src, std::string &data) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_DETECTION_DATA);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    const char *response;
    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &response, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    //nlohmann::json json_response = nlohmann::json::parse(response);
    //std::cout << json_response.dump(4) << std::endl;

    data = std::string(response);
    dbus_message_unref(reply);
    return status;
}

int DBusClient::getTrack1Data(int src, std::string &data) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_TRACK_1_DATA);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    const char *response;
    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &response, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    //nlohmann::json json_response = nlohmann::json::parse(response);
    //std::cout << json_response.dump(4) << std::endl;

    data = std::string(response);
    dbus_message_unref(reply);
    return status;
}

int DBusClient::getTrack2Data(int src, std::string &data) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_TRACK_2_DATA);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    const char *response;
    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &response, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }
    
    //nlohmann::json json_response = nlohmann::json::parse(response);
    //std::cout << json_response.dump(4) << std::endl;

    data = std::string(response);
    dbus_message_unref(reply);
    return status;
}

//laser
int DBusClient::getLaserDistance(int &distance) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_LASER_DISTANCE);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &distance, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setLaserDistance(int distance) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_LASER_DISTANCE);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &distance, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::recordStart(int src) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, RECORD_START);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::recordStop(int src) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, RECORD_STOP);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::snapshot(int src) {
    int status = STATUS_FAILURE;    
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SNAPSHOT);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::getMethod(const char *method, int src, int &value) {
    int status = STATUS_FAILURE; 
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, method);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setMethod(const char *method, int src, int value) {
    int status = STATUS_FAILURE; 
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, method);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }    

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, nullptr);
    dbus_message_unref(msg);
    if (!reply) {
        std::cerr << "Failed to get DBus reply" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}
