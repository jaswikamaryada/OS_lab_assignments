# ===== Compiler and flags =====
CXX = g++
CXXFLAGS = -std=c++17 -O2 -pthread

# ===== Directories =====
IMG_DIR = ./images

# ===== Default Target =====
all: part1 part2A part2B part2C part3_1 part3_2

# ===== Part I (Sequential) =====
part1: image_sharpener_part1.cpp libppm.cpp libppm.h
	@echo "compiling sequentially"
	$(CXX) $(CXXFLAGS) image_sharpener_part1.cpp libppm.cpp -o part1.out
	@echo "succesfully built"

run_part1:
	@read -p "Enter image number (1-7): " IMG; \
	OUT=$${IMG}_output_part1.ppm; \
	echo "Running sequential version on $$IMG.ppm ..."; \
	./part1.out $(IMG_DIR)/$$IMG.ppm $(IMG_DIR)/$$OUT; \
	echo "Output saved as $(IMG_DIR)/$$OUT"

# ===== Part II.A (Parallel - std::thread with atomics) =====
part2A: image_sharpener_part2A.cpp libppm.cpp libppm.h
	@echo "Compiling Parallel (3-thread) Version..."
	$(CXX) $(CXXFLAGS) image_sharpener_part2A.cpp libppm.cpp -o part2_A.out
	@echo "succesfully built"

run_part2A:
	@read -p "Enter image number (1-7): " IMG; \
	OUT=$${IMG}_output_part2A.ppm; \
	echo "Running parallel version on $$IMG.ppm ..."; \
	./part2_A.out $(IMG_DIR)/$$IMG.ppm $(IMG_DIR)/$$OUT; \
	echo "Output saved as $(IMG_DIR)/$$OUT"

# ===== Part II.B (Multi-process - Pipes) =====
part2B: image_sharpener_part2B.cpp libppm.cpp libppm.h
	@echo "Compiling Multi-process (Pipes) Version..."
	$(CXX) $(CXXFLAGS) image_sharpener_part2B.cpp libppm.cpp -o part2B.out
	@echo "successfully built"

run_part2B:
	@read -p "Enter image number (1-7): " IMG; \
	OUT=$${IMG}_output_part2B.ppm; \
	echo "Running multi-process version on $$IMG.ppm ..."; \
	./part2B.out $(IMG_DIR)/$$IMG.ppm $(IMG_DIR)/$$OUT; \
	echo "Output saved as $(IMG_DIR)/$$OUT"

# ===== Part II.C (Multi-process - Shared Memory + Semaphores) =====
part2C: image_sharpener_part2C.cpp libppm.cpp libppm.h
	@echo "Compiling Multi-process (Shared Memory + Semaphores) Version..."
	$(CXX) $(CXXFLAGS) image_sharpener_part2C.cpp libppm.cpp -o part2C.out
	@echo "succesfully built"

run_part2C:
	@read -p "Enter image number (1-7): " IMG; \
	OUT=$${IMG}_output_part2C.ppm; \
	echo "Running shared-memory version on $$IMG.ppm ..."; \
	./part2C.out $(IMG_DIR)/$$IMG.ppm $(IMG_DIR)/$$OUT; \
	echo "Output saved as $(IMG_DIR)/$$OUT"

# ===== Part III.1 (Sockets - S1+S2 on client, S3 on server) =====
part3_1: part3_1_server.cpp part3_1_client.cpp libppm.cpp libppm.h
	@echo "Compiling Socket-based Version (part3-1)...."
	$(CXX) $(CXXFLAGS) part3_1_server.cpp libppm.cpp -o part3_1_server.out
	$(CXX) $(CXXFLAGS) part3_1_client.cpp libppm.cpp -o part3_1_client.out
	@echo "succesfully built, Use make run_part3_1_server and make run_part3_1_client"

run_part3_1_server:
	@read -p "Enter name of the file to be kept for the output sharpened image(e.g. out3_1.ppm): " OUT; \
	read -p "Enter port number: " PORT; \
	echo "Starting server on port $$PORT ..."; \
	./part3_1_server.out $(IMG_DIR)/$$OUT $$PORT

run_part3_1_client:
	@read -p "Enter server IP (default 127.0.0.1): " IP; \
	IP=$${IP:-127.0.0.1}; \
	read -p "Enter port number: " PORT; \
	read -p "Enter image number (1-7): " IMG; \
	echo "Running client on $$IP:$$PORT with image $$IMG.ppm ..."; \
	./part3_1_client.out $$IP $$PORT $(IMG_DIR)/$$IMG.ppm

# ===== Part III.2 (Sockets - S1 on client, S2+S3 on server) =====
part3_2: part3_2_server.cpp part3_2_client.cpp libppm.cpp libppm.h
	@echo "Compiling Socket-based Version (Part3-2)..."
	$(CXX) $(CXXFLAGS) part3_2_server.cpp libppm.cpp -o part3_2_server.out
	$(CXX) $(CXXFLAGS) part3_2_client.cpp libppm.cpp -o part3_2_client.out
	@echo "succesfully built, Use make run_part3_2_server and make run_part3_2_client"

run_part3_2_server:
	@read -p "Enter input image number (1-7): " IMG; \
	read -p "Enter name of the file to be kept for the output sharpened image (e.g., out3_2.ppm): " OUT; \
	read -p "Enter port number: " PORT; \
	echo "Starting server for image $$IMG on port $$PORT ..."; \
	./part3_2_server.out $(IMG_DIR)/$$IMG.ppm $(IMG_DIR)/$$OUT $$PORT

run_part3_2_client:
	@read -p "Enter server IP (default 127.0.0.1): " IP; \
	IP=$${IP:-127.0.0.1}; \
	read -p "Enter port number: " PORT; \
	read -p "Enter input image number (1-7): " IMG; \
	echo "Running client on $$IP:$$PORT with image $$IMG.ppm ..."; \
	./part3_2_client.out $$IP $$PORT $(IMG_DIR)/$$IMG.ppm

# ===== Clean =====
clean:
	rm -f *.out $(IMG_DIR)/*output*.ppm $(IMG_DIR)/out3_*.ppm
	@echo "Cleaned build files and output images."
