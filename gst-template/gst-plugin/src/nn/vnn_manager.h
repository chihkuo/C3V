
#pragma once

#include "vsi_nn_pub.h"

#include "vnn_global.h"
#include "yolo.h"
#include <opencv2/opencv.hpp>

#define VNN_APP_DEBUG (FALSE)
#define VNN_VERSION_MAJOR 1
#define VNN_VERSION_MINOR 1
#define VNN_VERSION_PATCH 30
#define VNN_RUNTIME_VERSION \
    (VNN_VERSION_MAJOR * 10000 + VNN_VERSION_MINOR * 100 + VNN_VERSION_PATCH)

_version_assert(VNN_RUNTIME_VERSION <= VSI_NN_VERSION,
                CASE_VERSION_is_higher_than_OVXLIB_VERSION)
                

enum VNN_Mode {
    evm_YOLOv7_RGB,
    evm_YOLOv7_THERMAL
};

//main
#define NEW_VXNODE(_node, _type, _in, _out, _uid) do {\
        _node = vsi_nn_AddNode( graph, _type, _in, _out, NULL );\
        if( NULL == _node ) {\
            goto error;\
        }\
        _node->uid = (uint32_t)_uid;\
    } while(0)

#define NEW_VIRTUAL_TENSOR(_id, _attr, _dtype) do {\
        memset( _attr.size, 0, VSI_NN_MAX_DIM_NUM * sizeof(vsi_size_t));\
        _attr.dim_num = VSI_NN_DIM_AUTO;\
        _attr.vtl = !VNN_APP_DEBUG;\
        _attr.is_const = FALSE;\
        _attr.dtype.vx_type = _dtype;\
        _id = vsi_nn_AddTensor( graph, VSI_NN_TENSOR_ID_AUTO,\
                & _attr, NULL );\
        if( VSI_NN_TENSOR_ID_NA == _id ) {\
            goto error;\
        }\
    } while(0)

// Set const tensor dims out of this macro.
#define NEW_CONST_TENSOR(_id, _attr, _dtype, _ofst, _size) do {\
        data = load_data( fp, _ofst, _size  );\
        if( NULL == data ) {\
            goto error;\
        }\
        _attr.vtl = FALSE;\
        _attr.is_const = TRUE;\
        _attr.dtype.vx_type = _dtype;\
        _id = vsi_nn_AddTensor( graph, VSI_NN_TENSOR_ID_AUTO,\
                & _attr, data );\
        free( data );\
        if( VSI_NN_TENSOR_ID_NA == _id ) {\
            goto error;\
        }\
    } while(0)

// Set generic tensor dims out of this macro.
#define NEW_NORM_TENSOR(_id, _attr, _dtype) do {\
        _attr.vtl = FALSE;\
        _attr.is_const = FALSE;\
        _attr.dtype.vx_type = _dtype;\
        if ( enable_from_handle )\
        {\
            _id = vsi_nn_AddTensorFromHandle( graph, VSI_NN_TENSOR_ID_AUTO,\
                    & _attr, NULL );\
        }\
        else\
        {\
            _id = vsi_nn_AddTensor( graph, VSI_NN_TENSOR_ID_AUTO,\
                    & _attr, NULL );\
        }\
        if( VSI_NN_TENSOR_ID_NA == _id ) {\
            goto error;\
        }\
    } while(0)

// Set generic tensor dims out of this macro.
#define NEW_NORM_TENSOR_FROM_HANDLE(_id, _attr, _dtype) do {\
        _attr.vtl = FALSE;\
        _attr.is_const = FALSE;\
        _attr.dtype.vx_type = _dtype;\
        _id = vsi_nn_AddTensorFromHandle( graph, VSI_NN_TENSOR_ID_AUTO,\
                & _attr, NULL );\
        if( VSI_NN_TENSOR_ID_NA == _id ) {\
            goto error;\
        }\
    } while(0)

#define NET_NODE_NUM            (1)
#define NET_NORM_TENSOR_NUM     (4)
#define NET_CONST_TENSOR_NUM    (0)
#define NET_VIRTUAL_TENSOR_NUM  (3)
#define NET_TOTAL_TENSOR_NUM    (NET_NORM_TENSOR_NUM + NET_CONST_TENSOR_NUM + NET_VIRTUAL_TENSOR_NUM)

