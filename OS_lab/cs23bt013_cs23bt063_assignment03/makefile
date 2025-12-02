CXX = g++
CXXFLAGS = -Wall -O2


TARGET1 = part1
TARGET2 = part2

SRC1 = simulator_part1.cpp
SRC2 = simulator_part2.cpp

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CXX) $(CXXFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CXX) $(CXXFLAGS) -o $(TARGET2) $(SRC2)


run1: $(TARGET1)
	./$(TARGET1) $(ALG) $(FILE)


run2: $(TARGET2)
	./$(TARGET2) $(ALG) $(FILE)

clean:
	rm -f $(TARGET1) $(TARGET2)
