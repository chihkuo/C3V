# 编译器和编译器选项
CXX = g++

ifeq ($(C3V_RELEASE),1)
	CXXFLAGS = -g -Wall -Wextra -std=c++17 -O3 -DNDBUG -DC3V_RELEASE \
		-I./include `pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0 dbus-1`
else
	CXXFLAGS = -g -Wall -Wextra -std=c++17 -O3 -I./include `pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0 dbus-1`
endif

# 源文件和目标文件
SOURCES = main.cpp GStreamerPipeline.cpp DBusServer.cpp AlgoDataMgr.cpp CameraAPI.cpp CameraFunctionMgr.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# 可执行文件
TARGET = main_ap

# 默认目标
all: $(TARGET)

# 链接目标文件
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ `pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0 dbus-1`

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标
clean:
	rm -f $(OBJECTS) $(TARGET)

# 伪目标
.PHONY: all clean

