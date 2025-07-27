#include "GStreamerPipeline.h"
#include <iostream>

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <future>

#if defined(__aarch64__) || defined(__arm__)
#define C3V_PLATFORM 1
#define V4L2H264ENC
#elif defined(__x86_64__) || defined(_M_X64)
#define C3V_PLATFORM 0
#endif

#ifdef C3V_RELEASE
#define CHECK_SDCARD_MOUNTED
#endif

constexpr std::array<const char *, 4> resolutions = {"1920x1080", "1280x720", "640x480", "320x240"};

GStreamerPipeline::GStreamerPipeline(CameraSource source)
    : pipeline(nullptr)
    , rec_pipeline(nullptr)
    , algoprocess(nullptr)
    , rec_appsink(nullptr)
    , rec_appsrc(nullptr)
    , rec_filesink(nullptr)
    , snap_appsink(nullptr)
    , snap_pipeline(nullptr)
    , videorate(nullptr)
    , encoder(nullptr)
    , capsfilter(nullptr)
    , videobalance(nullptr)
    , is_recording(false)
    , videoEncode(0)
    , videoZoom(10)
    , algoNotifyCallback_(nullptr)
    , cameraSource_(source) {
    const char *path = cameraSource_ == CameraSource::RGB ? RGB_CONFIG : THERMAL_CONFIG;
    std::cout << "Config Path: " << path << std::endl;
    cameraConfig_.loadFromFile(path);
    cameraConfig_.dumpConfig();
}

GStreamerPipeline::~GStreamerPipeline() {
    if (rec_pipeline) {
        gst_element_set_state(rec_pipeline, GST_STATE_NULL);
        gst_object_unref(rec_pipeline);        
    }

    if (snap_pipeline) {
        gst_element_set_state(snap_pipeline, GST_STATE_NULL);
        gst_object_unref(snap_pipeline);
    }

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
}

void GStreamerPipeline::attachToServer(GstRTSPServer *server, GstRTSPMountPoints *mounts, const std::string &path) {

    gchar *pipeline_desc;
    if (cameraSource_ == CameraSource::RGB) {
 #if C3V_PLATFORM
        //record + snapshot
        const gchar *pipeline_cmd = \
            "v4l2src device=/dev/video36 name=camera0 ! videoconvert name=convert0 ! video/x-raw,width=1920,height=1080 ! videorate name=videorate drop-only=true "
            "! videocrop name=videocrop ! videoscale method=nearest-neighbour ! capsfilter name=capsfilter ! videobalance name=videobalance "
            "! tee name=t "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! algoprocess name=algoprocess silent=false ! v4l2h264enc name=encoder ! h264parse ! seiparse ! rtph264pay name=pay0 pt=96 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=rec_sink emit-signals=false sync=false drop=true max-buffers=1 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=snap_sink drop=true max-buffers=1";

        /*//with EIS
        //const gchar *pipeline_cmd = \
        //    "v4l2srceis device=/dev/video36 name=camera0 ! video/x-raw,width=1920,height=1080,format=UYVY ! gyroeis file0=/home/sunplus/EIS/ release/arm64-v8a/resource/env/gstreamer_plugin.ini "
        //    "! videorate name=videorate drop-only=true ! videocrop name=videocrop ! videoscale method=nearest-neighbour ! capsfilter name=capsfilter ! videobalance name=videobalance "
        //    "! tee name=t "
        //    "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! algoprocess name=algoprocess silent=false ! videoconvert ! v4l2h264enc name=encoder ! h264parse ! seiparse ! rtph264pay name=pay0 pt=96 "
        //    "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=rec_sink emit-signals=false sync=false drop=true max-buffers=1 "
        //    "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=snap_sink drop=true max-buffers=1";*/
#else
        const gchar *pipeline_cmd = \
            "v4l2src device=/dev/video0 name=camera0 ! videoconvert name=convert0 ! video/x-raw,width=640,height=480,framerate=30/1 ! videorate name=videorate drop-only=true "
            "! videocrop name=videocrop ! videoscale method=nearest-neighbour ! capsfilter name=capsfilter ! videoconvert ! video/x-raw,format=NV12 ! videobalance name=videobalance ! timeoverlay "
            "! tee name=t "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! algoprocess name=algoprocess silent=false ! videoconvert ! video/x-raw,format=I420 ! x264enc name=encoder tune=zerolatency bitrate=500 speed-preset=ultrafast ! h264parse ! seiparse ! rtph264pay name=pay0 pt=96 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=rec_sink emit-signals=false sync=false drop=true max-buffers=1 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=snap_sink drop=true max-buffers=1";
#endif
        // Use g_strdup to duplicate the pipeline string
        pipeline_desc = g_strdup(pipeline_cmd);
    } else {
#if C3V_PLATFORM
        //record + snapshot
        const gchar *pipeline_cmd = \
            "v4l2src device=/dev/video0 name=camera0 ! videoconvert name=convert0 ! videorate name=videorate drop-only=true "
            "! videocrop name=videocrop ! videoscale method=nearest-neighbour ! capsfilter name=capsfilter ! videoconvert ! video/x-raw,format=BGR ! videobalance name=videobalance "
            "! tee name=t "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! algoprocess name=algoprocess silent=false ! videoconvert ! video/x-raw,format=UYVY ! v4l2h264enc name=encoder ! h264parse ! seiparse ! rtph264pay name=pay0 pt=96 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=rec_sink emit-signals=false sync=false drop=true max-buffers=1 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=snap_sink drop=true max-buffers=1";

#else
        const gchar *pipeline_cmd = \
            "videotestsrc name=camera0 ! videoconvert name=convert0 ! video/x-raw,width=640,height=480,framerate=30/1 ! videorate name=videorate drop-only=true "
            "! videocrop name=videocrop ! videoscale method=nearest-neighbour ! capsfilter name=capsfilter ! video/x-raw,format=BGR ! videobalance name=videobalance ! timeoverlay "
            "! tee name=t "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 ! algoprocess name=algoprocess silent=false ! videoconvert ! video/x-raw,format=I420 ! x264enc name=encoder tune=zerolatency bitrate=500 speed-preset=ultrafast ! h264parse ! seiparse ! rtph264pay name=pay0 pt=96 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=rec_sink emit-signals=false sync=false drop=true max-buffers=1 "
            "t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=snap_sink drop=true max-buffers=1";
#endif     

        // Use g_strdup to duplicate the pipeline string
        pipeline_desc = g_strdup(pipeline_cmd);       
    }

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, pipeline_desc);
    gst_rtsp_media_factory_set_shared(factory, TRUE);

    g_signal_connect(factory, "media-configure", G_CALLBACK(mediaConfigureCallback), this);
    gst_rtsp_mount_points_add_factory(mounts, path.c_str(), factory);
    g_free(pipeline_desc);
}

