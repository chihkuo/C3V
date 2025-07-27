#include "GStreamerPipeline.h"
#include "DBusServer.h"

#include <iostream>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

struct AttachArgs {
    GStreamerPipeline *pipeline;
    const char *path;
};

gpointer attachPipelineToServer(gpointer data) {
    gst_init(nullptr, nullptr);
    auto* args = static_cast<AttachArgs*>(data);

    GstRTSPServer *server = gst_rtsp_server_new();
#ifdef C3V_RELEASE
    gst_rtsp_server_set_service(server, "554");
#endif
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    args[0].pipeline->attachToServer(server, mounts, args[0].path);
    args[1].pipeline->attachToServer(server, mounts, args[1].path);

    g_signal_connect(server, "client-connected", G_CALLBACK(args[0].pipeline->client_connected), args[0].pipeline);

    g_object_unref(mounts);
    gst_rtsp_server_attach(server, nullptr);

    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    return nullptr;
}

std::string getIPAddresses() {
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;
    void* tmpAddrPtr = nullptr;

    if (getifaddrs(&ifAddrStruct) == -1) {
        perror("getifaddrs");
        return std::string();
    }

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        if (std::string(ifa->ifa_name) == "lo") {
            continue;
        }        

        if (ifa->ifa_addr->sa_family == AF_INET) { 
            tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            return std::string(addressBuffer);
        }
    }

    if (ifAddrStruct != nullptr) {
        freeifaddrs(ifAddrStruct);
    }

    return std::string();
}


int main(int argc, char *argv[]) {
    GStreamerPipeline gst_pipeline_rgb(CameraSource::RGB);
    GStreamerPipeline gst_pipeline_thermal(CameraSource::THERMAL);

    AttachArgs args[2] = {
        {&gst_pipeline_rgb, "/eo"},
        {&gst_pipeline_thermal, "/ir"}
    };

    GThread* gst_server_thread = g_thread_new("GStreamerPipeline", attachPipelineToServer, &args);

    std::string strIp = getIPAddresses();
    int port = 8554;
#ifdef C3V_RELEASE
    port = 554;
#endif
    std::cout << "RTSP streams available:\n"
              << "  RGB    : rtsp://"<< strIp <<":" << port << "/eo\n"
              << "  Thermal: rtsp://"<< strIp <<":" << port << "/ir\n";

    DBusServer dbus_server(&gst_pipeline_rgb, &gst_pipeline_thermal);

    GThread *dbus_thread = g_thread_new("DBusServer", [](gpointer data) -> gpointer {
        DBusServer *server = static_cast<DBusServer*>(data);
        server->startServer();
        return nullptr;
    }, &dbus_server);

    g_thread_join(gst_server_thread);
    g_thread_join(dbus_thread);

    return 0;
}
