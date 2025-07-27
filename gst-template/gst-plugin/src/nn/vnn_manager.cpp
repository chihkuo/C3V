#include "vnn_manager.h"

#include <chrono>
#include <omp.h>
#include <cmath>
#include <vector>

#define INPUT_W 640
#define INPUT_H 640
#define INPUT_C 3
#define INPUT_N (INPUT_W * INPUT_H)
#define MAX_OUTPUT_TENSORS_NUM 3


VNN_Manager::VNN_Manager(VNN_Mode mode)
: m_mode(mode)
{
    vnn_CreateNeuralNetwork();
}

VNN_Manager::~VNN_Manager()
{
    vnn_ReleaseNeuralNetwork();
}

vsi_status VNN_Manager::vnn_CreateNeuralNetwork()
{
    vsi_status status = VSI_FAILURE;

    if (m_mode == evm_YOLOv7_RGB)
    {
        std::cout << "vnn_CreateNeuralNetwork mode: YOLOv7_RGB" << std::endl;
	    const char *data_file_name = "/home/sunplus/network_binary_v7tiny_rgb.nb";
	    m_graph = vnn_CreateYolov7TinyUint8(data_file_name, NULL, NULL, 0, NULL, 0);
    }
	else if (m_mode == evm_YOLOv7_THERMAL)
    {
        std::cout << "vnn_CreateNeuralNetwork mode: YOLOv7_THERMAL" << std::endl;
	    const char *data_file_name = "/home/sunplus/network_binary_v7_thermal.nb";
	    m_graph = vnn_CreateYolov7TUint8(data_file_name, NULL, NULL, 0, NULL, 0);   
    }

    m_yolo.setModel(m_mode);
    status = vsi_nn_VerifyGraph(m_graph);

    printf("VerifyGraph status: %d\n", status);
  
    vnn_PreTableInit(m_graph);
    printf("vnn_PreTableInit status: %d\n", status);

    _load_input_meta();
    printf("======= _load_input_meta. =======\n");
    in_data.resize(INPUT_N * INPUT_C);
    return status;
}

vsi_status VNN_Manager::vnn_ReleaseNeuralNetwork()
{
    vnn_ReleaseYolov7TUint8(m_graph, TRUE);
    return VSI_SUCCESS;
}

vsi_status VNN_Manager::vnn_PreProcessNeuralNetwork(cv::Mat &frame)
{
    m_input_size = frame.size();
    return vnn_PreProcessYolov7TUint8(m_graph, frame);
}

vsi_status VNN_Manager::vnn_ProcessGraph()
{
    return vsi_nn_RunGraph(m_graph);
}

vsi_status VNN_Manager::vnn_GetResult(std::vector<Output> &output)
{
    std::vector<cv::Mat> outputTensor;
    vnn_GetOutputTensor(outputTensor);
    m_yolo.Detect(m_input_size, outputTensor, output);
    return VSI_SUCCESS;
}

vsi_status VNN_Manager::vnn_GetOutputTensor(std::vector<cv::Mat> &outputTensor)
{
    std::vector<cv::Mat> vecTensor(3);

    #pragma omp parallel for
    for (int idx = 0; idx < 3; idx++)
    {
        vsi_nn_tensor_t *tensor = vsi_nn_GetTensor(m_graph, m_graph->output.tensors[idx]);
        vsi_size_t i, sz, stride;
        uint8_t *tensor_data = NULL;
        sz = 1;
        for (i = 0; i < tensor->attr.dim_num; i++)
        {
            sz *= tensor->attr.size[i];
        }

        stride = (vsi_size_t)vsi_nn_TypeGetBytes(tensor->attr.dtype.vx_type);
        if (stride == 0)
        {
            stride = 1;
        }

        tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(m_graph, tensor);
        vecTensor[idx] = _dtype_to_float32(tensor_data, tensor, idx);

        if (tensor_data)
            vsi_nn_Free(tensor_data);
    }

    outputTensor.swap(vecTensor);
    return VSI_SUCCESS;
}

void VNN_Manager::vnn_ReleaseYolov7TinyUint8(vsi_nn_graph_t *graph, vsi_bool release_ctx)
{
    vsi_nn_context_t ctx;
    if( NULL != graph )
    {
        ctx = graph->ctx;
        vsi_nn_ReleaseGraph( &graph );

        /*-----------------------------------------
        Unregister client ops
        -----------------------------------------*/
        

        if( release_ctx )
        {
            vsi_nn_ReleaseContext( &ctx );
        }
    }    
}

