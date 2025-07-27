/* GStreamer
 * Copyright (C) 2021 Fluendo S.A. <support@fluendo.com>
 *   Authors: Andoni Morales Alastruey <amorales@fluendo.com>
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string.h>
#include <gst/base/gstbytereader.h>
#include "video-meta.h"

/**
 * SECTION:gstvideometa
 * @title: GstVideo META Unregistered User Data
 * @short_description: Utilities for META User Data Unregistered
 *
 * A collection of objects and methods to assist with META User Data Unregistered
 * metadata in H.264 and H.265 streams.
 *
 * Since: 1.22
 */

#ifndef GST_DISABLE_GST_DEBUG
#define GST_CAT_DEFAULT ensure_debug_category()
static GstDebugCategory *
ensure_debug_category (void)
{
  static gsize cat_gonce = 0;

  if (g_once_init_enter (&cat_gonce)) {
    gsize cat_done;

    cat_done = (gsize) _gst_debug_category_new ("video-meta", 0,
        "H.264 / H.265 META messages utilities");

    g_once_init_leave (&cat_gonce, cat_done);
  }

  return (GstDebugCategory *) cat_gonce;
}
#else
#define ensure_debug_category1() /* NOOP */
#endif /* GST_DISABLE_GST_DEBUG */

/* META User Data Unregistered implementation */

/**
 * gst_video_meta_user_data_unregistered_meta_api_get_type:
 *
 * Returns: #GType for the #GstVideoMETAUserDataUnregisteredMeta structure.
 *
 * Since: 1.22
 */
GType
gst_video_meta_user_data_unregistered_meta_api_get_type (void)
{
  static GType type = 0;

  if (g_once_init_enter (&type)) {
    static const gchar *tags[] = {
      GST_META_TAG_VIDEO_STR,
      NULL
    };
    GType _type =
        gst_meta_api_type_register ("GstVideoMETAUserDataUnregisteredMetaAPI",
        tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean
gst_video_meta_user_data_unregistered_meta_init (GstMeta * meta, gpointer params,
    GstBuffer * buffer)
{
  GstVideoMETAUserDataUnregisteredMeta *emeta =
      (GstVideoMETAUserDataUnregisteredMeta *) meta;

  emeta->data = NULL;
  emeta->size = 0;

  return TRUE;
}

static gboolean
gst_video_meta_user_data_unregistered_meta_transform (GstBuffer * dest,
    GstMeta * meta, GstBuffer * buffer, GQuark type, gpointer data)
{
  GstVideoMETAUserDataUnregisteredMeta *smeta =
      (GstVideoMETAUserDataUnregisteredMeta *) meta;

  if (GST_META_TRANSFORM_IS_COPY (type)) {
    GST_DEBUG ("copy META User Data Unregistered metadata");
    gst_buffer_add_video_meta_user_data_unregistered_meta (dest,
        smeta->uuid, smeta->data, smeta->size);
    return TRUE;
  } else {
    /* return FALSE, if transform type is not supported */
    return FALSE;
  }
}

static void
gst_video_meta_user_data_unregistered_meta_free (GstMeta * meta, GstBuffer * buf)
{
  GstVideoMETAUserDataUnregisteredMeta *smeta =
      (GstVideoMETAUserDataUnregisteredMeta *) meta;

  g_free (smeta->data);
  smeta->data = NULL;
}

/**
 * gst_video_meta_user_data_unregistered_meta_get_info:
 *
 * Returns: #GstMetaInfo pointer that describes #GstVideoMETAUserDataUnregisteredMeta.
 *
 * Since: 1.22
 */
const GstMetaInfo *
gst_video_meta_user_data_unregistered_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter ((GstMetaInfo **) & meta_info)) {
    const GstMetaInfo *mi =
        gst_meta_register (GST_VIDEO_META_USER_DATA_UNREGISTERED_META_API_TYPE,
        "GstVideoMETAUserDataUnregisteredMeta",
        sizeof (GstVideoMETAUserDataUnregisteredMeta),
        gst_video_meta_user_data_unregistered_meta_init,
        gst_video_meta_user_data_unregistered_meta_free,
        gst_video_meta_user_data_unregistered_meta_transform);
    g_once_init_leave ((GstMetaInfo **) & meta_info, (GstMetaInfo *) mi);
  }
  return meta_info;
}

/**
 * gst_buffer_add_video_meta_user_data_unregistered_meta:
 * @buffer: a #GstBuffer
 * @uuid: User Data Unregistered UUID
 * @data: (transfer none): META User Data Unregistered buffer
 * @size: size of the data buffer
 *
 * Attaches #GstVideoMETAUserDataUnregisteredMeta metadata to @buffer with the given
 * parameters.
 *
 * Returns: (transfer none): the #GstVideoMETAUserDataUnregisteredMeta on @buffer.
 *
 * Since: 1.22
 */
GstVideoMETAUserDataUnregisteredMeta *
gst_buffer_add_video_meta_user_data_unregistered_meta (GstBuffer * buffer,
    const guint8 uuid[16], guint8 * data, gsize size)
{
  GstVideoMETAUserDataUnregisteredMeta *meta;
  g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);
  g_return_val_if_fail (data != NULL, NULL);

  meta = (GstVideoMETAUserDataUnregisteredMeta *) gst_buffer_add_meta (buffer,
      GST_VIDEO_META_USER_DATA_UNREGISTERED_META_INFO, NULL);
  g_assert (meta != NULL);
  memcpy (meta->uuid, uuid, 16);
  meta->data = g_malloc (size);
  memcpy (meta->data, data, size);
  meta->size = size;

  return meta;
}