#ifndef DBUS_DEFINITION_H
#define DBUS_DEFINITION_H

#define DBUS_NAME    "com.sraytech.service"
#define DBUS_PATH    "/com/sraytech/service"
#define DBUS_INTERFACE  "com.sraytech.service"

constexpr int SRC_NUM = 2;
constexpr int SRC_RGB = 0;
constexpr int SRC_THERMAL = 1;

constexpr int RES_1920X1080 = 1;
constexpr int RES_1280X720 = 2;
constexpr int RES_640X480 = 3;
constexpr int RES_320X240 = 4;

constexpr int STATUS_SUCCESS = 0;
constexpr int STATUS_FAILURE = -1;

// DBUS METHODS NAME
#define SWITCH_SINK         "SwitchSink"

//Video
#define GET_VIDEO_CHANNEL   "GetVideoChannel"
#define GET_VIDEO_RESOLUTION    "GetVideoResolution"
#define GET_VIDEO_FPS       "GetVideoFps"
#define GET_VIDEO_BITRATE   "GetVideoBitrate"
#define GET_VIDEO_ENCODE    "GetVideoEncode"
#define GET_VIDEO_ZOOM      "GetVideoZoom"
#define GET_VIDEO_DISPLAY   "GetVideoDisplay"

#define SET_VIDEO_CHANNEL   "SetVideoChannel"
#define SET_VIDEO_RESOLUTION   "SetVideoResolution"
#define SET_VIDEO_FPS       "SetVideoFps"
#define SET_VIDEO_BITRATE   "SetVideoBitrate"
#define SET_VIDEO_ENCODE    "SetVideoEncode"
#define SET_VIDEO_ZOOM      "SetVideoZoom"
#define SET_VIDEO_DISPLAY   "SetVideoDisplay"

//Thermal
#define GET_COLOR_PALETTE   "GetColorPalette"
#define SET_COLOR_PALETTE   "SetColorPalette"

//AI
#define GET_AI_TRIGGER      "GetAiTrigger"
#define SET_AI_TRIGGER      "SetAiTrigger"

//Tracking
#define GET_TRACKING_ID     "GetTrackingId"
#define SET_TRACKING_ID     "SetTrackingId"
#define SET_CSRT_ROI        "SetCsrtRoi"

#define GET_DETECTION_DATA  "GetDectectionData"
#define GET_TRACK_1_DATA    "GetTrack1Data"
#define GET_TRACK_2_DATA    "GetTrack2Data"

//Laser
#define GET_LASER_DISTANCE "GetLaserDistance"
#define SET_LASER_DISTANCE "SetLaserDistance"

//ColorPalette
#define GET_COLOR_PALETTE "GetColorPalette"
#define SET_COLOR_PALETTE "SetColorPalette"

//Record
#define RECORD_START       "RecordStart"
#define RECORD_STOP        "RecordStop"

//Snapshot
#define SNAPSHOT           "Snapshot"

#endif