vsi_nn_graph_t *VNN_Manager::vnn_CreateYolov7TinyUint8(const char *data_file_name, vsi_nn_context_t in_ctx, const vsi_nn_preprocess_map_element_t *pre_process_map, uint32_t pre_process_map_count, const vsi_nn_postprocess_map_element_t *post_process_map, uint32_t post_process_map_count)
{
    uint32_t                _infinity = VSI_NN_FLOAT32_INF;
    vsi_status              status;
    vsi_bool                release_ctx;
    vsi_nn_context_t        ctx;
    vsi_nn_graph_t *        graph;
    vsi_nn_node_t *         node[NET_NODE_NUM];
    vsi_nn_tensor_id_t      norm_tensor[NET_NORM_TENSOR_NUM];
    
    vsi_nn_tensor_attr_t    attr;
    FILE *                  fp;
    uint8_t *               data;
    uint32_t                i = 0;
    char *                  use_img_process_s;
    char *                  use_from_handle = NULL;
    int32_t                 enable_pre_post_process = 0;
    int32_t                 enable_from_handle = 0;
    vsi_bool                sort = FALSE;
    vsi_bool                inference_with_nbg = FALSE;
    char*                   pos = NULL;





    (void)(_infinity);
    ctx = NULL;
    graph = NULL;
    status = VSI_FAILURE;
    memset( &attr, 0, sizeof( attr ) );
    memset( &node, 0, sizeof( vsi_nn_node_t * ) * NET_NODE_NUM );

    fp = fopen( (char*)data_file_name, "rb" );
    if( NULL == fp )
    {
        VSILOGE( "Open file %s failed.", data_file_name );
        goto error;
    }

    pos = strstr((char*)data_file_name, ".nb");
    if( pos && strcmp(pos, ".nb") == 0 )
    {
        inference_with_nbg = TRUE;
    }

    if( NULL == in_ctx )
    {
        ctx = vsi_nn_CreateContext();
    }
    else
    {
        ctx = in_ctx;
    }

    use_img_process_s = getenv( "VSI_USE_IMAGE_PROCESS" );
    if( use_img_process_s )
    {
        enable_pre_post_process = atoi(use_img_process_s);
    }
    use_from_handle = getenv( "VSI_USE_FROM_HANDLE" );
    if ( use_from_handle )
    {
        enable_from_handle = atoi(use_from_handle);
    }

    graph = vsi_nn_CreateGraph( ctx, NET_TOTAL_TENSOR_NUM, NET_NODE_NUM );
    if( NULL == graph )
    {
        VSILOGE( "Create graph fail." );
        goto error;
    }
    vsi_nn_SetGraphVersion( graph, VNN_VERSION_MAJOR, VNN_VERSION_MINOR, VNN_VERSION_PATCH );
    vsi_nn_SetGraphInputs( graph, NULL, 1 );
    vsi_nn_SetGraphOutputs( graph, NULL, 3 );

/*-----------------------------------------
  Register client ops
 -----------------------------------------*/


/*-----------------------------------------
  Node definitions
 -----------------------------------------*/
    if( !inference_with_nbg )
    {

    /*-----------------------------------------
      lid       - nbg_0
      var       - node[0]
      name      - nbg
      operation - nbg
      input     - [640, 640, 3, 1]
      output    - [85, 80, 80, 3, 1]
                  [85, 40, 40, 3, 1]
                  [85, 20, 20, 3, 1]
    -----------------------------------------*/
    NEW_VXNODE(node[0], VSI_NN_OP_NBG, 1, 3, 0);
    node[0]->nn_param.nbg.type = VSI_NN_NBG_FILE;
    node[0]->nn_param.nbg.url = data_file_name;

    }
    else
    {
    NEW_VXNODE(node[0], VSI_NN_OP_NBG, 1, 3, 0);
    node[0]->nn_param.nbg.type = VSI_NN_NBG_FILE;
    node[0]->nn_param.nbg.url = data_file_name;

    }

/*-----------------------------------------
  Tensor initialize
 -----------------------------------------*/
    attr.dtype.fmt = VSI_NN_DIM_FMT_NCHW;
    /* @attach_Transpose_/model.77/Transpose/out0_0:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 85;
    //attr.size[0] = 16;
    attr.size[1] = 80;
    attr.size[2] = 80;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.11858520656824112;
    //attr.dtype.scale = 0.17932529747486115;
    attr.dtype.zero_point = 182;
    //attr.dtype.zero_point = 195;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[0], attr, VSI_NN_TYPE_UINT8);

    /* @attach_Transpose_/model.77/Transpose_1/out0_1:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 85;
    //attr.size[0] = 16;
    attr.size[1] = 40;
    attr.size[2] = 40;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.12396541982889175;
    //attr.dtype.scale = 0.20005743205547333;
    attr.dtype.zero_point = 171;                                                                
    //attr.dtype.zero_point = 194;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[1], attr, VSI_NN_TYPE_UINT8);

    /* @attach_Transpose_/model.77/Transpose_2/out0_2:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 85;
    //attr.size[0] = 16;
    attr.size[1] = 20;
    attr.size[2] = 20;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.09778282046318054;
    //attr.dtype.scale = 0.21836230158805847;
    attr.dtype.zero_point = 179;
    //attr.dtype.zero_point = 208;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[2], attr, VSI_NN_TYPE_UINT8);

    /* @images_144:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 640;
    attr.size[1] = 640;
    attr.size[2] = 3;
    attr.size[3] = 1;
    attr.dim_num = 4;
    attr.dtype.scale = 0.003921568859368563;
    //attr.dtype.scale = 0.003913651220500469;
    attr.dtype.zero_point = 0;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[3], attr, VSI_NN_TYPE_UINT8);



    if( !inference_with_nbg )
    {




/*-----------------------------------------
  Connection initialize
 -----------------------------------------*/
    node[0]->input.tensors[0] = norm_tensor[3];
    node[0]->output.tensors[0] = norm_tensor[0];
    node[0]->output.tensors[1] = norm_tensor[1];
    node[0]->output.tensors[2] = norm_tensor[2];

    /* nbg_0 */


    }
    else
    {
    node[0]->input.tensors[0] = norm_tensor[3];
    node[0]->output.tensors[0] = norm_tensor[0];
    node[0]->output.tensors[1] = norm_tensor[1];
    node[0]->output.tensors[2] = norm_tensor[2];

    /* nbg_0 */


    }
    graph->input.tensors[0] = norm_tensor[3];
    graph->output.tensors[0] = norm_tensor[0];
    graph->output.tensors[1] = norm_tensor[1];
    graph->output.tensors[2] = norm_tensor[2];


    if( enable_pre_post_process )
    {
        sort = TRUE;
        printf("pre_count: %d, post_count: %d\n", pre_process_map_count, post_process_map_count);
        if( pre_process_map_count > 0 )
        {
            for( i = 0; i < pre_process_map_count; i++ )
            {
                status = vsi_nn_AddGraphPreProcess(graph, pre_process_map[i].graph_input_idx,
                                                   pre_process_map[i].preprocesses,
                                                   pre_process_map[i].preprocess_count);

		printf("vsi_nn_AddGraphPreProcess, status: %d\n", status);
                TEST_CHECK_STATUS( status, error );
            }
        }

        if( post_process_map_count > 0 )
        {
            for( i = 0; i < post_process_map_count; i++ )
            {
                 status = vsi_nn_AddGraphPostProcess(graph, post_process_map[i].graph_output_idx,
                                                     post_process_map[i].postprocesses,
                                                     post_process_map[i].postprocess_count);
                 TEST_CHECK_STATUS( status, error );
            }
        }
    }

    status = vsi_nn_SetupGraph( graph, sort );
    printf("vsi_nn_SetupGraph, status: %d\n", status);
    TEST_CHECK_STATUS( status, error );


    if( VSI_FAILURE == status )
    {
        goto error;
    }

    fclose( fp );

    return graph;