int GStreamerPipeline::recordStart()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isPlaying())
    {
        std::cout << ">> streaming is not ready..." << std::endl;
        return STATUS_FAILURE;
    }

    std::cout << ">> recordStart" << std::endl;
    if (!is_recording) {
        g_print("Start Recording...\n");

        gchar *filename = generate_filename("rec", "mp4");
        if (!filename) {
            return STATUS_FAILURE;       
        }

        GstElement *algo = gst_bin_get_by_name(GST_BIN(rec_pipeline), "algoprocess");
        if (algo) {
            int type;
            getColorPalette(type);
            g_object_set(G_OBJECT(algo), "colormap-type", type, NULL);
            gst_object_unref(algo);
        }

        g_object_set(rec_filesink, "location", filename, NULL);
        g_free(filename);

        GstStateChangeReturn ret = gst_element_set_state(rec_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_ASYNC) {
            //g_print("Failed to set rec_pipeline to PLAYING state. (GST_STATE_CHANGE_ASYNC)\n");
            is_recording = true;
        }
        else if (ret == GST_STATE_CHANGE_SUCCESS) {
            //g_print("Successed to set rec_pipeline to PLAYING state. (GST_STATE_CHANGE_SUCCESS)\n");
            is_recording = true;
        }
        else if (ret == GST_STATE_CHANGE_FAILURE) {
            g_print("Failed to set rec_pipeline to PLAYING state. (GST_STATE_CHANGE_FAILURE)\n");
            is_recording = false;
        }
        else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
            g_print("Failed to set rec_pipeline to PLAYING state. (GST_STATE_CHANGE_NO_PREROLL)\n");
            is_recording = false;
        }

        return is_recording ? STATUS_SUCCESS : STATUS_FAILURE;
    } else {
        g_print("Already recording.\n");
        return STATUS_FAILURE;
    }    
}

int GStreamerPipeline::recordStop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isPlaying())
    {
        std::cout << ">> streaming is not ready..." << std::endl;
        return STATUS_FAILURE;
    }

    std::cout << ">> recordStop" << std::endl;
    if (is_recording) {
        g_print("Stop Recording...\n");
        gst_app_src_end_of_stream(GST_APP_SRC(rec_appsrc));
        return STATUS_SUCCESS;
    } else {
        g_print("Not currently recording.\n");
        return STATUS_FAILURE;
    }
}

int GStreamerPipeline::snapshot()
{
    if (!isPlaying() || !snap_appsink)
    {
        std::cout << ">> streaming is not ready..." << std::endl;
        return STATUS_FAILURE;
    }

    std::cout << ">> Snapshot" << std::endl;
    return capture_snapshot(snap_appsink);
}

char *GStreamerPipeline::generate_filename(const char *prefix, const char *extension)
{
    if (!prefix || !extension) {
        return nullptr;
    }

    const gchar *src_prefix = cameraSource_ == CameraSource::RGB ? "eo" : "ir";
    gint64 now_us = g_get_real_time();
    time_t now_sec = now_us / 1000000;
    gint millis = (now_us % 1000000) / 1000;
    
    struct tm *tm_now = localtime(&now_sec);
    
    char buf[64];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", tm_now);
    
    char full_filename[80];
    snprintf(full_filename, sizeof(full_filename), "%s_%s_%03d.%s", src_prefix, buf, millis, extension);

#if C3V_PLATFORM

    const char *path = cameraConfig_.camera.record_path.c_str();

#ifdef CHECK_SDCARD_MOUNTED
    const std::string mountPoint = std::string(path);
    std::ifstream mounts("/proc/mounts");
    std::string line;
    bool isMounted = false;
    while (std::getline(mounts, line)) {
        if (line.find(mountPoint) != std::string::npos) {
            isMounted = true;
            break;
        }
    }

    if (!isMounted) {
        std::cout << path << ", not mounted." << std::endl;
        return NULL;
    }
#endif 

    gchar *full_path = g_strdup_printf("%s/%s", path, full_filename);
#else
    gchar *full_path = g_strdup_printf("/home/jerry/sdcard/%s", full_filename);
#endif

    static int num = 0;
    g_print("( %03d ) File Name: %s\n", ++num, full_filename);

    return full_path;
}

bool GStreamerPipeline::isPlaying()
{
    GstState current_state, pending_state;
    GstStateChangeReturn ret = gst_element_get_state(
        pipeline, &current_state, &pending_state, GST_CLOCK_TIME_NONE);

    if (ret == GST_STATE_CHANGE_SUCCESS && current_state == GST_STATE_PLAYING) {
        //std::cout << "Pipeline is running (PLAYING)." << std::endl;
        return true;
    } else {
        std::cout << "Pipeline is not running." << std::endl;
        std::cout << "Current state: " << gst_element_state_get_name(current_state) << std::endl;
        return false;
    }
}

