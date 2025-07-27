/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2024 Ubuntu-20.04 <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * SECTION:element-algoprocess
 *
 * FIXME:Describe algoprocess here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! algoprocess ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <stdio.h>
#include "gstalgoprocess.h"
#include "video-meta.h"
#include "cJSON.h"

GST_DEBUG_CATEGORY_STATIC (gst_algoprocess_debug);
#define GST_CAT_DEFAULT gst_algoprocess_debug

#define GST_TYPE_ALGO_MODEL \
  (gst_algo_model_get_type ())

static GType
gst_algo_model_get_type (void)
{
  static GType type = 0;
  static const GEnumValue values[] = {
    {GST_ALGO_MODEL_RGB, "Use rgb algorithm model", "rgb-model"},
    {GST_ALGO_MODEL_THERMAL, "Use thermal algorithm model", "thermal-model"},
    {0, NULL, NULL},
  };

  if (!type) {
    type = g_enum_register_static ("GstAlgoModel", values);
  }
  return type;
};

#define DEFAULT_ALGO_MODEL          GST_ALGO_MODEL_RGB


#define GST_TYPE_PROCESSING_MODE \
  (gst_processing_mode_get_type ())

static GType
gst_processing_mode_get_type (void)
{
  static GType type = 0;
  static const GEnumValue values[] = {
    {GST_PROCESSING_DISABLE, "Disable Algorithm Processing", "disable"},
    {GST_PROCESSING_DETECT, "Use detect mode", "detect"},
    {GST_PROCESSING_TRACK_BOTSORT, "Use track 1 mode", "track1"},
    {GST_PROCESSING_TRACK_OPENCV, "Use track 2 mode", "track2"},    
    {0, NULL, NULL},
  };

  if (!type) {
    type = g_enum_register_static ("GstProcessingMode", values);
  }
  return type;
};

#define DEFAULT_PROCESSING_MODE          GST_PROCESSING_DISABLE

#define DEFAULT_TRACK_ID -1
#define MAX_TRACK_ID  65535
#define MIN_TRACK_ID  -1

#define DEFAULT_COLORMAP_TYPE -1
#define MAX_COLORMAP_TYPE  10
#define MIN_COLORMAP_TYPE  -1

#define DEFAULT_TRACKER_MODE 0
#define MAX_TRACKER_MODE  1
#define MIN_TRACKER_MODE  0

#define DEFAULT_DISPLAY_MODE  0
#define MAX_DISPLAY_MODE  3
#define MIN_DISPLAY_MODE  0