error:
    if( NULL != fp )
    {
        fclose( fp );
    }

    release_ctx = ( NULL == in_ctx );
    vsi_nn_DumpGraphToJson( graph );
    vnn_ReleaseYolov7TinyUint8( graph, release_ctx );

    return NULL;
}

void VNN_Manager::vnn_ReleaseYolov7TUint8(vsi_nn_graph_t *graph, vsi_bool release_ctx)
{
    vsi_nn_context_t ctx;
    if( NULL != graph )
    {
        ctx = graph->ctx;
        vsi_nn_ReleaseGraph( &graph );

        /*-----------------------------------------
        Unregister client ops
        -----------------------------------------*/
        

        if( release_ctx )
        {
            vsi_nn_ReleaseContext( &ctx );
        }
    }       
}

vsi_nn_graph_t *VNN_Manager::vnn_CreateYolov7TUint8(const char *data_file_name, vsi_nn_context_t in_ctx, const vsi_nn_preprocess_map_element_t *pre_process_map, uint32_t pre_process_map_count, const vsi_nn_postprocess_map_element_t *post_process_map, uint32_t post_process_map_count)
{
    uint32_t                _infinity = VSI_NN_FLOAT32_INF;
    vsi_status              status;
    vsi_bool                release_ctx;
    vsi_nn_context_t        ctx;
    vsi_nn_graph_t *        graph;
    vsi_nn_node_t *         node[NET_NODE_NUM];
    vsi_nn_tensor_id_t      norm_tensor[NET_NORM_TENSOR_NUM];
    
    vsi_nn_tensor_attr_t    attr;
    FILE *                  fp;
    uint8_t *               data;
    uint32_t                i = 0;
    char *                  use_img_process_s;
    char *                  use_from_handle = NULL;
    int32_t                 enable_pre_post_process = 0;
    int32_t                 enable_from_handle = 0;
    vsi_bool                sort = FALSE;
    vsi_bool                inference_with_nbg = FALSE;
    char*                   pos = NULL;





    (void)(_infinity);
    ctx = NULL;
    graph = NULL;
    status = VSI_FAILURE;
    memset( &attr, 0, sizeof( attr ) );
    memset( &node, 0, sizeof( vsi_nn_node_t * ) * NET_NODE_NUM );

    fp = fopen( (char*)data_file_name, "rb" );
    if( NULL == fp )
    {
        VSILOGE( "Open file %s failed.", data_file_name );
        goto error;
    }

    pos = strstr((char*)data_file_name, ".nb");
    if( pos && strcmp(pos, ".nb") == 0 )
    {
        inference_with_nbg = TRUE;
    }

    if( NULL == in_ctx )
    {
        ctx = vsi_nn_CreateContext();
    }
    else
    {
        ctx = in_ctx;
    }

    use_img_process_s = getenv( "VSI_USE_IMAGE_PROCESS" );
    if( use_img_process_s )
    {
        enable_pre_post_process = atoi(use_img_process_s);
    }
    use_from_handle = getenv( "VSI_USE_FROM_HANDLE" );
    if ( use_from_handle )
    {
        enable_from_handle = atoi(use_from_handle);
    }

    graph = vsi_nn_CreateGraph( ctx, NET_TOTAL_TENSOR_NUM, NET_NODE_NUM );
    if( NULL == graph )
    {
        VSILOGE( "Create graph fail." );
        goto error;
    }
    vsi_nn_SetGraphVersion( graph, VNN_VERSION_MAJOR, VNN_VERSION_MINOR, VNN_VERSION_PATCH );
    vsi_nn_SetGraphInputs( graph, NULL, 1 );
    vsi_nn_SetGraphOutputs( graph, NULL, 3 );

/*-----------------------------------------
  Register client ops
 -----------------------------------------*/


/*-----------------------------------------
  Node definitions
 -----------------------------------------*/
    if( !inference_with_nbg )
    {

    /*-----------------------------------------
      lid       - nbg_0
      var       - node[0]
      name      - nbg
      operation - nbg
      input     - [640, 640, 3, 1]
      output    - [16, 80, 80, 3, 1]
                  [16, 40, 40, 3, 1]
                  [16, 20, 20, 3, 1]
    -----------------------------------------*/
    NEW_VXNODE(node[0], VSI_NN_OP_NBG, 1, 3, 0);
    node[0]->nn_param.nbg.type = VSI_NN_NBG_FILE;
    node[0]->nn_param.nbg.url = data_file_name;

    }
    else
    {
    NEW_VXNODE(node[0], VSI_NN_OP_NBG, 1, 3, 0);
    node[0]->nn_param.nbg.type = VSI_NN_NBG_FILE;
    node[0]->nn_param.nbg.url = data_file_name;

    }

/*-----------------------------------------
  Tensor initialize
 -----------------------------------------*/
    attr.dtype.fmt = VSI_NN_DIM_FMT_NCHW;
    /* @attach_Transpose_/model.105/Transpose/out0_0:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 16;
    attr.size[1] = 80;
    attr.size[2] = 80;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.1994536817073822;
    attr.dtype.zero_point = 204;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[0], attr, VSI_NN_TYPE_UINT8);

    /* @attach_Transpose_/model.105/Transpose_1/out0_1:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 16;
    attr.size[1] = 40;
    attr.size[2] = 40;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.1625015288591385;
    attr.dtype.zero_point = 183;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[1], attr, VSI_NN_TYPE_UINT8);

    /* @attach_Transpose_/model.105/Transpose_2/out0_2:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 16;
    attr.size[1] = 20;
    attr.size[2] = 20;
    attr.size[3] = 3;
    attr.size[4] = 1;
    attr.dim_num = 5;
    attr.dtype.scale = 0.1777673363685608;
    attr.dtype.zero_point = 186;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[2], attr, VSI_NN_TYPE_UINT8);

    /* @images_306:out0 */
    memset( &attr, 0, sizeof( attr ) );
    attr.size[0] = 640;
    attr.size[1] = 640;
    attr.size[2] = 3;
    attr.size[3] = 1;
    attr.dim_num = 4;
    attr.dtype.scale = 0.003921568859368563;
    attr.dtype.zero_point = 0;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
    NEW_NORM_TENSOR(norm_tensor[3], attr, VSI_NN_TYPE_UINT8);



    if( !inference_with_nbg )
    {




/*-----------------------------------------
  Connection initialize
 -----------------------------------------*/
    node[0]->input.tensors[0] = norm_tensor[3];
    node[0]->output.tensors[0] = norm_tensor[0];
    node[0]->output.tensors[1] = norm_tensor[1];
    node[0]->output.tensors[2] = norm_tensor[2];

    /* nbg_0 */


    }
    else
    {
    node[0]->input.tensors[0] = norm_tensor[3];
    node[0]->output.tensors[0] = norm_tensor[0];
    node[0]->output.tensors[1] = norm_tensor[1];
    node[0]->output.tensors[2] = norm_tensor[2];

    /* nbg_0 */


    }
    graph->input.tensors[0] = norm_tensor[3];
    graph->output.tensors[0] = norm_tensor[0];
    graph->output.tensors[1] = norm_tensor[1];
    graph->output.tensors[2] = norm_tensor[2];


    if( enable_pre_post_process )
    {
        sort = TRUE;
        printf("pre_count: %d, post_count: %d\n", pre_process_map_count, post_process_map_count);
        if( pre_process_map_count > 0 )
        {
            for( i = 0; i < pre_process_map_count; i++ )
            {
                status = vsi_nn_AddGraphPreProcess(graph, pre_process_map[i].graph_input_idx,
                                                   pre_process_map[i].preprocesses,
                                                   pre_process_map[i].preprocess_count);

		printf("vsi_nn_AddGraphPreProcess, status: %d\n", status);
                TEST_CHECK_STATUS( status, error );
            }
        }

        if( post_process_map_count > 0 )
        {
            for( i = 0; i < post_process_map_count; i++ )
            {
                 status = vsi_nn_AddGraphPostProcess(graph, post_process_map[i].graph_output_idx,
                                                     post_process_map[i].postprocesses,
                                                     post_process_map[i].postprocess_count);
                 TEST_CHECK_STATUS( status, error );
            }
        }
    }

    status = vsi_nn_SetupGraph( graph, sort );
    printf("vsi_nn_SetupGraph, status: %d\n", status);
    TEST_CHECK_STATUS( status, error );


    if( VSI_FAILURE == status )
    {
        goto error;
    }

    fclose( fp );

    return graph;

error:
    if( NULL != fp )
    {
        fclose( fp );
    }

    release_ctx = ( NULL == in_ctx );
    vsi_nn_DumpGraphToJson( graph );
    vnn_ReleaseYolov7TUint8( graph, release_ctx );

    return NULL;
}