gboolean GStreamerPipeline::message_cb(GstBus *bus, GstMessage *message, gpointer user_data) {

    GstObject *src = GST_MESSAGE_SRC(message);

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err = NULL;
            gchar *debug = NULL;

            gst_message_parse_error(message, &err, &debug);
            g_printerr("ERROR: %s\n", err->message);
            if (debug) g_printerr("Debug info: %s\n", debug);

            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_EOS: {
            GStreamerPipeline *pipeline_instance = static_cast<GStreamerPipeline*>(user_data);
            if (GST_OBJECT(src) == GST_OBJECT(pipeline_instance->rec_pipeline)) {
                GstElement *pipeline = pipeline_instance->rec_pipeline;
                gst_element_set_state(pipeline, GST_STATE_NULL);
                sync();
                pipeline_instance->is_recording = false;
                g_print("EOS signal from rec_pipeline.\n");
            } else if (GST_OBJECT(src) == GST_OBJECT(pipeline_instance->snap_pipeline)) {
                gst_element_set_state(pipeline_instance->snap_pipeline, GST_STATE_NULL);
                sync();
                g_print("EOS signal from snap_pipeline\n");
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int GStreamerPipeline::capture_snapshot(GstElement* sink) {

    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    GstElement *snap_appsrc = gst_bin_get_by_name(GST_BIN(snap_pipeline), "src");
    if (snap_appsrc == nullptr) {
        gst_sample_unref(sample);
        std::cerr << "Error: snap_appsrc element not found in pipeline" << std::endl;
        return STATUS_FAILURE;
    }

    GstElement *algo = gst_bin_get_by_name(GST_BIN(snap_pipeline), "algoprocess");
    if (algo) {
        int type;
        getColorPalette(type);
        g_object_set(G_OBJECT(algo), "colormap-type", type, NULL);
        gst_object_unref(algo);
    }

    GstElement *snap_filesink = gst_bin_get_by_name(GST_BIN(snap_pipeline), "fsink");
    if (snap_filesink == nullptr) {
        gst_sample_unref(sample);
        gst_object_unref(snap_appsrc);
        std::cerr << "Error: snap_filesink element not found in pipeline" << std::endl;
        return STATUS_FAILURE;
    }

    gchar *filename = generate_filename("snap", "jpg");
    
    if (!filename) {
        gst_object_unref(snap_appsrc);
        gst_sample_unref(sample);
        return STATUS_FAILURE;       
    }

    g_object_set(snap_filesink, "location", filename, NULL);
    gst_object_unref(snap_filesink);
    g_free(filename);

    GstStateChangeReturn ret_state = gst_element_set_state(snap_pipeline, GST_STATE_PLAYING);
    if (ret_state == GST_STATE_CHANGE_FAILURE) {
        g_print("Failed to set snap_pipeline to PLAYING state. (GST_STATE_CHANGE_FAILURE)\n");
    }
    else if (ret_state == GST_STATE_CHANGE_NO_PREROLL) {
        g_print("Failed to set snap_pipeline to PLAYING state. (GST_STATE_CHANGE_NO_PREROLL)\n");
    }

    GstFlowReturn ret = gst_app_src_push_sample(GST_APP_SRC(snap_appsrc), sample);

    gst_object_unref(snap_appsrc);
    gst_sample_unref(sample);
    return ret == GST_FLOW_OK ? STATUS_SUCCESS : STATUS_FAILURE;
}

gboolean GStreamerPipeline::on_need_data_handler(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    GStreamerPipeline *pipeline_instance = static_cast<GStreamerPipeline*>(user_data);
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(pipeline_instance->rec_appsink));
    if (!sample) {
        g_print("need_data_handler, No more data from appsink.\n");
        return FALSE;
    }

    GstFlowReturn ret = gst_app_src_push_sample(GST_APP_SRC(pipeline_instance->rec_appsrc), sample);
    if (ret != GST_FLOW_OK) {
        g_printerr("Failed to push sample to appsrc\n");
    }

    gst_sample_unref(sample);
    return TRUE; 
}

gboolean GStreamerPipeline::mediaConfigureCallback(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    GStreamerPipeline *pipeline_instance = static_cast<GStreamerPipeline*>(user_data);

    g_print("media ( %p )\n", media);
    g_signal_connect(media, "unprepared", G_CALLBACK(media_unprepared_callback), user_data);

    // get pipeline
    GstElement *pipeline = gst_rtsp_media_get_element(media);
    pipeline_instance->pipeline = pipeline;

    GstElement *algoprocess = gst_bin_get_by_name(GST_BIN(pipeline), "algoprocess");
    if (!algoprocess)
        g_printerr("! ERROR: algoprocess element not found in pipeline\n");
    pipeline_instance->algoprocess = algoprocess;
    gst_object_unref(algoprocess);

    GstElement *videorate = gst_bin_get_by_name(GST_BIN(pipeline), "videorate");
    if (!videorate)
        g_printerr("! ERROR: videorate element not found in pipeline\n");
    pipeline_instance->videorate = videorate;
    gst_object_unref(videorate);

    GstElement *encoder = gst_bin_get_by_name(GST_BIN(pipeline), "encoder");
    if (!encoder)
        g_printerr("! ERROR: encoder element not found in pipeline\n");
    pipeline_instance->encoder = encoder;
    gst_object_unref(encoder);

    GstElement *capsfilter = gst_bin_get_by_name(GST_BIN(pipeline), "capsfilter");
    if (!capsfilter)
        g_printerr("! ERROR: capsfilter element not found in pipeline\n");
    pipeline_instance->capsfilter = capsfilter;
    gst_object_unref(capsfilter);

    GstElement *videobalance = gst_bin_get_by_name(GST_BIN(pipeline), "videobalance");
    if (!videobalance)
        g_printerr("! ERROR: videobalance element not found in pipeline\n");
    pipeline_instance->videobalance = videobalance;
    gst_object_unref(videobalance);
    
    //set property
    if (pipeline_instance->cameraSource_ == CameraSource::THERMAL) {
        g_object_set(G_OBJECT(algoprocess), "algo-model", 1, NULL);
    }

    // register event
    GstPad *algo_sink_pad = gst_element_get_static_pad(algoprocess, "sink");
    if (!algo_sink_pad)
        g_printerr("! ERROR: algo_sink_pad element not found in pipeline\n");
    gst_pad_add_probe(algo_sink_pad, GST_PAD_PROBE_TYPE_EVENT_UPSTREAM, event_probe_callback, user_data, NULL);
    gst_object_unref(algo_sink_pad);

    GstElement *rec_appsink = gst_bin_get_by_name(GST_BIN(pipeline), "rec_sink");
    if (!rec_appsink)
        g_printerr("! ERROR: rec_appsink element not found in pipeline\n");
    pipeline_instance->rec_appsink = rec_appsink;
    gst_object_unref(rec_appsink);


#if C3V_PLATFORM
    GstElement *rec_pipeline = gst_parse_launch(
        "appsrc name=src ! algoprocess name=algoprocess execution-mode=1 ! videoconvert ! video/x-raw,format=UYVY ! v4l2h264enc name=encoder ! h264parse "
        "! mp4mux ! queue ! filesink name=fsink", 
        NULL);
#else
    GstElement *rec_pipeline = gst_parse_launch(
        "appsrc name=src is-live=true format=time ! algoprocess name=algoprocess execution-mode=1 ! videoconvert ! x264enc name=encoder tune=zerolatency bitrate=500 speed-preset=ultrafast ! h264parse "
        "! mp4mux ! filesink name=fsink", 
        NULL);
#endif

    pipeline_instance->rec_pipeline = rec_pipeline;

    GstElement *appsrc = gst_bin_get_by_name(GST_BIN(rec_pipeline), "src");
    if (!appsrc)
        g_printerr("! ERROR: appsrc element not found in pipeline\n");
    pipeline_instance->rec_appsrc = appsrc;
    g_signal_connect(appsrc, "need-data", G_CALLBACK(on_need_data_handler), user_data);
    gst_object_unref(appsrc);

    GstElement *filesink = gst_bin_get_by_name(GST_BIN(rec_pipeline), "fsink");
    if (!filesink)
        g_printerr("! ERROR: filesink element not found in pipeline\n");
    pipeline_instance->rec_filesink = filesink;
    gst_object_unref(filesink);

    GstBus *bus = gst_element_get_bus(rec_pipeline);
    if (!bus)
        g_printerr("! ERROR: bus element not found in pipeline\n");
    gst_bus_add_watch(bus, message_cb, user_data);
    gst_object_unref(bus);

#if C3V_PLATFORM
    GstElement *snap_pipeline = gst_parse_launch(
        "appsrc name=src num-buffers=1 ! algoprocess name=algoprocess execution-mode=1 ! videoconvert ! jpegenc ! filesink name=fsink o-sync=true",
        NULL);
#else
    GstElement *snap_pipeline = gst_parse_launch(
        "appsrc name=src num-buffers=1 ! algoprocess name=algoprocess execution-mode=1 ! videoconvert ! jpegenc ! filesink name=fsink",
        NULL); 
#endif  

    pipeline_instance->snap_pipeline = snap_pipeline;

    GstElement *snap_appsink = gst_bin_get_by_name(GST_BIN(pipeline), "snap_sink");
    if (!snap_appsink)
        g_printerr("! ERROR: snap_appsink element not found in pipeline\n");
    pipeline_instance->snap_appsink = snap_appsink;
    gst_object_unref(snap_appsink);

    GstBus *bus_snap = gst_element_get_bus(snap_pipeline);
    if (!bus_snap)
        g_printerr("! ERROR: bus_snap element not found in pipeline\n");
    gst_bus_add_watch(bus_snap, message_cb, user_data);
    gst_object_unref(bus_snap);

#if C3V_PLATFORM
    if (pipeline_instance->cameraSource_ == CameraSource::RGB) {
        pipeline_instance->cameraFunctionMgr_ = std::make_unique<CameraFunctionMgr>();
    }
#endif

    // load setting
    const char *path = pipeline_instance->cameraSource_ == CameraSource::RGB ? RGB_CONFIG : THERMAL_CONFIG;
    pipeline_instance->cameraConfig_.loadFromFile(path);

    // initial setting
    const CameraParams *camera = &pipeline_instance->cameraConfig_.camera;
    pipeline_instance->setColorPalette(camera->color_map);
    pipeline_instance->setVideoFps(camera->fps);
    pipeline_instance->setVideoBitrate(camera->bitrate);
    pipeline_instance->setVideoResolution(camera->width, camera->height);
    pipeline_instance->setBrightness(camera->brightness);
    pipeline_instance->setContrast(camera->contrast);
    pipeline_instance->setSaturation(camera->saturation);
    pipeline_instance->setHue(camera->hue);
    pipeline_instance->setSharpness(camera->sharpness);
    pipeline_instance->setDenoise(camera->denoise);
    pipeline_instance->setVideoDisplay(camera->display_mode);
    pipeline_instance->setAiTrigger(camera->ai_mode);
    pipeline_instance->setTrackerMode(camera->tracker_mode);

    pipeline_instance->is_recording = false;
    return TRUE;
}

GstPadProbeReturn GStreamerPipeline::event_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GStreamerPipeline *pipeline_instance = static_cast<GStreamerPipeline*>(user_data);

    if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM) {
        GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
        switch (GST_EVENT_TYPE(event)) {
            case GST_EVENT_EOS: {
                g_print("EOS event detected on pad: %s\n", GST_PAD_NAME(pad));
            }
            break;
            default:
            	break;
        }
    }
    else if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_EVENT_UPSTREAM) {
        GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
        switch (GST_EVENT_TYPE(event)) {
            case GST_EVENT_EOS: {
                g_print("EOS event detected on pad: %s\n", GST_PAD_NAME(pad));
            }
            break;
            
            case GST_EVENT_CUSTOM_UPSTREAM: {
                const GstStructure *structure = gst_event_get_structure(event);
                if (gst_structure_has_name(structure, "detect-results-event")) {
                    process_algo_event(pipeline_instance, AlgoEventType::Algo_Detect, structure);
                }
                else if (gst_structure_has_name(structure, "botsort-results-event")) {
                    process_algo_event(pipeline_instance, AlgoEventType::Algo_BotSort_Track, structure);
                }
                else if (gst_structure_has_name(structure, "tracking-results-event")) {
                    process_algo_event(pipeline_instance, AlgoEventType::Algo_OpenCV_Track, structure);
                }
            }
            break;
            
            default:
            	break;
        }
    }

    return GST_PAD_PROBE_OK;
}

