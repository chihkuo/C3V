#include "CameraAPI.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>



CameraAPI::CameraAPI(const std::string& device) {
    if (!openDevice(device)) {
        std::cerr << "Error opening device: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

CameraAPI::~CameraAPI() {
    closeDevice();
}

bool CameraAPI::openDevice(const std::string& device) {
    fd = open(device.c_str(), O_RDWR);
    return fd >= 0;
}

void CameraAPI::closeDevice() {
    if (fd >= 0) {
        close(fd);
    }
}

int CameraAPI::setZoom(int zoomLevel) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
    ctrl.value = zoomLevel;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting zoom: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getZoom() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting zoom: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setSensor_Wide() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_SENSOR_SELECT;
    ctrl.value = SENSOR_MODE_WIDE;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting sensor: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::setSensor_Tele() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_SENSOR_SELECT;
    ctrl.value = SENSOR_MODE_TELE;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting sensor: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getSensorMode() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_SENSOR_SELECT;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting zoom mode: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setZoom_Speed_Normal() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_ZOOM_SPEED;
    ctrl.value = SENSOR_ZOOM_SPEED_NORMAL;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting zoom normal: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::setZoom_Speed_Fast() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_ZOOM_SPEED;
    ctrl.value = SENSOR_ZOOM_SPEED_FAST;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting zoom fast: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::setWhite_Balance(int whitebalance) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
    ctrl.value = whitebalance;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting white balance: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getWhite_Balance() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting white balance: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setBrightness(int brightness) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_BRIGHTNESS;
    ctrl.value = brightness;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting brightness: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getBrightness() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_BRIGHTNESS;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting brightness: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setContrast(int contrast) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_CONTRAST;
    ctrl.value = contrast;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting contrast: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getContrast() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_CONTRAST;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting contrast: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setSaturation(int saturation) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_SATURATION;
    ctrl.value = saturation;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting saturation: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getSaturation() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_SATURATION;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting saturation: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

// unused function

int CameraAPI::setExposure(int exposure) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_EXPOSURE;
    ctrl.value = exposure;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting exposure: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getExposure() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_EXPOSURE;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting exposure: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setExposure_Met(int exposure_met) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_EXPOSURE_METERING;
    ctrl.value = exposure_met;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting exposure_met: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getExposure_Met() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_EXPOSURE_METERING;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting exposure_met: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}


int CameraAPI::setGain(int gain) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_GAIN;
    ctrl.value = gain;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting gain: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getGain() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_GAIN;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting gain: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}


int CameraAPI::setHFlip(int hflip) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_HFLIP;
    ctrl.value = hflip;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting hflip: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getHFlip() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_HFLIP;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting hflip: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}


int CameraAPI::setVFlip(int vflip) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_VFLIP;
    ctrl.value = vflip;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting vflip: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getVFlip() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_VFLIP;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting vflip: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}


int CameraAPI::setGamma(int gamma) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_GAMMA;
    ctrl.value = gamma;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting gamma: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getGamma() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_GAMMA;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting gamma: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setAuto_Focus(int auto_focus) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_FOCUS_AUTO;
    ctrl.value = auto_focus;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting auto_focus: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getAuto_Focus() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_FOCUS_AUTO;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting auto_focus: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setDenoise(int denoise) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_DENOISE;
    ctrl.value = denoise;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting denoise: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getDenoise() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_DENOISE;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting denoise: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}


int CameraAPI::setSharpening(int sharpening) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_SHARPENING;
    ctrl.value = sharpening;
    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting sharpening: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getSharpening() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_SHARPENING;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting sharpening: " << strerror(errno) << std::endl;
        return -1;
    }

    return ctrl.value;
}

int CameraAPI::setFPS(int fps) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_FPS;
    ctrl.value = fps;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting fps: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getFPS() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_FPS;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting fps: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}

int CameraAPI::setResolution(int resolution) {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_RESOLUTION;
    ctrl.value = resolution;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        std::cerr << "Error setting resolution mode: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraAPI::getResolution() {
    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_AP1302_RESOLUTION;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        std::cerr << "Error getting resolution mode: " << strerror(errno) << std::endl;
        return -1;
    }
    return ctrl.value;
}
