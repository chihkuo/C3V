#include "DBusServer.h"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <chrono>


DBusServer::DBusServer(GStreamerPipeline *gst_pipeline_rgb, GStreamerPipeline *gst_pipline_thermal) 
    : connection(nullptr)
    , distance_(0) 
 {
    gstPipeline_[SRC_RGB] = gst_pipeline_rgb;
    gstPipeline_[SRC_THERMAL] = gst_pipline_thermal;

    gstPipeline_[SRC_RGB]->setNotifyCallback([this](PipelineAlgoEvent& event) {
        handleCallback(SRC_RGB, event);
    });

    gstPipeline_[SRC_THERMAL]->setNotifyCallback([this](PipelineAlgoEvent& event) {
        handleCallback(SRC_THERMAL, event);
    });    
}

DBusServer::~DBusServer() {
    if (connection) {
        dbus_connection_unref(connection);
    }
}

void DBusServer::startServer() {
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Error connecting to D-Bus: " << error.message << std::endl;
        dbus_error_free(&error);
        return;
    }

    int ret = dbus_bus_request_name(connection, DBUS_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Error requesting name: " << error.message << std::endl;
        dbus_error_free(&error);
        return;
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        std::cerr << "Failed to obtain D-Bus name" << std::endl;
        return;
    }

    std::cout << "D-Bus server started. Listening for messages..." << std::endl;

    listenForMessages();
}

void DBusServer::listenForMessages() {
    while (true) {
        dbus_connection_read_write(connection, 100);
        DBusMessage *msg = dbus_connection_pop_message(connection);
        while (msg != nullptr) {
            handleMethodCall(msg);
            dbus_message_unref(msg);
            msg = dbus_connection_pop_message(connection);
        }
    }
}

void DBusServer::handleCallback(int index, PipelineAlgoEvent& event)
{
    algodataMgr_[index].setData(AlgoEventType::Algo_Results, event.data);
}

