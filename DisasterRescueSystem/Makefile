CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = main.cpp rescue_queue.cpp resources.cpp locations.cpp history.cpp
TARGET = disaster_rescue.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	del /f $(TARGET) 2>nul || rm -f $(TARGET)
