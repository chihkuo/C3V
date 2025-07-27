# 指定编译器
CXX = g++
CXXFLAGS = -Wall -g `pkg-config --cflags dbus-1`
LDFLAGS = `pkg-config --libs dbus-1` -lpthread

# Nlohmann JSON 包含目录
JSON_INCLUDE = -Iinclude  # 假设 json.hpp 在 include/nlohmann/ 目录下

# 可执行文件名
TARGET = laser_dbus

# 源代码文件
SRCS = laser_dbus.cpp dbus_client.cpp

# 生成的目标
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# 编译目标
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(JSON_INCLUDE) -c $< -o $@

# 清理目标
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

