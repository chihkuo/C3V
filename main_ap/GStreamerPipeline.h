#ifndef GSTREAMER_PIPELINE_H
#define GSTREAMER_PIPELINE_H

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>

#include "DBusDefinition.h"
#include "DataDefinition.h"
#include "CameraConfig.h"
#include "CameraFunctionMgr.h"

using AlgoNotifyCallback = std::function<void(PipelineAlgoEvent&)>;

enum CameraSource {
    RGB = 0,
    THERMAL
};

class GStreamerPipeline {
public:
    GStreamerPipeline(CameraSource source = CameraSource::RGB);
    ~GStreamerPipeline();

    void attachToServer(GstRTSPServer *server, GstRTSPMountPoints *mounts, const std::string &path);

    //Record
    int recordStart();
    int recordStop();

    //Snapshot
    int snapshot();

    //Video
    int getVideoChannel(int &channel);
    int getVideoResolution(int &resolution);
    int getVideoFps(int &fps);
    int getVideoBitrate(int &bitrate);
    int getVideoEncode(int &encode);
    int getVideoZoom(int &zoomLevel);
    int getVideoDisplay(int &mode);

    int setVideoChannel(int channel);
    int setVideoResolution(int resolution);
    int setVideoResolution(int width, int height);
    int setVideoFps(int fps);
    int setVideoBitrate(int bitrate);
    int setVideoEncode(int encode);
    int setVideoZoom(int zoomLevel);
    int setVideoDisplay(int mode);

    int getBrightness(int &valaue);
    int getContrast(int &value);
    int getSaturation(int &value);
    int getHue(int &value);
    int getSharpness(int &value);
    int getDenoise(int &value);

    int setBrightness(int valaue);
    int setContrast(int value);
    int setSaturation(int value);
    int setHue(int value);
    int setSharpness(int value);
    int setDenoise(int value);

    int getTeleZoomSpeed(int &value);
    int getWideZoomSpeed(int &value);
    int setTeleZoomSpeed(int value);
    int setWideZoomSpeed(int value);

    //Thermal
    int getColorPalette(int &type);
    int setColorPalette(int type);

    //AI
    int getAiTrigger(int &trigger);
    int setAiTrigger(int trigger);

    //Tracking
    int getTrackerMode(int &mode);
    int setTrackerMode(int mode);
    int getTrackingId(int &id);
    int setTrackingId(int id);

    int setTrackingRoi(int x, int y, int width, int height);
    void send_tracking_roi_event(GstElement* element, gint roi_x, gint roi_y, gint roi_width, gint roi_height);    

    void setNotifyCallback(AlgoNotifyCallback callback);

    static void client_connected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data);


private:
    static gboolean mediaConfigureCallback(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
    static void media_unprepared_callback(GstRTSPMedia *media, gpointer user_data);
    static GstPadProbeReturn event_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data); 
    static gboolean message_cb(GstBus *bus, GstMessage *message, gpointer user_data);
    static void client_disconnected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data);
    static void process_algo_event(GStreamerPipeline *pipeline_instance, AlgoEventType event_type,  const GstStructure *structure);


    static gboolean on_need_data_handler(GstElement *appsrc, guint unused_size, gpointer user_data);
    int capture_snapshot(GstElement* sink);
    char* generate_filename(const char *prefix, const char *extension);

    bool isPlaying();

    GstElement *pipeline, *rec_pipeline;
    GstElement *input_selector;
    GstElement *algoprocess;
    GstElement *rec_appsink, *rec_appsrc, *rec_filesink;
    GstElement *snap_appsink;
    GstElement *snap_pipeline;
    GstElement *videorate;
    GstElement *encoder;
    GstElement *capsfilter;
    GstElement *videobalance;

    std::atomic<bool> is_recording;

    int videoChannel;
    int videoEncode;
    int videoZoom;
    int trackingId;

    AlgoNotifyCallback algoNotifyCallback_;
    CameraSource cameraSource_;
    CameraConfig cameraConfig_;
    std::unique_ptr<CameraFunctionMgr> cameraFunctionMgr_;
    std::mutex mutex_;
};

#endif // GSTREAMER_PIPELINE_H

