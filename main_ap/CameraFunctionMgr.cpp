#include "CameraFunctionMgr.h"

#include <iostream>

CameraFunctionMgr::CameraFunctionMgr()
{
    auto device = "/dev/video36";
    cameraApi = std::make_unique<CameraAPI>(device);
}

CameraFunctionMgr::~CameraFunctionMgr()
{
    
}

int CameraFunctionMgr::getBrightness(int &brightness)
{
    const int brightness_basic = SENSOR_BRIGHTNESS;
    const int brightness_step = SENSOR_BRIGHTNESS_STEP;
    int brightness_val = cameraApi->getBrightness();

    if (brightness_val >= brightness_basic && brightness_val <= brightness_basic + 8 * brightness_step) {
        brightness = (brightness_val - brightness_basic) / brightness_step;
    } 
    else if (brightness_val >= 65536 + (-8) * brightness_step && brightness_val < 65536) {
        brightness = (brightness_val - 65536) / brightness_step;
    } 
    else {
        return -1;
    }

    //std::cout << __func__ << " brightness: " << brightness << ", value: " << brightness_val << std::endl;
    return 0;
}

int CameraFunctionMgr::setBrightness(int brightness)
{
    const int brightness_basic = SENSOR_BRIGHTNESS; 
    const int brightness_step = SENSOR_BRIGHTNESS_STEP;
    int brightness_val = brightness_basic;

    if (brightness >= 0 && brightness <= 8) {
        brightness_val = brightness_basic + brightness * brightness_step;
    }
    else if (brightness < 0 && brightness >= -8) {
        brightness_val = 65536 +  brightness * brightness_step;
    }

    //std::cout << __func__ << " brightness: " << brightness << ", value: " << brightness_val << std::endl;
    return cameraApi->setBrightness(brightness_val);
}

int CameraFunctionMgr::getContrast(int &contrast)
{
    const int contrast_basic = SENSOR_CONTRAST;
    const int contrast_step = SENSOR_CONTRAST_STEP;
    int contrast_val = cameraApi->getContrast();

    if (contrast_val >= contrast_basic && contrast_val <= contrast_basic + 8 * contrast_step) {
        contrast = (contrast_val - contrast_basic) / contrast_step;
    } 
    else if (contrast_val >= 65536 + (-8) * contrast_step && contrast_val < 65536) {
        contrast = (contrast_val - 65536) / contrast_step;
    } 
    else {
        return -1;
    }

    //std::cout << __func__ << " contrast: " << contrast << ", value: " << contrast_val << std::endl;
    return 0;
}

int CameraFunctionMgr::setContrast(int contrast)
{
    const int contrast_basic = SENSOR_CONTRAST; 
    const int contrast_step = SENSOR_CONTRAST_STEP;
    int contrast_val = contrast_basic;

    if (contrast >= 0 && contrast <= 8) {
        contrast_val = contrast_basic + contrast * contrast_step;
    }
    else if (contrast < 0 && contrast >= -8) {
        contrast_val = 65536 +  contrast * contrast_step;
    }

    //std::cout << __func__ << " contrast: " << contrast << ", value: " << contrast_val << std::endl;
    return cameraApi->setContrast(contrast_val);
}

int CameraFunctionMgr::getSaturation(int &saturation)
{
    const int saturation_basic = SENSOR_SATURATION;
    const int saturation_step = SENSOR_SATURATION_STEP;
    int saturation_val = cameraApi->getSaturation();

    saturation = saturation_val / saturation_step - 8;
    //std::cout << __func__ << " saturation: " << saturation << ", value: " << saturation_val << std::endl;
    return 0;
}

int CameraFunctionMgr::setSaturation(int saturation)
{
    const int saturation_basic = SENSOR_SATURATION; 
    const int saturation_step = SENSOR_SATURATION_STEP;
    int saturation_val = saturation_basic;

    if (saturation >= -8 && saturation <= 8) {
        saturation_val = saturation_basic + saturation * saturation_step;
    }

    //std::cout << __func__ << " saturation: " << saturation << ", value: " << saturation_val << std::endl;
    return cameraApi->setSaturation(saturation_val);
}

int CameraFunctionMgr::getSharpness(int &sharpness)
{
    const int sharpness_basic = SENSOR_SHARPNESS; 
    const int sharpness_step = SENSOR_SHARPNESS_SETP;
    int sharpness_val = cameraApi->getSharpening();


    if (sharpness_val >= sharpness_basic && sharpness_val <= sharpness_basic + 8 * sharpness_step) {
        sharpness = (sharpness_val - sharpness_basic) / sharpness_step;
    } 
    else if (sharpness_val >= 65536 + (-8) * sharpness_step && sharpness_val < 65536) {
        sharpness = (sharpness_val - 65536) / sharpness_step;
    } 
    else {
        return -1;
    }

    //std::cout << __func__ << " sharpness: " << sharpness << ", value: " << sharpness_val << std::endl;
    return 0;
}

int CameraFunctionMgr::setSharpness(int sharpness)
{
    const int sharpness_basic = SENSOR_SHARPNESS;
    const int sharpness_step = SENSOR_SHARPNESS_SETP;
    int sharpness_val = sharpness_basic;

    if (sharpness >= 0 && sharpness <= 8) {
        sharpness_val = sharpness_basic + sharpness * sharpness_step;
    }
    else if (sharpness < 0 && sharpness >= -8) {
        sharpness_val = 65536 +  sharpness * sharpness_step;
    }

    //std::cout << __func__ << " sharpness: " << sharpness << ", value: " << sharpness_val << std::endl;
    return cameraApi->setSharpening(sharpness_val);
}

int CameraFunctionMgr::getDenoise(int &denoise)
{
    const int denoise_basic = SENSOR_DENOISE; 
    const int denoise_step = SENSOR_DENOISE_SETP;
    int denoise_val = cameraApi->getDenoise();

    if (denoise_val >= denoise_basic && denoise_val <= denoise_basic + 8 * denoise_step) {
        denoise = (denoise_val - denoise_basic) / denoise_step;
    } 
    else if (denoise_val >= 65536 + (-8) * denoise_step && denoise_val < 65536) {
        denoise = (denoise_val - 65536) / denoise_step;
    } 
    else {
        return -1;
    }

    //std::cout << __func__ << " denoise: " << denoise << ", value: " << denoise_val << std::endl;
    return 0;
}

int CameraFunctionMgr::setDenoise(int denoise)
{
    const int denoise_basic = SENSOR_DENOISE; 
    const int denoise_step = SENSOR_DENOISE_SETP;
    int denoise_val = denoise_basic;

    if (denoise >= 0 && denoise <= 8) {
        denoise_val = denoise_basic + denoise * denoise_step;
    }
    else if (denoise < 0 && denoise >= -8) {
        denoise_val = 65536 +  denoise * denoise_step;
    }

    //std::cout << __func__ << " denoise: " << denoise << ", value: " << denoise_val << std::endl;
    return cameraApi->setDenoise(denoise_val);
}