vsi_status VNN_Manager::vnn_PreProcessYolov7TUint8(vsi_nn_graph_t *graph, cv::Mat &frame)
{
    return _handle_frame_input(graph, frame);
}

void VNN_Manager::_load_input_meta()
{
    uint32_t i;
    for (i = 0; i < INPUT_META_NUM; i++)
    {
        memset(&input_meta_tab[i].image.preprocess,
               VNN_PREPRO_NONE, sizeof(int32_t) * VNN_PREPRO_NUM);
    }
    /* lid: images_306 */
    input_meta_tab[0].image.preprocess[0] = VNN_PREPRO_REORDER;
    input_meta_tab[0].image.preprocess[1] = VNN_PREPRO_MEAN;
    input_meta_tab[0].image.preprocess[2] = VNN_PREPRO_SCALE;
    input_meta_tab[0].image.reorder[0] = 2;
    input_meta_tab[0].image.reorder[1] = 1;
    input_meta_tab[0].image.reorder[2] = 0;
    input_meta_tab[0].image.mean[0] = 0;
    input_meta_tab[0].image.mean[1] = 0;
    input_meta_tab[0].image.mean[2] = 0;
    input_meta_tab[0].image.scale[0] = 0.003921569;
    input_meta_tab[0].image.scale[1] = 0.003921569;
    input_meta_tab[0].image.scale[2] = 0.003921569;    
}


