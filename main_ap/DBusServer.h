#ifndef DBUS_SERVER_H
#define DBUS_SERVER_H

#include <array>
#include <dbus/dbus.h>
#include "GStreamerPipeline.h"
#include "DBusDefinition.h"
#include "AlgoDataMgr.h"

class DBusServer {
public:
    DBusServer(GStreamerPipeline *gst_pipeline_rgb, GStreamerPipeline *gst_pipeline_thermal);
    ~DBusServer();

    void startServer();
    void listenForMessages();

    void handleCallback(int index, PipelineAlgoEvent& event);

private:
    DBusConnection *connection;
    std::array<GStreamerPipeline*, SRC_NUM> gstPipeline_;
    std::array<AlgoDataMgr, SRC_NUM> algodataMgr_;

    int distance_;

    bool handleMethodCall(DBusMessage *msg);
    bool isValidSrcIndex(int srcIndex);
};

#endif // DBUS_SERVER_H

