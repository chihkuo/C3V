/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2024 Ubuntu-20.04 <<user@hostname.org>>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-setseidata
 *
 * FIXME:Describe setseidata here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! setseidata ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
// #include <gst/base/base.h>
// #include <gst/controller/controller.h>

#include "gstsetseidata.h"
#include "video-meta.h"
#include "cJSON.h"
#include <time.h>
#include <math.h>


GST_DEBUG_CATEGORY_STATIC (gst_setseidata_debug);
#define GST_CAT_DEFAULT gst_setseidata_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
};

/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_setseidata_parent_class parent_class
G_DEFINE_TYPE (Gstsetseidata, gst_setseidata, GST_TYPE_BASE_TRANSFORM);
GST_ELEMENT_REGISTER_DEFINE (setseidata, "setseidata", GST_RANK_NONE,
    GST_TYPE_SETSEIDATA);

static void gst_setseidata_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_setseidata_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_setseidata_transform_ip (GstBaseTransform *
    base, GstBuffer * outbuf);

/* GObject vmethod implementations */

/* initialize the setseidata's class */
static void
gst_setseidata_class_init (GstsetseidataClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_setseidata_set_property;
  gobject_class->get_property = gst_setseidata_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  gst_element_class_set_details_simple (gstelement_class,
      "setseidata",
      "Generic/Filter",
      "FIXME:Generic Template Filter", "Ubuntu-20.04 <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_setseidata_transform_ip);

  /* debug category for fltering log messages
   *
   * FIXME:exchange the string 'Template setseidata' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_setseidata_debug, "setseidata", 0,
      "Template setseidata");
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_setseidata_init (Gstsetseidata * filter)
{
  srand(time(NULL));
  filter->silent = FALSE;
}

static void
gst_setseidata_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstsetseidata *filter = GST_SETSEIDATA (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_setseidata_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstsetseidata *filter = GST_SETSEIDATA (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

//
int generateRandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

double generateRandomDouble(double min, double max) {
    double scale = rand() / (double) RAND_MAX; // [0, 1.0]
    double value = min + scale * (max - min);  // [min, max]
    return round(value * 100) / 100; // 保留两位小数
}


/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_setseidata_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  Gstsetseidata *filter = GST_SETSEIDATA (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));


  // if (filter->silent == FALSE)
  //   g_print ("I'm plugged, therefore I'm in.\n");
  if (filter->silent == TRUE) {
    g_print ("silent = TRUE\n");
    return GST_FLOW_OK;
  }

  // time_t now = time(NULL);
  // struct tm *t = localtime(&now);
  // guint8 sei_data[64];
  // strftime((char*)sei_data, sizeof(sei_data), "SRAY_SEI_DATA=%Y/%m/%d %H:%M:%S", t);

  // // Add the SEI data to the buffer as metadata
  // GstVideoMETAUserDataUnregisteredMeta* meta =  gst_buffer_add_video_meta_user_data_unregistered_meta(outbuf, SEI_UUID, sei_data, sizeof(sei_data));

    //srand(time(NULL));

    cJSON *results = cJSON_CreateArray();
    cJSON *detection = cJSON_CreateObject();

    int numResults = generateRandomInt(1, 10);
    for (int i = 0; i < numResults; ++i) {
        cJSON *result = cJSON_CreateObject();
        cJSON_AddNumberToObject(result, "class_id", generateRandomInt(0, 10));
        cJSON_AddNumberToObject(result, "x", generateRandomInt(0, 580));
        cJSON_AddNumberToObject(result, "y", generateRandomInt(0, 420));
        cJSON_AddNumberToObject(result, "width", generateRandomInt(5, 50));
        cJSON_AddNumberToObject(result, "height", generateRandomInt(5, 50));
        cJSON_AddNumberToObject(result, "confidence", generateRandomDouble(0.01, 1.0));
        cJSON_AddItemToArray(results, result);
    }

    cJSON_AddItemToObject(detection, "results", results);

    char *detection_result = cJSON_PrintUnformatted(detection);
    int detection_size = strlen(detection_result);

    //printf("Detection Result: %s\n", detection_result);
    //printf("Detection Size: %d\n", detection_size);

  // Add the SEI data to the buffer as metadata
  GstVideoMETAUserDataUnregisteredMeta* meta =  gst_buffer_add_video_meta_user_data_unregistered_meta(outbuf, SEI_UUID, detection_result, detection_size);

  cJSON_Delete(detection);
  free(detection_result);    


  if (meta) {
      //g_print("%s, SEI Data: %s\n", __func__, meta->data);
  } else {
      g_print("%s, Failed to add SEI meta!\n", __func__);
  }      


  // GstVideoMETAUserDataUnregisteredMeta *ret = gst_buffer_get_video_meta_user_data_unregistered_meta(outbuf);
  
  // if (ret) {
  //     //g_print("JSON Data: %s\n", ret->data);
  // } else {
  //     g_print("JSON Failed to read SEI meta!\n");
  // }  

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
setseidata_init (GstPlugin * setseidata)
{
  return GST_ELEMENT_REGISTER (setseidata, setseidata);
}

/* gstreamer looks for this structure to register setseidatas
 *
 * FIXME:exchange the string 'Template setseidata' with you setseidata description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    setseidata,
    "setseidata",
    setseidata_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
