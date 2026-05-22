CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -Iinclude -Ilibs
LDFLAGS = -pthread

TARGET = sysmon
SRCS = src/main.cpp src/WebServer.cpp src/CpuMonitor.cpp src/MemoryMonitor.cpp src/ProcessList.cpp src/Utils.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
