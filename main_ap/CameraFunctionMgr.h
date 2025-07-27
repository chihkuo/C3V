#ifndef CAMERA_FUNCTION_MGR_H
#define CAMERA_FUNCTION_MGR_H

#include <memory>

#include "CameraAPI.h"


class CameraFunctionMgr {
    public:
        CameraFunctionMgr();
        ~CameraFunctionMgr();

        int getBrightness(int &brightness);
        int setBrightness(int brightness);

        int getContrast(int &contrast);
        int setContrast(int contrast);

        int getSaturation(int &saturation);
        int setSaturation(int saturation);

        int getSharpness(int &sharpness);
        int setSharpness(int sharpness);

        int getDenoise(int &denoise);
        int setDenoise(int denoise);

    private:
        std::unique_ptr<CameraAPI> cameraApi;
};

#endif