void GStreamerPipeline::process_algo_event(GStreamerPipeline *pipeline_instance, AlgoEventType event_type, const GstStructure *structure) {
    if (pipeline_instance->algoNotifyCallback_ == nullptr)
        return;

    const GValue *results_val = gst_structure_get_value(structure, "results");
    if (results_val && G_VALUE_HOLDS_POINTER(results_val)) {
        GPtrArray *algo_results = (GPtrArray *)g_value_get_pointer(results_val);
        if (algo_results) {
            PipelineAlgoEvent event(event_type);
            for (guint i = 0; i < algo_results->len; i++) {
                GstStructure *result_struct = (GstStructure *)g_ptr_array_index(algo_results, i);

                AlgoData data;
                gst_structure_get_int(result_struct, "track_id", &data.track_id);
                gst_structure_get_int(result_struct, "class_id", &data.class_id);
                gst_structure_get_int(result_struct, "confidence", &data.confidence);
                gst_structure_get_int(result_struct, "box_x", &data.rect.x);
                gst_structure_get_int(result_struct, "box_y", &data.rect.y);
                gst_structure_get_int(result_struct, "box_width", &data.rect.width);
                gst_structure_get_int(result_struct, "box_height", &data.rect.height);
                event.data.emplace_back(data);
            }

            pipeline_instance->algoNotifyCallback_(event);
        }
    }
}

