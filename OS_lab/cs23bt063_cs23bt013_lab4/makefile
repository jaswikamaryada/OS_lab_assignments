CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = simulator
SRC = simulator.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	@./$(TARGET) $(PAGE_SIZE) $(NUM_FRAMES) $(POLICY) $(ALLOC) $(TRACE)

clean:
	rm -f $(TARGET)
