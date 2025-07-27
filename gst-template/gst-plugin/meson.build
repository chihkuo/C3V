plugin_c_args = ['-DHAVE_CONFIG_H']
plugin_cpp_args = ['-std=c++17']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-template-plugin')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer template Plug-ins')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')
configure_file(output : 'config.h', configuration : cdata)


gstvideo_dep = dependency('gstreamer-video-1.0',
    fallback: ['gst-plugins-base', 'video_dep'])
    
opencv_dep = dependency('opencv4', required: true)

openmp_dep = dependency('openmp', required: true)


# The videoprocess  Plugin
libdir_vsimulator = '/home/chih/source_code/VeriSilicon/VivanteIDE5.8.2/cmdtools/vsimulator/lib'
libdir_bootsort = '/home/chih/source_code/c3v_17/gst-template/gst-plugin/src/libs/botsort/build'

third_party_libs_inc = [
   'src/libs/cJSON',
   'src/libs/botsort/include',
 ]   

nn_inc = [
  'src/nn',
  'src/nn/include',
  'src/nn/include/ovxlib',
  'src/nn/include/ovxlib/utils',
  ]
  
third_party_libs_sources = [
   'src/libs/cJSON/cJSON.c',
 ]   
 
c_sources = [
  'src/plugin.c',
  'src/video-meta.c',
  'src/gstgetseidata.c',
  'src/gstsetseidata.c',
  'src/gsttssei.c',
  'src/gstseiparse.c',
  'src/gstalgoprocess.c',
  ]
  
cpp_sources = [
  'src/nn/nn_api.cpp',
  'src/nn/vnn_manager.cpp',
  'src/nn/yolo.cpp',
  'src/nn/opencv_utility.cpp',
  'src/nn/utility.cpp',
  'src/nn/algo_manager.cpp',
  'src/nn/yolo_info.cpp',
  'src/nn/tracker_algo.cpp',
  'src/nn/WorkerThread.cpp'
]

  
gstvideoprocess_sources = c_sources + cpp_sources + third_party_libs_sources

gstvideoprocessexample = library('gstvideoprocess',
  gstvideoprocess_sources,
  c_args: plugin_c_args,
  cpp_args: plugin_cpp_args,
  dependencies : [gst_dep, gstbase_dep, opencv_dep, openmp_dep],
  include_directories: [nn_inc, third_party_libs_inc],
  link_args: ['-lm', '-L' + libdir_vsimulator, '-lovxlib', '-L' + libdir_bootsort, '-lbotsort'],
  install : true,
  install_dir : plugins_install_dir,
)