int GStreamerPipeline::setTrackingRoi(int x, int y, int width, int height) {
    std::cout << "Setting ROI: x=" << x << ", y=" << y 
              << ", width=" << width << ", height=" << height << std::endl;
    
    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }
    
    if (!GST_IS_ELEMENT(algoprocess)) {
        std::cerr << "Error: algoprocess is not a valid GStreamer element" << std::endl;
        return STATUS_FAILURE;
    }
    
    send_tracking_roi_event(algoprocess, x, y, width, height);
    return STATUS_SUCCESS;
}

void GStreamerPipeline::send_tracking_roi_event(GstElement *element, gint roi_x, gint roi_y, gint roi_width, gint roi_height) {
    GstPad *sink_pad = gst_element_get_static_pad(element, "sink");
    if (!sink_pad) {
        g_print("Failed to get sink pad\n");
        return;
    }

    GstStructure* structure = gst_structure_new("tracking-roi-event",
        "roi_x", G_TYPE_INT, roi_x,
        "roi_y", G_TYPE_INT, roi_y,
        "roi_width", G_TYPE_INT, roi_width,
        "roi_height", G_TYPE_INT, roi_height,
        NULL);
    
    GstEvent *event = gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, structure);

    gboolean result = gst_pad_send_event(sink_pad, event);
    if (!result) {
        g_print("Failed to send TRACKING ROI event\n");
    } else {
        g_print("TRACKING ROI event sent successfully\n");
    }

    gst_object_unref(sink_pad);
}

void GStreamerPipeline::client_connected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data)
{
    g_signal_connect(client, "closed", G_CALLBACK(client_disconnected), user_data);
}

void GStreamerPipeline::client_disconnected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data)
{
    std::cout << "client_disconnected" << std::endl;
}

void GStreamerPipeline::media_unprepared_callback(GstRTSPMedia *media, gpointer user_data)
{
    g_print("====  media_unprepared_callback  ====\n");
    GStreamerPipeline *pipeline_instance = static_cast<GStreamerPipeline*>(user_data);
    g_signal_handlers_disconnect_by_func(media, (gpointer)media_unprepared_callback, user_data);
    if (pipeline_instance->rec_pipeline) {
        gst_element_set_state(pipeline_instance->rec_pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline_instance->rec_pipeline);
        pipeline_instance->rec_pipeline = nullptr;
    }

    if (pipeline_instance->snap_pipeline) {
        gst_element_set_state(pipeline_instance->snap_pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline_instance->snap_pipeline);
        pipeline_instance->snap_pipeline = nullptr;
    }
}