//pre
#define INPUT_META_NUM 1

typedef enum _vnn_file_type
{
    NN_FILE_NONE,
    NN_FILE_TENSOR,
    NN_FILE_QTENSOR,
    NN_FILE_JPG,
    NN_FILE_BINARY
} vnn_file_type_e;

typedef enum _vnn_pre_order
{
    VNN_PREPRO_NONE = -1,
    VNN_PREPRO_REORDER,
    VNN_PREPRO_MEAN,
    VNN_PREPRO_SCALE,
    VNN_PREPRO_NUM
} vnn_pre_order_e;

typedef struct _vnn_input_meta
{
    union
    {
        struct
        {
            int32_t preprocess[VNN_PREPRO_NUM];
            uint32_t reorder[4];
            float mean[4];
            float scale[4];
            int32_t channel_count;
        } image;
    };
} vnn_input_meta_t;

class VNN_Manager
{
public:
    VNN_Manager(VNN_Mode mode = evm_YOLOv7_RGB);
    ~VNN_Manager();

    VNN_Mode mode() { return m_mode; };

    vsi_status vnn_PreProcessNeuralNetwork(cv::Mat &frame);
    vsi_status vnn_ProcessGraph();
    vsi_status vnn_GetResult(std::vector<Output>& output);

private:
    vsi_status vnn_CreateNeuralNetwork();
    vsi_status vnn_ReleaseNeuralNetwork();
    vsi_status vnn_GetOutputTensor(std::vector<cv::Mat> &outputTensor);

    vsi_nn_graph_t *m_graph;
    Yolo m_yolo;
    cv::Size m_input_size;
    VNN_Mode m_mode;

    //main
    void vnn_ReleaseYolov7TinyUint8
        (
        vsi_nn_graph_t * graph,
        vsi_bool release_ctx
        );

    vsi_nn_graph_t * vnn_CreateYolov7TinyUint8
        (
        const char * data_file_name,
        vsi_nn_context_t in_ctx,
        const vsi_nn_preprocess_map_element_t * pre_process_map,
        uint32_t pre_process_map_count,
        const vsi_nn_postprocess_map_element_t * post_process_map,
        uint32_t post_process_map_count
        );    

    void vnn_ReleaseYolov7TUint8
        (
        vsi_nn_graph_t * graph,
        vsi_bool release_ctx
        );

    vsi_nn_graph_t * vnn_CreateYolov7TUint8
        (
        const char * data_file_name,
        vsi_nn_context_t in_ctx,
        const vsi_nn_preprocess_map_element_t * pre_process_map,
        uint32_t pre_process_map_count,
        const vsi_nn_postprocess_map_element_t * post_process_map,
        uint32_t post_process_map_count
        );

    vsi_status vnn_PreProcessYolov7TUint8(
        vsi_nn_graph_t *graph,
        cv::Mat &frame);
    
    void _load_input_meta();
    vsi_status _handle_frame_input(vsi_nn_graph_t *graph, cv::Mat &frame);
    uint8_t *_get_frame_data(vsi_nn_tensor_t *tensor, vnn_input_meta_t *meta, cv::Mat &frame);
    float *_imageData_to_float32(uint8_t *bmpData, vsi_nn_tensor_t *tensor);
    void _data_transform(float *fdata, vnn_input_meta_t *meta, vsi_nn_tensor_t *tensor);
    void _data_transform(cv::Mat &m, vnn_input_meta_t *meta, vsi_nn_tensor_t *tensor);
    void _data_scale(float *fdata, vnn_input_meta_t *meta, vsi_nn_tensor_t *tensor);

    vsi_status vnn_PreTableInit(vsi_nn_graph_t *graph);

    cv::Mat _dtype_to_float32(uint8_t *data, vsi_nn_tensor_t *tensor, uint8_t idx);
    uint8_t *_float32_to_dtype(float *fdata, vsi_nn_tensor_t *tensor);

    //post
    vsi_status vnn_PostProcessYolov7TUint8(vsi_nn_graph_t *graph);
  
    //pre
    vnn_input_meta_t input_meta_tab[INPUT_META_NUM];
    std::unique_ptr<uint8_t[]> u2d;
    std::unique_ptr<std::vector<std::vector<float>>> _out_tensor;
    std::vector<uint8_t> in_data;
};
