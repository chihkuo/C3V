#ifndef CAMERA_API_H
#define CAMERA_API_H

#include <linux/videodev2.h>
#include <string>

#ifndef V4L2_CID_USER_CCS_BASE
#define V4L2_CID_USER_CCS_BASE		(V4L2_CID_USER_BASE + 0x10f0)
#endif
#define V4L2_CID_USER_AP1302_BASE		(V4L2_CID_USER_CCS_BASE + 128)
#define V4L2_CID_AP1302_STEREO_ORDER	(V4L2_CID_USER_AP1302_BASE + 0)
#define V4L2_CID_AP1302_SENSOR_SELECT	(V4L2_CID_USER_AP1302_BASE + 1)
#define V4L2_CID_AP1302_ZOOM_SPEED	(V4L2_CID_USER_AP1302_BASE + 2)
#define V4L2_CID_AP1302_DENOISE		(V4L2_CID_USER_AP1302_BASE + 3)
#define V4L2_CID_AP1302_SHARPENING	(V4L2_CID_USER_AP1302_BASE + 4)
#define V4L2_CID_AP1302_FPS		(V4L2_CID_USER_AP1302_BASE + 5)
#define V4L2_CID_AP1302_RESOLUTION	(V4L2_CID_USER_AP1302_BASE + 6)

#define SENSOR_MODE_TELE 1085
#define SENSOR_MODE_WIDE 1086

#define SENSOR_ZOOM_SPEED_NORMAL 32768
#define SENSOR_ZOOM_SPEED_FAST	 512

#define SENSOR_BRIGHTNESS 0
#define SENSOR_CONTRAST 0
#define SENSOR_SATURATION 4096
#define SENSOR_SHARPNESS 0
#define SENSOR_DENOISE 0

#define SENSOR_ZOOM_RATIO_1X 256
#define SENSOR_BRIGHTNESS_STEP 512
#define SENSOR_CONTRAST_STEP 2560
#define SENSOR_SATURATION_STEP 512
#define SENSOR_SHARPNESS_SETP 2048
#define SENSOR_DENOISE_SETP 2048

class CameraAPI {
public:
    CameraAPI(const std::string& device);
    ~CameraAPI();

    int setZoom(int zoomLevel);
    int getZoom();
    int setSensor_Tele();
    int setSensor_Wide();
    int getSensorMode();

    int setZoom_Speed_Normal();
    int setZoom_Speed_Fast();

    int setWhite_Balance(int whitebalance);
    int getWhite_Balance();

    // for adjust
    int setBrightness(int brightness);
    int getBrightness();
    int setContrast(int contrast);
    int getContrast();
    int setSaturation(int saturation);
    int getSaturation();

    int setDenoise(int denoise);
    int getDenoise();
    int setSharpening(int sharpening);
    int getSharpening();
    int setFPS(int fps);
    int getFPS();
    int setResolution(int resolution);
    int getResolution();

    //unused
    int setExposure(int exposure);
    int getExposure();
    int setExposure_Met(int exposure_met);
    int getExposure_Met();
    int setGain(int gain);
    int getGain();
    int setHFlip(int hflip);
    int getHFlip();
    int setVFlip(int vflip);
    int getVFlip();
    int setGamma(int gamma);
    int getGamma();
    int setAuto_Focus(int auto_focus);
    int getAuto_Focus();
    

private:
    int fd;
    bool openDevice(const std::string& device);
    void closeDevice();
};

#endif // CAMERA_API_H