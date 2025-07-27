#include <gst/gst.h>

/* 定義插件結構 */
#define GST_TYPE_MY_PLUGIN (gst_my_plugin_get_type())
G_DECLARE_FINAL_TYPE(GstMyPlugin, gst_my_plugin, GST, MY_PLUGIN, GstElement)

struct _GstMyPlugin {
    GstElement element;
};

G_DEFINE_TYPE(GstMyPlugin, gst_my_plugin, GST_TYPE_ELEMENT);

/* 初始化插件類 */
static void gst_my_plugin_class_init(GstMyPluginClass *klass)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);

    gst_element_class_set_metadata(element_class, "MyPlugin", "Generic", "A simple test plugin", "Author Name <author@example.com>");
}

/* 初始化插件實例 */
static void gst_my_plugin_init(GstMyPlugin *plugin)
{
    /* 初始化代碼，這裡可以不做任何事情 */
}

/* GStreamer 初始化函數 */
static gboolean plugin_init(GstPlugin *plugin)
{
    return gst_element_register(plugin, "myplugin", GST_RANK_NONE, GST_TYPE_MY_PLUGIN);
}

/* 插件描述資訊 */
#define PACKAGE "myplugin"

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    myplugin,
    "A simple test plugin",
    plugin_init,
    "1.0",
    "LGPL",
    "GStreamer",
    "https://gstreamer.freedesktop.org/")
