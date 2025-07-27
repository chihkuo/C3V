#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#include <dbus/dbus.h>
#include <iostream>

class DBusClient {
public:
    DBusClient();
    ~DBusClient();

    int getVideoChannel(int src, int &channel);
    int getVideoResolution(int src, int &resolution);
    int getVideoFps(int src, int &fps);
    int getVideoBitrate(int src, int &bitrate);
    int getVideoEncode(int src, int &encoder);
    int getVideoZoom(int src, int &ratio);
    int getVideoDisplay(int src, int &mdoe);

    int setVideoChannel(int src, int channel);
    int setVideoResolution(int src, int resolution);
    int setVideoFps(int src, int fps);
    int setVideoBitrate(int src, int bitrate);
    int setVideoEncode(int src, int encode);
    int setVideoZoom(int src, int ratio);
    int setVideoDisplay(int src, int display);

    int getBrightness(int src, int &value);
    int getContrast(int src, int &value);
    int getSaturation(int src, int &value);
    int getHue(int src, int &value);
    int getSharpness(int src, int &value);
    int getDenoise(int src, int &value);

    int setBrightness(int src, int value);
    int setContrast(int src, int value);
    int setSaturation(int src, int value);
    int setHue(int src, int value);
    int setSharpness(int src, int value);
    int setDenoise(int src, int value);

    //Thermal
    int getColorPalette(int src, int &type);
    int setColorPalette(int src, int type);

    //AI
    int getAiTrigger(int src, int &mode);
    int setAiTrigger(int src, int mode);

    //Tracking
    int getTrackerMode(int src, int &id);
    int setTrackerMode(int src, int id);
    int getTrackingId(int src, int &id);
    int setTrackingId(int src, int id);
    int setTrackingRoi(int src, int x, int y, int width, int height);
    int setCsrtRoi(int src, int x, int y, int width, int height);
    
    //Laser
    int getLaserDistance(int &distance);
    int setLaserDistance(int distance);    
    
    int getDetectionData(int src, std::string &data);
    int getTrack1Data(int src, std::string &data);
    int getTrack2Data(int src, std::string &data);
    
    //Record / Snapshot
    int recordStart(int src);
    int recordStop(int src);
    int snapshot(int src);

    int getMethod(const char *method, int src, int &value);
    int setMethod(const char *method, int src, int value);
    
private:
    DBusConnection *conn;
    int timeout_ms;
};

#endif // DBUS_CLIENT_H