vsi_status VNN_Manager::_handle_frame_input(vsi_nn_graph_t * graph, cv::Mat & frame)
{
    vsi_status status;
    vsi_nn_tensor_t *tensor;
    vnn_input_meta_t meta;
    status = VSI_FAILURE;
    tensor = NULL;

    memset(&meta, 0, sizeof(vnn_input_meta_t));
    tensor = vsi_nn_GetTensor(graph, graph->input.tensors[0]);
    meta = input_meta_tab[0];

    _get_frame_data(tensor, &meta, frame);
    /* Copy the Pre-processed data to input tensor */
    status = vsi_nn_CopyDataToTensor(graph, tensor, in_data.data());
    status = VSI_SUCCESS;
    return status;
}

uint8_t *VNN_Manager::_get_frame_data(vsi_nn_tensor_t * tensor, vnn_input_meta_t * meta, cv::Mat & frame)
{
    if (frame.empty())
    {
        std::cout << "get_frame_data null" << std::endl;
        return NULL;
    }

    cv::Mat resizedImage, rgbFrame;
    cv::resize(frame, resizedImage, cv::Size(640, 640),0, 0, cv::INTER_NEAREST);
    cv::cvtColor(resizedImage, rgbFrame, cv::COLOR_BGR2RGB);

    for (int i = 0; i < _cnt_of_array(meta->image.preprocess); i++)
    {
        switch (meta->image.preprocess[i])
        {
        case VNN_PREPRO_NONE:
            break;
        case VNN_PREPRO_REORDER:
            _data_transform(rgbFrame, meta, tensor);
            break;
        case VNN_PREPRO_MEAN:
            //_data_mean(fdata, meta, tensor);
            break;
        case VNN_PREPRO_SCALE:
            //_data_scale(fdata, meta, tensor);
            break;
        default:
            break;
        }
    }

    return nullptr;
}

