#ifndef __DEFINITION_H__
#define __DEFINITION_H__

typedef enum
{
  ALGO_MODEL_RGB,
  ALGO_MODEL_THERMAL,
} AlgoModel;

typedef enum
{
  PROCESSING_DISABLE,
  PROCESSING_DETECT,
  PROCESSING_TRACK_BOTSORT,
  PROCESSING_TRACK_OPENCV,
} ProcessingMode;

typedef enum
{
  DISPLAY_NONE  = 0,
  DISPLAY_BOX   = 1 << 0,
  DISPLAY_TEXT  = 1 << 1,
  DISPLAY_BOX_AND_TEXT = DISPLAY_BOX | DISPLAY_TEXT
} DisplayMode;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} BOX;

typedef struct {
    int track_id;
    int class_id;
    int confidence;
    BOX box;
} RESULT;

typedef struct {
    AlgoModel algo_model;
    unsigned char *img;
    int width;
    int height;
    int data_size;
    const char *format;
    ProcessingMode process_mode;
    int track_id;
    BOX *box;
    DisplayMode display_mode;
    int colormap_type;
    int tracker_mode;
} AlgoParams;

#endif