#define DEFAULT_EXECUTION_MODE  0
#define MAX_EXECUTION_MODE  1
#define MIN_EXECUTION_MODE  0


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
  PROP_ALGO_MODEL,
  PROP_PROCESSING_MODE,
  PROP_TRACK_ID,
  PROP_COLORMAP_TYPE,
  PROP_TRACKER_MODE,
  PROP_DISPLAY_MODE,
  PROP_EXECUTION_MODE,
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_algoprocess_parent_class parent_class
G_DEFINE_TYPE (Gstalgoprocess, gst_algoprocess, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (algoprocess, "algoprocess", GST_RANK_NONE,
    GST_TYPE_ALGOPROCESS);

static void gst_algoprocess_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_algoprocess_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_algoprocess_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_algoprocess_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

static gchar *g_event_name[] = {
    NULL,
    "detect-results-event",
    "botsort-results-event",
    "tracking-results-event"
};

void send_results_event(GstPad *pad, RESULT *results, int result_size, const gchar *event_name);

static GstStateChangeReturn algoprocess_element_change_state(GstElement *element, GstStateChange transition);

/* initialize the algoprocess's class */
static void
gst_algoprocess_class_init (GstalgoprocessClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_algoprocess_set_property;
  gobject_class->get_property = gst_algoprocess_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_ALGO_MODEL,
      g_param_spec_enum ("algo-model",
          "Select algorithm model",
          "Select algorithm model (RGB or Thermal)",
          GST_TYPE_ALGO_MODEL,
          DEFAULT_ALGO_MODEL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
      );

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_PROCESSING_MODE,
      g_param_spec_enum ("processing-mode",
          "Select processing mode",
          "Select processing mode (disable / detect / track 1 / track 2)",
          GST_TYPE_PROCESSING_MODE,
          DEFAULT_PROCESSING_MODE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
      );

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_TRACK_ID,
      g_param_spec_int ("track-id", "Track ID",
          "Set Track ID", MIN_TRACK_ID, MAX_TRACK_ID, DEFAULT_TRACK_ID,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_COLORMAP_TYPE,
      g_param_spec_int ("colormap-type", "ColorMap Type",
          "Set ColorMap Type", MIN_COLORMAP_TYPE, MAX_COLORMAP_TYPE, DEFAULT_COLORMAP_TYPE,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_TRACKER_MODE,
      g_param_spec_int ("tracker-mode", "Tracker Mode",
          "Set Tracker Mode", MIN_TRACKER_MODE, MAX_TRACKER_MODE, DEFAULT_TRACKER_MODE,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_DISPLAY_MODE,
      g_param_spec_int ("display-mode", "Display Mode",
          "Set Display Mode (0: disable, 1: box, 2: text, 3: box + text)", MIN_DISPLAY_MODE, MAX_DISPLAY_MODE, DEFAULT_DISPLAY_MODE,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_EXECUTION_MODE,
  g_param_spec_int ("execution-mode", "Execution Mode",
      "Set Execution Mode (0: algo , 1: colormap)", MIN_EXECUTION_MODE, MAX_EXECUTION_MODE, DEFAULT_EXECUTION_MODE,
      (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gst_element_class_set_details_simple (gstelement_class,
      "algoprocess",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "Ubuntu-20.04 <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));

  GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
  element_class->change_state = algoprocess_element_change_state;  
}

static GstStateChangeReturn
algoprocess_element_change_state(GstElement *element, GstStateChange transition)
{
    GstStateChangeReturn ret;
    Gstalgoprocess *filter = GST_ALGOPROCESS(element);

    switch (transition) {
        case GST_STATE_CHANGE_READY_TO_NULL:
            if (filter->execution_mode == 0) {
                g_print("GST_STATE_CHANGE_READY_TO_NULL\n");
                g_print("(%d) STOP Thread ID: %p\n", filter->algo_model, (void*)pthread_self());
                stop_image_process(filter->algo_model);
            }
            break;                        
        default:
            break;
    }

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);
    return ret;
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_algoprocess_init (Gstalgoprocess * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_algoprocess_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_algoprocess_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
  filter->processing_mode = GST_PROCESSING_DISABLE;
  filter->preprocessing_mode = GST_PROCESSING_DISABLE;
  filter->track_id = DEFAULT_TRACK_ID;
  filter->colormap_type = DEFAULT_COLORMAP_TYPE;
  filter->tracker_mode = DEFAULT_TRACKER_MODE;
  filter->display_mode = DEFAULT_DISPLAY_MODE;
  filter->execution_mode = DEFAULT_EXECUTION_MODE;
  filter->tracking_box = (BOX){0, 0 , 0, 0};

  initialize_func();
}

static void
gst_algoprocess_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstalgoprocess *filter = GST_ALGOPROCESS (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_ALGO_MODEL:
      filter->algo_model = g_value_get_enum (value);
      break;
    case PROP_PROCESSING_MODE:
      g_print("gs changed processing mode: %d -> %d\n", filter->processing_mode, g_value_get_enum (value)); 
      filter->processing_mode = g_value_get_enum (value);
      break;
    case PROP_TRACK_ID:
      filter->track_id = g_value_get_int (value);
      g_print("gs set trackid: %d\n", filter->track_id); 
      filter->processing_mode = GST_PROCESSING_TRACK_BOTSORT;
      break;
    case PROP_COLORMAP_TYPE:
      filter->colormap_type = g_value_get_int (value);
      break;
    case PROP_TRACKER_MODE:
      filter->tracker_mode = g_value_get_int (value);
      break;      
    case PROP_DISPLAY_MODE:
      filter->display_mode = g_value_get_int (value);
      break;
      case PROP_EXECUTION_MODE:
      filter->execution_mode = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_algoprocess_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstalgoprocess *filter = GST_ALGOPROCESS (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_ALGO_MODEL:
      g_value_set_enum (value, filter->algo_model);
      break;
    case PROP_PROCESSING_MODE:
      g_value_set_enum (value, filter->processing_mode);
      break;
    case PROP_TRACK_ID:
      g_value_set_int (value, filter->track_id);
      break;
    case PROP_COLORMAP_TYPE:
      g_value_set_int (value, filter->colormap_type);
      break;
    case PROP_TRACKER_MODE:
      g_value_set_int (value, filter->tracker_mode);
      break;
    case PROP_DISPLAY_MODE:
      g_value_set_int (value, filter->display_mode);
      break;
    case PROP_EXECUTION_MODE:
      g_value_set_int (value, filter->execution_mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_algoprocess_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  Gstalgoprocess *filter;
  gboolean ret;

  filter = GST_ALGOPROCESS (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CUSTOM_DOWNSTREAM: {

        const GstStructure *structure = gst_event_get_structure(event);

        if (gst_structure_has_name(structure, "tracking-roi-event")) {
            gint roi_x, roi_y, roi_width, roi_height;
            gst_structure_get_int(structure, "roi_x", &roi_x);
            gst_structure_get_int(structure, "roi_y", &roi_y);
            gst_structure_get_int(structure, "roi_width", &roi_width);
            gst_structure_get_int(structure, "roi_height", &roi_height);

            filter->tracking_box.x = roi_x;
            filter->tracking_box.y = roi_y;
            filter->tracking_box.width = roi_width;
            filter->tracking_box.height = roi_height;
            g_print("set new coordinates x=%d, y=%d, width=%d, height=%d \n",roi_x, roi_y, roi_width, roi_height);

            filter->processing_mode = GST_PROCESSING_TRACK_OPENCV;
            ret = TRUE;
            break;
        }
    }
    case GST_EVENT_CAPS: {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      GstStructure *structure = gst_caps_get_structure(caps, 0);
      filter->video_format = (char*)gst_structure_get_string(structure, "format");
      gst_structure_get_int(structure, "width", &filter->video_width);
      gst_structure_get_int(structure, "height", &filter->video_height);
      g_print("GST_EVENT_CAPS, Video Format: %s, Width: %d, Height: %d\n", filter->video_format, filter->video_width, filter->video_height);       

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_algoprocess_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  Gstalgoprocess *filter;

  filter = GST_ALGOPROCESS (parent);

  if (filter->silent == TRUE) {
      return  gst_pad_push (filter->srcpad, buf);
  }

  if (filter->processing_mode != filter->preprocessing_mode) {
      stop_image_process(filter->algo_model);
      filter->preprocessing_mode = filter->processing_mode;
  }

  if (filter->processing_mode == GST_PROCESSING_DISABLE && filter->colormap_type < 0) {
      return  gst_pad_push (filter->srcpad, buf);
  }

  if (!gst_buffer_is_writable(buf)) {
      buf = gst_buffer_make_writable(buf);
      if (!buf) {
          GST_ERROR_OBJECT(pad, "Failed to make the buffer writable");
          return GST_FLOW_ERROR;
      }
  }

  char *detection_result = NULL;
  GstMapInfo map;
  if (gst_buffer_map(buf, &map, GST_MAP_WRITE)) {

      RESULT *results = NULL;
      int result_size = 0;

      AlgoParams params = {
          .algo_model = filter->algo_model,
          .img = map.data,
          .width = filter->video_width,
          .height = filter->video_height,
          .data_size = map.size,
          .format = filter->video_format,
          .process_mode = (ProcessingMode)filter->processing_mode,
          .track_id = filter->track_id,
          .box = &filter->tracking_box,
          .display_mode = (DisplayMode)filter->display_mode,
          .colormap_type = filter->colormap_type,
          .tracker_mode = filter->tracker_mode
      };

      if (filter->processing_mode != GST_PROCESSING_DISABLE) {
          result_size = start_image_process(&params, &results);
          send_results_event(filter->sinkpad, results, result_size, g_event_name[(int)filter->processing_mode]);
      }

      if (result_size) {
          cJSON *result_array = cJSON_CreateArray();
          cJSON *detection = cJSON_CreateObject();

          for (int i = 0; i < result_size; i++) {
              cJSON *result = cJSON_CreateObject();
              cJSON_AddNumberToObject(result, "track_id", results[i].track_id);
              cJSON_AddNumberToObject(result, "class_id", results[i].class_id);
              cJSON_AddNumberToObject(result, "x", results[i].box.x);
              cJSON_AddNumberToObject(result, "y", results[i].box.y);
              cJSON_AddNumberToObject(result, "width", results[i].box.width);
              cJSON_AddNumberToObject(result, "height", results[i].box.height);
              cJSON_AddNumberToObject(result, "confidence", results[i].confidence);
              cJSON_AddItemToArray(result_array, result);
          }
          
          release_results(&results);

          cJSON_AddItemToObject(detection, "results", result_array);

          detection_result = cJSON_PrintUnformatted(detection);
          int detection_size = strlen(detection_result);

          gst_buffer_add_video_meta_user_data_unregistered_meta(buf, SEI_UUID, (guint8 *)detection_result, detection_size);
          cJSON_Delete(detection);
      }

      draw_results_and_colormap(&params, detection_result);

      if (detection_result) {
          free(detection_result);
          detection_result = NULL;
      }

      gst_buffer_unmap(buf, &map);
  }

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}

void send_results_event(GstPad *pad, RESULT *results, int result_size, const gchar *event_name)
{
  GPtrArray *algo_results = g_ptr_array_new_with_free_func((GDestroyNotify)gst_structure_free);

  for (int i = 0; i < result_size; i++) {
      GstStructure *result_struct = gst_structure_new(
          "result",
          "track_id", G_TYPE_INT, results[i].track_id,
          "class_id", G_TYPE_INT, results[i].class_id,
          "confidence", G_TYPE_INT, results[i].confidence,
          "box_x", G_TYPE_INT, results[i].box.x,
          "box_y", G_TYPE_INT, results[i].box.y,
          "box_width", G_TYPE_INT, results[i].box.width,
          "box_height", G_TYPE_INT, results[i].box.height,
          NULL);
      
      g_ptr_array_add(algo_results, result_struct);
  }

  GValue results_val = G_VALUE_INIT;
  g_value_init(&results_val, G_TYPE_POINTER);
  g_value_set_pointer(&results_val, algo_results);

  GstStructure *event_payload = gst_structure_new_empty(event_name);
  gst_structure_set_value(event_payload, "results", &results_val);

  GstEvent *event = gst_event_new_custom(GST_EVENT_CUSTOM_UPSTREAM, event_payload);
  gst_pad_push_event(pad, event);

  g_value_unset(&results_val);
  g_ptr_array_unref(algo_results);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
algoprocess_init (GstPlugin * algoprocess)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template algoprocess' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_algoprocess_debug, "algoprocess",
      0, "Template algoprocess");

  return GST_ELEMENT_REGISTER (algoprocess, algoprocess);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstalgoprocess"
#endif

/* gstreamer looks for this structure to register algoprocesss
 *
 * exchange the string 'Template algoprocess' with your algoprocess description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    algoprocess,
    "algoprocess",
    algoprocess_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