int GStreamerPipeline::getVideoChannel(int &channel)
{
    channel = videoChannel;
    std::cout << "getVideoChannel" << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoResolution(int &resolution)
{
    resolution = 0;

    if (!capsfilter) {
        resolution = 0;
        int width = cameraConfig_.camera.width;
        int height = cameraConfig_.camera.height;
        for (size_t i = 0; i < resolutions.size(); i++) {
            int res_width, res_height;
            std::sscanf(resolutions[i], "%dx%d", &res_width, &res_height);
            if (width == res_width && height == res_height) {
                resolution = i + 1;
                break;
            }
        }

        std::cout << "(config) width: " << width << ", height: " << height << ", resolution: " << resolution << std::endl;
        return STATUS_SUCCESS;
    }

    GstCaps *caps = NULL;
    g_object_get(capsfilter, "caps", &caps, NULL);

    if (caps == NULL) {
        std::cerr << "No caps set on capsfilter." << std::endl;
        return STATUS_FAILURE;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;

    if (gst_structure_get_int(structure, "width", &width) &&
        gst_structure_get_int(structure, "height", &height)) {
        gst_caps_unref(caps);

        for (size_t i = 0; i < resolutions.size(); i++) {
            int res_width, res_height;
            std::sscanf(resolutions[i], "%dx%d", &res_width, &res_height);
            if (width == res_width && height == res_height) {
                resolution = i + 1;
                break;
            }
        }
    } else {
        gst_caps_unref(caps);
        std::cerr << "Failed to get width and height from caps." << std::endl;
        return STATUS_FAILURE;
    }

    std::cout << "resolution: " << width << "x" << height << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoFps(int &fps)
{
    fps = 0;

    if (videorate) {
        g_object_get(G_OBJECT(videorate), "max-rate", &fps, NULL);
        std::cout << "getVideoFps:" << fps << std::endl;
    }
    else {
        fps = cameraConfig_.camera.fps;
        std::cout << "(config) getVideoFps:" << fps << std::endl;
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoBitrate(int &bitrate)
{
    bitrate = 0;

    if (encoder) {
        g_object_get(G_OBJECT(encoder), "bitrate", &bitrate, NULL);
#ifdef V4L2H264ENC
        bitrate = (int)(bitrate / 1000);
#endif
        std::cout << "getVideoBitrate:" << bitrate << std::endl;
    }
    else {
        bitrate = cameraConfig_.camera.bitrate;
        std::cout << "(config) getVideoBitrate:" << bitrate << std::endl;
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoEncode(int &encode)
{
    encode = videoEncode;
    std::cout << "getVideoEncode" << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoZoom(int &zoomlevel)
{
    zoomlevel = videoZoom;
    std::cout << "getVideoZoom" << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getVideoDisplay(int &mode)
{
    mode = 0;

    if (algoprocess) {
        g_object_get(G_OBJECT(algoprocess), "display-mode", &mode, NULL);
        std::cout << "getVideoDisplay:" << mode << std::endl;
    }
    else {
        mode = cameraConfig_.camera.display_mode;
        std::cout << "(config) getVideoDisplay:" << mode << std::endl;
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoChannel(int channel)
{
    videoChannel = channel;
    std::cout << "setVideoChannel, " << channel << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoResolution(int resolution)
{
    resolution--;
    if (resolution < 0 || resolution >= static_cast<int>(resolutions.size())) {
        std::cerr << "Invalid resolution index: " << resolution << ". No changes made." << std::endl;
        return STATUS_FAILURE;
    }    

    int width = 0;
    int height = 0;

    std::sscanf(resolutions[resolution], "%dx%d", &width, &height);

    cameraConfig_.camera.width = width;
    cameraConfig_.camera.height = height;
    cameraConfig_.save();

    if (!pipeline || !capsfilter) {
        std::cerr << "Error: pipeline / capsfilter is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw",
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        NULL);

    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    std::cout << "Resolution updated to: " << width << "x" << height << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoResolution(int width, int height)
{
    int resolution = 0;
    for (size_t i = 0; i < resolutions.size(); i++) {
        int res_width, res_height;
        std::sscanf(resolutions[i], "%dx%d", &res_width, &res_height);
        if (width == res_width && height == res_height) {
            resolution = i + 1;
            break;
        }
    }

    return setVideoResolution(resolution);
}

int GStreamerPipeline::setVideoFps(int fps)
{
    std::cout << "setVideoFps, " << fps << std::endl;
    cameraConfig_.camera.fps = fps;
    cameraConfig_.save();

    if (!pipeline || !videorate) {
        std::cerr << "Error: pipeline / videorate is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(videorate), "max-rate", fps, NULL);

    GstPad *sinkpad = gst_element_get_static_pad(capsfilter, "src");
    gst_pad_send_event(sinkpad, gst_event_new_reconfigure());
    gst_object_unref(sinkpad);

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoBitrate(int bitrate)
{
    cameraConfig_.camera.bitrate = bitrate;
    cameraConfig_.save();
    std::cout << "setVideoBitrate, " << bitrate << std::endl;

    if (!encoder) {
        std::cerr << "Error: encoder is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

#ifdef V4L2H264ENC
    bitrate *= 1000;
#endif

    g_object_set(G_OBJECT(encoder), "bitrate", bitrate, NULL);
    GstElement *rec_encoder = gst_bin_get_by_name(GST_BIN(rec_pipeline), "encoder");
    if (rec_encoder) {
        g_object_set(G_OBJECT(rec_encoder), "bitrate", bitrate, NULL);
        gst_object_unref(rec_encoder);
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoEncode(int encode)
{
    //videoEncode = encode;  //only 0:H264
    std::cout << "setVideoEncode, " << encode << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoZoom(int zoomLevel)
{
    if (!pipeline) {
        std::cerr << "Error: pipeline is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    // 確保 zoomLevel 是正數且有效
    if (zoomLevel < 10) {
        std::cerr << "Invalid zoomLevel, must be > 10" << std::endl;
        return STATUS_FAILURE;
    }

    videoZoom = zoomLevel;

    // 將輸入轉換為實際的縮放倍率
    float zoomFactor = zoomLevel / 10.0f;

    // 獲取 videocrop 元件
    GstElement *videocrop = gst_bin_get_by_name(GST_BIN(pipeline), "videocrop");
    if (!videocrop) {
        std::cerr << "Error: videocrop element not found in pipeline" << std::endl;
        return STATUS_FAILURE;
    }

    // 獲取來源的 Pad
    GstElement *camera = gst_bin_get_by_name(GST_BIN(pipeline), "camera0");
    if (!camera) {
        std::cerr << "Error: camera0 element not found in pipeline" << std::endl;
        return STATUS_FAILURE;
    }

    GstPad *src_pad = gst_element_get_static_pad(camera, "src");
    if (!src_pad) {
        std::cerr << "Error: Failed to get src pad from camera0" << std::endl;
        gst_object_unref(camera);
        gst_object_unref(videocrop);
        return STATUS_FAILURE;
    }

    // 獲取目前的 Caps 並提取寬高資訊
    GstCaps *caps = gst_pad_get_current_caps(src_pad);
    if (!caps) {
        std::cerr << "Error: Failed to get current caps from src pad" << std::endl;
        gst_object_unref(src_pad);
        gst_object_unref(camera);
        gst_object_unref(videocrop);
        return STATUS_FAILURE;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int original_width = 0, original_height = 0;
    if (!gst_structure_get_int(structure, "width", &original_width) ||
        !gst_structure_get_int(structure, "height", &original_height)) {
        std::cerr << "Error: Failed to get width and height from caps" << std::endl;
        gst_caps_unref(caps);
        gst_object_unref(src_pad);                                                                                      
        gst_object_unref(camera);
        gst_object_unref(videocrop);
        return STATUS_FAILURE;
    }

    gst_caps_unref(caps);
    gst_object_unref(src_pad);
    gst_object_unref(camera);

    // 計算顯示比例
    float display_ratio = 1.0f / zoomFactor;

    // 計算需要裁剪的像素數
    int crop_pixels_width = static_cast<int>((1.0f - display_ratio) * original_width / 2.0f);
    int crop_pixels_height = static_cast<int>((1.0f - display_ratio) * original_height / 2.0f);

    // 限制裁剪像素值，避免超過畫面邊界
    crop_pixels_width = std::min(crop_pixels_width, original_width / 2);
    crop_pixels_height = std::min(crop_pixels_height, original_height / 2);
    std::cout << "original_width :" << original_width << "; original_height : " << original_height << std::endl;

    // 設置 videocrop 的裁剪參數
    g_object_set(G_OBJECT(videocrop), 
                 "top", crop_pixels_height,
                 "bottom", crop_pixels_height,
                 "left", crop_pixels_width,
                 "right", crop_pixels_width,
                 NULL);

    gst_object_unref(videocrop);

    std::cout << "Zoom set to level " << zoomLevel 
              << " (factor: " << zoomFactor << ")"
              << ", crop: top=" << crop_pixels_height 
              << ", bottom=" << crop_pixels_height 
              << ", left=" << crop_pixels_width 
              << ", right=" << crop_pixels_width << std::endl;

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setVideoDisplay(int mode)
{
    std::cout << "setVideoDisplay, " << mode << std::endl;
    mode = std::clamp(mode, 0, 3);
    cameraConfig_.camera.display_mode = mode;
    cameraConfig_.save();

    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(algoprocess), "display-mode", mode, NULL);
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getColorPalette(int &type)
{
    type = 0;

    if (algoprocess) {
        g_object_get(G_OBJECT(algoprocess), "colormap-type", &type, NULL);
    }
    else {
        type = cameraConfig_.camera.color_map;
        std::cout << "(config) getColorPalette:" << type << std::endl;
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setColorPalette(int type)
{
    type = std::clamp(type, -1, 9);
    std::cout << "setColorPalette, " << type << std::endl;
    cameraConfig_.camera.color_map = type;
    cameraConfig_.save();

    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(algoprocess), "colormap-type", type, NULL);

    if (rec_pipeline) {
        GstElement *algo = gst_bin_get_by_name(GST_BIN(rec_pipeline), "algoprocess");
        if (algo) {
            g_object_set(G_OBJECT(algo), "colormap-type", type, NULL);
            gst_object_unref(algo);
        }
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::getAiTrigger(int &trigger)
{
    trigger = 0;

    if (algoprocess) {
        g_object_get(G_OBJECT(algoprocess), "processing-mode", &trigger, NULL);
        std::cout << "source: " << cameraSource_ << " getAiTrigger: " << trigger << std::endl;
    }
    else {
        trigger = cameraConfig_.camera.ai_mode;
        std::cout << "source: " << cameraSource_ << " (config) getAiTrigger: " << trigger << std::endl;
    }

    return STATUS_SUCCESS;
}

int GStreamerPipeline::setAiTrigger(int trigger)
{
    trigger = std::clamp(trigger, 0, 3);
    std::cout << "source: " << cameraSource_ << " setAiTrigger, " << trigger << std::endl;
    cameraConfig_.camera.ai_mode = trigger;
    cameraConfig_.save();    

    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(algoprocess), "processing-mode", trigger, NULL);
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getTrackerMode(int &mode)
{
    mode = 0;

    if (algoprocess) {
        g_object_get(G_OBJECT(algoprocess), "tracker-mode", &mode, NULL);
        std::cout << "source: " << cameraSource_ << " getTrackerMode: " << mode << std::endl;
    }
    else {
        mode = cameraConfig_.camera.tracker_mode;
        std::cout << "source: " << cameraSource_ << " (config) getTrackerMode: " << mode << std::endl;
    }
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setTrackerMode(int mode)
{
    mode = std::clamp(mode, 0, 1);
    std::cout << "source: " << cameraSource_ << " setTrackerMode, " << mode << std::endl;
    cameraConfig_.camera.tracker_mode = mode;
    cameraConfig_.save();

    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(algoprocess), "tracker-mode", mode, NULL);
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getTrackingId(int &trackingId)
{
    trackingId = 0;

    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_get(G_OBJECT(algoprocess), "track-id", &trackingId, NULL);
    std::cout << "source: " << cameraSource_  << "getTrackingId: " << trackingId << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setTrackingId(int id)
{
    if (!algoprocess) {
        std::cerr << "Error: algoprocess is not initialized" << std::endl;
        return STATUS_FAILURE;
    }

    g_object_set(G_OBJECT(algoprocess), "track-id", id, NULL);
    std::cout << "source: " << cameraSource_  << "setTrackingId: " << id << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getBrightness(int &value)
{
    value = 0;

    if (!videobalance) {
        value = cameraConfig_.camera.brightness;
        std::cout << "(config) getBrightness:" << value << std::endl;
        return STATUS_SUCCESS;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->getBrightness(value);
        std::cout << "getBrightness:" << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble brightness = 0;
        g_object_get(G_OBJECT(videobalance), "brightness", &brightness, NULL);
        value = std::round(brightness * 10.0);
        std::cout << "getBrightness:" << value << ", brightness: " << brightness << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::getContrast(int &value)
{
    value = 0;

    if (!videobalance) {
        value = cameraConfig_.camera.contrast;
        std::cout << "(config) getContrast:" << value << std::endl;
        return STATUS_SUCCESS;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->getContrast(value);
        std::cout << "getContrast:" << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble contrast = 0;
        g_object_get(G_OBJECT(videobalance), "contrast", &contrast, NULL);
        value = std::round(10.0 * contrast - 10.0);
        std::cout << "getContrast:" << value << ", contrast: " << contrast << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::getSaturation(int &value)
{
    value = 0;

    if (!videobalance) {
        value = cameraConfig_.camera.saturation;
        std::cout << "(config) getSaturation:" << value << std::endl;
        return STATUS_SUCCESS;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->getSaturation(value);
        std::cout << "getSaturation:" << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble saturation = 0;
        g_object_get(G_OBJECT(videobalance), "saturation", &saturation, NULL);
        value = std::round(10.0 * saturation - 10.0);
        std::cout << "getSaturation:" << value << ", saturation: " << saturation << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::getHue(int &value)
{
    value = 0;

    if (!videobalance) {
        value = cameraConfig_.camera.hue;
        std::cout << "(config) getHue:" << value << std::endl;
        return STATUS_SUCCESS;
    }

    gdouble hue = 0;
    g_object_get(G_OBJECT(videobalance), "hue", &hue, NULL);
    value = std::round(hue * 10.0);
    std::cout << "getHue:" << value << ", hue: " << hue << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getSharpness(int &value)
{
    value = 0;

    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Sharpness is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->getSharpness(value);
        std::cout << "getSharpness:" << value << std::endl;
        return status;
    }
    else {
        value = cameraConfig_.camera.sharpness;
        std::cout << "(config) getSharpness:" << value << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::getDenoise(int &value)
{
    value = 0;

    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Denoise is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->getDenoise(value);
        std::cout << "getDenoise:" << value << std::endl;
        return status;
    }
    else {
        value = cameraConfig_.camera.denoise;  
        std::cout << "(config) getDenoise:" << value << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::setBrightness(int value)
{
    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.brightness = value;
    cameraConfig_.save();

    if (!videobalance) {
        std::cerr << "Error: videobalance is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->setBrightness(value);
        std::cout << "setBrightness, " << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble brightness = value / 10.0;
        g_object_set(G_OBJECT(videobalance), "brightness", brightness, NULL);
        std::cout << "setBrightness, " << value << ", brightness: " << brightness << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::setContrast(int value)
{
    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.contrast = value;
    cameraConfig_.save();

    if (!videobalance) {
        std::cerr << "Error: videobalance is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->setContrast(value);
        std::cout << "setContrast, " << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble contrast = (value + 10.0) / 10.0;
        g_object_set(G_OBJECT(videobalance), "contrast", contrast, NULL);
        std::cout << "setContrast, " << value << ", contrast: " << contrast << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::setSaturation(int value)
{
    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.saturation = value;
    cameraConfig_.save();

    if (!videobalance) {
        std::cerr << "Error: videobalance is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    if (cameraFunctionMgr_) {
        int status = cameraFunctionMgr_->setSaturation(value);
        std::cout << "setSaturation, " << value << ", status: " << status << std::endl;
        return status;
    }
    else {
        gdouble saturation = (value + 10.0) / 10.0;
        g_object_set(G_OBJECT(videobalance), "saturation", saturation, NULL);
        std::cout << "setSaturation, " << value << ", saturation: " << saturation << std::endl;
        return STATUS_SUCCESS;
    }
}

int GStreamerPipeline::setHue(int value)
{
    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.hue = value;
    cameraConfig_.save();

    if (!videobalance) {
        std::cerr << "Error: videobalance is not initialized!" << std::endl;
        return STATUS_FAILURE;
    }

    gdouble hue = value / 10.0;
    g_object_set(G_OBJECT(videobalance), "hue", hue, NULL);
    std::cout << "setHue, " << value << ", hue: " << hue << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setSharpness(int value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Sharpness is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.sharpness = value;
    cameraConfig_.save();

    int status = STATUS_SUCCESS;
    if (cameraFunctionMgr_) {
        status = cameraFunctionMgr_->setSharpness(value);
    }

    std::cout << "setSharpness, " << value << std::endl;
    return status;
}

int GStreamerPipeline::setDenoise(int value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Denoise is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = std::clamp(value, -8, 8);
    cameraConfig_.camera.denoise = value;
    cameraConfig_.save();

    int status = STATUS_SUCCESS;
    if (cameraFunctionMgr_) {
        status = cameraFunctionMgr_->setDenoise(value);
    }

    std::cout << "setDenoise, " << value << std::endl;
    return status;
}

int GStreamerPipeline::getTeleZoomSpeed(int &value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Tele Zoom Speed is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = cameraConfig_.camera.tele_zoom_speed;  
    std::cout << "(config) getTeleZoomSpeed:" << value << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::getWideZoomSpeed(int &value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Tele Zoom Speed is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = cameraConfig_.camera.wide_zoom_speed;  
    std::cout << "(config) getWideZoomSpeed:" << value << std::endl;
    return STATUS_SUCCESS;
}

int GStreamerPipeline::setTeleZoomSpeed(int value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Tele Zoom Speed is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = std::clamp(value, 0, 7);
    cameraConfig_.camera.tele_zoom_speed = value;
    cameraConfig_.save();

    int status = STATUS_SUCCESS;
    std::cout << "setTeleZoomSpeed, " << value << std::endl;
    return status;
}

int GStreamerPipeline::setWideZoomSpeed(int value)
{
    if (cameraSource_ != CameraSource::RGB) {
        std::cerr << "Error: Wide Zoom Speed is not supported!" << std::endl;
        return STATUS_FAILURE;
    }

    value = std::clamp(value, 0, 7);
    cameraConfig_.camera.wide_zoom_speed = value;
    cameraConfig_.save();

    int status = STATUS_SUCCESS;
    std::cout << "setWideZoomSpeed, " << value << std::endl;
    return status;
}

void GStreamerPipeline::setNotifyCallback(AlgoNotifyCallback callback)
{
    algoNotifyCallback_ = callback; 
}
