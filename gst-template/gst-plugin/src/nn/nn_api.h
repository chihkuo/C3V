#ifndef __NN_API_H__
#define __NN_API_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#include "definition.h"

void initialize_func();
void draw_results_and_colormap(const AlgoParams *params, const char *detection_result_json);
int start_image_process(const AlgoParams *params, RESULT **result);
int stop_image_process(int algo_model);
void release_results(RESULT **result);

G_END_DECLS

#endif /* __NN_API_H__ */