float *VNN_Manager::_imageData_to_float32(uint8_t *bmpData, vsi_nn_tensor_t *tensor)
{
    float *fdata;
    vsi_size_t sz, i;

    fdata = NULL;
    sz = vsi_nn_GetElementNum(tensor);
    fdata = (float *)malloc(sz * sizeof(float));
    if (!fdata)
        return NULL;

    #pragma omp parallel for
    for (i = 0; i < sz; i++)
    {
        fdata[i] = static_cast<float>(bmpData[i]);
    }

    return fdata;
}

void VNN_Manager:: _data_transform(cv::Mat &m, vnn_input_meta_t *meta, vsi_nn_tensor_t *tensor)
{
    #pragma omp parallel for
    for (int j = 0; j < INPUT_C; j++) {
        uint8_t *src = m.data + j;
        
        uint8_t *dst = in_data.data() + INPUT_N *j; //BGR
        for (int i = 0; i < INPUT_N; i++, src += INPUT_C)
            dst[i] = u2d[*src];
    }
}

void VNN_Manager::_data_scale(float *fdata, vnn_input_meta_t *meta, vsi_nn_tensor_t *tensor)
{
    vsi_size_t s0, s1, s2;
    vsi_size_t i, j, offset;
    float *fdata_ptr;
    float val, scale;

    s0 = tensor->attr.size[0];
    s1 = tensor->attr.size[1];
    s2 = tensor->attr.size[2];

    vsi_size_t s01 = s0 * s1;

    #pragma omp parallel for
    for (i = 0; i < s2; i++)
    {
        scale = meta->image.scale[i];
        offset = s01 * i;
        fdata_ptr = fdata + offset;
        for (j = 0; j < s01; j++)
        {
            fdata_ptr[j] *= scale;
        }
    }
}

