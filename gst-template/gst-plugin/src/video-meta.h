/* GStreamer
 * Copyright (C) <2021> Fluendo S.A. <contact@fluendo.com>
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

#ifndef __GST_VIDEO_META_USER_DATA_UNREGISTERED_H__
#define __GST_VIDEO_META_USER_DATA_UNREGISTERED_H__

#include <gst/gst.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

static const guint8 SEI_UUID[] = {
  0x86, 0x41, 0xc1, 0x40, 0xe9, 0xf6, 0xf6, 0x90,
  0xae, 0x42, 0xc8, 0xc0, 0xcd, 0x69, 0x21, 0x99
};


/**
 * GstVideoMETAUserDataUnregisteredMeta:
 * @meta: parent #GstMeta
 * @uuid: User Data Unregistered UUID
 * @data: Unparsed data buffer
 * @size: Size of the data buffer
 *
 * H.264 H.265 metadata from META User Data Unregistered messages
 *
 * Since: 1.22
 */
typedef struct {
  GstMeta meta;

  guint8 uuid[16];
  guint8 *data;
  gsize size;
  guint8 value;
} GstVideoMETAUserDataUnregisteredMeta;

GST_VIDEO_API
GType gst_video_meta_user_data_unregistered_meta_api_get_type (void);
/**
 * GST_VIDEO_META_USER_DATA_UNREGISTERED_META_API_TYPE:
 *
 * Since: 1.22
 */
#define GST_VIDEO_META_USER_DATA_UNREGISTERED_META_API_TYPE (\
    gst_video_meta_user_data_unregistered_meta_api_get_type())

GST_VIDEO_API
const GstMetaInfo *gst_video_meta_user_data_unregistered_meta_get_info (void);
/**
 * GST_VIDEO_META_USER_DATA_UNREGISTERED_META_INFO:
 *
 * Since: 1.22
 */
#define GST_VIDEO_META_USER_DATA_UNREGISTERED_META_INFO (\
    gst_video_meta_user_data_unregistered_meta_get_info())

/**
 * gst_buffer_get_video_meta_user_data_unregistered_meta:
 * @b: A #GstBuffer
 *
 * Gets the GstVideoMETAUserDataUnregisteredMeta that might be present on @b.
 *
 * Returns: (nullable): The first #GstVideoMETAUserDataUnregisteredMeta present on @b, or %NULL if
 * no #GstVideoMETAUserDataUnregisteredMeta are present
 *
 * Since: 1.22
 */
#define gst_buffer_get_video_meta_user_data_unregistered_meta(b) \
        ((GstVideoMETAUserDataUnregisteredMeta*)gst_buffer_get_meta((b),GST_VIDEO_META_USER_DATA_UNREGISTERED_META_API_TYPE))

GST_VIDEO_API
GstVideoMETAUserDataUnregisteredMeta *gst_buffer_add_video_meta_user_data_unregistered_meta (GstBuffer * buffer,
                                                                                           const guint8 uuid[16],
                                                                                           guint8 * data,
                                                                                           gsize size);

GST_VIDEO_API
gboolean gst_video_meta_user_data_unregistered_parse_precision_time_stamp (GstVideoMETAUserDataUnregisteredMeta * user_data,
                                                                          guint8 * status,
                                                                          guint64 * precision_time_stamp);

G_END_DECLS

#endif /* __GST_VIDEO_META_USER_DATA_UNREGISTERED_H__ */
