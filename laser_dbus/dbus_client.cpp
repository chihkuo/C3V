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
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_CHANNEL);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &channel, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::getVideoResolution(int src, int &resolution) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_RESOLUTION);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &resolution, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::getVideoFps(int src, int &fps) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_FPS);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &fps, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::getVideoBitrate(int src, int &bitrate) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_BITRATE);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &bitrate, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::getVideoEncode(int src, int &encode) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_ENCODE);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &encode, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setVideoChannel(int src, int channel) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_CHANNEL);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &channel, DBUS_TYPE_INVALID);
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

int DBusClient::setVideoResolution(int src, int resolution) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_RESOLUTION);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &resolution, DBUS_TYPE_INVALID);
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

int DBusClient::setVideoFps(int src, int fps) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_FPS);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &fps, DBUS_TYPE_INVALID);
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

int DBusClient::setVideoBitrate(int src, int bitrate) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_BITRATE);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &bitrate, DBUS_TYPE_INVALID);
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

int DBusClient::setVideoEncode(int src, int encode) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_ENCODE);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &encode, DBUS_TYPE_INVALID);
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

int DBusClient::getVideoZoom(int src, int &ratio) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_ZOOM);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &ratio, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setVideoZoom(int src, int ratio) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_ZOOM);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &ratio, DBUS_TYPE_INVALID);
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

int DBusClient::getVideoDisplay(int src, int &mode) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_VIDEO_DISPLAY);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &mode, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setVideoDisplay(int src, int display) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_VIDEO_DISPLAY);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &display, DBUS_TYPE_INVALID);
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

//Thermal
int DBusClient::getColorPalette(int src, int &type) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_COLOR_PALETTE);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &type, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setColorPalette(int src, int type) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_COLOR_PALETTE);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &type, DBUS_TYPE_INVALID);
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

//AI
int DBusClient::getAiTrigger(int src, int &mode) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_AI_TRIGGER);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &mode, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setAiTrigger(int src, int mode) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_AI_TRIGGER);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &mode, DBUS_TYPE_INVALID);
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

//Tracking
int DBusClient::getTrackingId(int src, int &id) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, GET_TRACKING_ID);
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

    dbus_bool_t success = dbus_message_get_args(reply, nullptr, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &id, DBUS_TYPE_INVALID);
    if (!success) {
        std::cerr << "Failed to parse DBus reply arguments" << std::endl;
    }

    dbus_message_unref(reply);
    return status;
}

int DBusClient::setTrackingId(int src, int id) {
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_TRACKING_ID);
    if (!msg) {
        std::cerr << "Failed to create DBus message" << std::endl;
        return STATUS_FAILURE;
    }

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &src, DBUS_TYPE_INT32, &id, DBUS_TYPE_INVALID);
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
    int status = STATUS_FAILURE;
    DBusMessage *msg = dbus_message_new_method_call(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, SET_CSRT_ROI);
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