vsi_status VNN_Manager::vnn_PreTableInit(vsi_nn_graph_t *graph)
{
    vsi_nn_tensor_t *tensor, *out_tensor;
    vsi_size_t w, h, c, stride;
    tensor = vsi_nn_GetTensor(graph, graph->input.tensors[0]);
    stride = vsi_nn_TypeGetBytes(tensor->attr.dtype.vx_type);

    //input tensor
    u2d = std::make_unique<uint8_t[]>(256);
    for (int i = 0; i < 256; i++)
    {
        vsi_nn_Float32ToDtype(i / 255.0f, u2d.get() + i * stride, &tensor->attr.dtype);
    }

    //output tensor
    _out_tensor = std::make_unique<std::vector<std::vector<float>>>(3);
    for (int idx = 0; idx < 3; idx++) {
        vsi_nn_tensor_t *tensor = vsi_nn_GetTensor(graph, graph->output.tensors[idx]);
        for (int i = 0; i < 256; i++) {
            uint8_t d = (uint8_t)i;
            float fd;
	    vsi_nn_DtypeToFloat32(&d, &fd, &tensor->attr.dtype);
            (*_out_tensor)[idx].push_back(fd);
	}
    }

    return VSI_SUCCESS;
}

cv::Mat VNN_Manager::_dtype_to_float32(uint8_t * ddata, vsi_nn_tensor_t *tensor, uint8_t idx)
{
    vsi_status status;
    vsi_size_t sz, i, stride;

    sz = vsi_nn_GetElementNum(tensor);
    stride = vsi_nn_TypeGetBytes(tensor->attr.dtype.vx_type);

    cv::Mat data(sz, 1, CV_32F);
    float *row_ptr = data.ptr<float>(0);

    const auto &tensor_data = (*_out_tensor)[idx];

    //#pragma omp parallel for
    for (i = 0; i < sz; i += 4)
    {
        row_ptr[i] = tensor_data[ddata[i]];
        row_ptr[i + 1] = tensor_data[ddata[i + 1]];
        row_ptr[i + 2] = tensor_data[ddata[i + 2]];
        row_ptr[i + 3] = tensor_data[ddata[i + 3]];
    }

    return data;
}

uint8_t *VNN_Manager::_float32_to_dtype(float *fdata, vsi_nn_tensor_t *tensor)
{
    vsi_status status;
    uint8_t *data;
    vsi_size_t sz, i, stride;

    sz = vsi_nn_GetElementNum(tensor);
    stride = vsi_nn_TypeGetBytes(tensor->attr.dtype.vx_type);
    if (stride == 0)
    {
        stride = 1;
    }

    data = (uint8_t *)malloc(stride * sz * sizeof(uint8_t));
    memset(data, 0, stride * sz * sizeof(uint8_t));

    float scale = input_meta_tab[0].image.scale[0];

    #pragma omp parallel for
    for (i = 0; i < sz; i += 4)
    {
        data[i] = u2d[(uint8_t)(fdata[i] / scale)];
        data[i + 1] = u2d[(uint8_t)(fdata[i + 1] / scale)];
        data[i + 2] = u2d[(uint8_t)(fdata[i + 2] / scale)];
        data[i + 3] = u2d[(uint8_t)(fdata[i + 3] / scale)];
    }

    return data;
}

vsi_status VNN_Manager::vnn_PostProcessYolov7TUint8(vsi_nn_graph_t *graph)
{
    return vsi_status();
}