bool DBusServer::handleMethodCall(DBusMessage *msg) {

    const char *method = dbus_message_get_member(msg);
    DBusMessage *reply = dbus_message_new_method_return(msg);

    if (method == nullptr) {
        return false;
    }
    
    dbus_int32_t srcIndex = -1;
    int status = STATUS_FAILURE;
    if (strcmp(method, SWITCH_SINK) == 0) {
        dbus_int32_t sinkIndex;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &sinkIndex, DBUS_TYPE_INVALID);

        std::cout << "Received switch request, sinkIndex: " << sinkIndex << std::endl;

        if (isValidSrcIndex(srcIndex)) {
            gstPipeline_[srcIndex]->setVideoChannel(sinkIndex);
        }
    }
    else if (strcmp(method, GET_VIDEO_CHANNEL) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int channel = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoChannel(channel);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &channel, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_CHANNEL) == 0) {
        dbus_int32_t channel;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &channel, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setVideoChannel(channel);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_RESOLUTION) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int resolution = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoResolution(resolution);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &resolution, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_RESOLUTION) == 0) {
        dbus_int32_t resolution;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &resolution, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setVideoResolution(resolution);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_FPS) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int fps = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoFps(fps);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &fps, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_FPS) == 0) {
        dbus_int32_t fps;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &fps, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setVideoFps(fps);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_BITRATE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int bitrate = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoBitrate(bitrate);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &bitrate, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_BITRATE) == 0) {
        dbus_int32_t bitrate;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &bitrate, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setVideoBitrate(bitrate);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_ENCODE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int encode = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoEncode(encode);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &encode, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_ENCODE) == 0) {
        dbus_int32_t encode;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &encode, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setVideoEncode(encode);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_ZOOM) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int zoom = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoZoom(zoom);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &zoom, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_ZOOM) == 0) {
        dbus_int32_t zoom;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &zoom, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setVideoZoom(zoom);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_VIDEO_DISPLAY) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int display = 0;
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->getVideoDisplay(display);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &display, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_VIDEO_DISPLAY) == 0) {
        dbus_int32_t display;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &display, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setVideoDisplay(display);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_COLOR_PALETTE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int type = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getColorPalette(type);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &type, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_COLOR_PALETTE) == 0) {
        dbus_int32_t type;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &type, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setColorPalette(type);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_AI_TRIGGER) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int trigger = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getAiTrigger(trigger);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &trigger, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_AI_TRIGGER) == 0) {
        dbus_int32_t trigger;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &trigger, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setAiTrigger(trigger);
            if (trigger == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                algodataMgr_[srcIndex].reset();
            }
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_TRACKER_MODE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int mode = 0;
        if (isValidSrcIndex(srcIndex)) {
	        status = gstPipeline_[srcIndex]->getTrackerMode(mode);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &mode, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_TRACKER_MODE) == 0) {
        dbus_int32_t mode;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &mode, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setTrackerMode(mode);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }    
    else if (strcmp(method, GET_TRACKING_ID) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int id = 0;
        if (isValidSrcIndex(srcIndex)) {
	        status = gstPipeline_[srcIndex]->getTrackingId(id);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &id, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_TRACKING_ID) == 0) {
        dbus_int32_t id;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &id, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setTrackingId(id);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_CSRT_ROI) == 0) {
    	dbus_int32_t x, y, width, height;
    	dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex,	DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, 
            DBUS_TYPE_INT32, &width, DBUS_TYPE_INT32, &height, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setTrackingRoi(x, y, width, height);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_TRACKING_ROI) == 0) {
    	dbus_int32_t x, y, width, height;
    	dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex,	DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, 
            DBUS_TYPE_INT32, &width, DBUS_TYPE_INT32, &height, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->setTrackingRoi(x, y, width, height);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_DETECTION_DATA) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        std::string json_string;
        if (isValidSrcIndex(srcIndex)) {
            json_string = algodataMgr_[srcIndex].getJson(AlgoEventType::Algo_Results);
            status = STATUS_SUCCESS;
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &json_string, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_TRACK_1_DATA) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        std::string json_string;
        if (isValidSrcIndex(srcIndex)) {
            json_string = algodataMgr_[srcIndex].getJson(AlgoEventType::Algo_Results);
            status = STATUS_SUCCESS;
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &json_string, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_TRACK_2_DATA) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        std::string json_string;
        if (isValidSrcIndex(srcIndex)) {
            json_string = algodataMgr_[srcIndex].getJson(AlgoEventType::Algo_Results);
            status = STATUS_SUCCESS;
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &json_string, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_LASER_DISTANCE) == 0) {
        int distance = distance_;
        status = STATUS_SUCCESS;
        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &distance, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_LASER_DISTANCE) == 0) {
        dbus_int32_t distance;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &distance, DBUS_TYPE_INVALID);
        distance_ = distance;
        status = STATUS_SUCCESS;
        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, RECORD_START) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->recordStart();
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, RECORD_STOP) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->recordStop();
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SNAPSHOT) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);
        
        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->snapshot();
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_BRIGHTNESS) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getBrightness(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_BRIGHTNESS) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setBrightness(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_CONTRAST) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getContrast(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_CONTRAST) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setContrast(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_SATURATION) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getSaturation(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_SATURATION) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setSaturation(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_HUE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getHue(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_HUE) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setHue(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_SHARPNESS) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getSharpness(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_SHARPNESS) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setSharpness(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_DENOISE) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getDenoise(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_DENOISE) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setDenoise(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_TELE_ZOOM_SPEED) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getTeleZoomSpeed(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_TELE_ZOOM_SPEED) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setTeleZoomSpeed(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, GET_WIDE_ZOOM_SPEED) == 0) {
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INVALID);

        int value = 0;
        if (isValidSrcIndex(srcIndex)) {
    	    status = gstPipeline_[srcIndex]->getWideZoomSpeed(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);
    }
    else if (strcmp(method, SET_WIDE_ZOOM_SPEED) == 0) {
        dbus_int32_t value;
        dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &srcIndex, DBUS_TYPE_INT32, &value, DBUS_TYPE_INVALID);

        if (isValidSrcIndex(srcIndex)) {
            status = gstPipeline_[srcIndex]->setWideZoomSpeed(value);
        }

        dbus_message_append_args(reply, DBUS_TYPE_INT32, &status, DBUS_TYPE_INVALID);
    }

    dbus_connection_send(connection, reply, nullptr);
    dbus_message_unref(reply);

    return true;
}

bool DBusServer::isValidSrcIndex(int srcIndex)
{
    return srcIndex >= 0 && srcIndex < static_cast<int>(std::size(gstPipeline_));
}
