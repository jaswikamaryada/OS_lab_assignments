#include "libppm.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace chrono;

// ---------- Stage 1 ----------
void S1_smoothen(image_t* input, image_t* smooth) {
    for (int i = 0; i < input->height; i++) {
        for (int j = 0; j < input->width; j++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int row = i - 1; row <= i + 1; row++) {
                for (int col = j - 1; col <= j + 1; col++) {
                    if (row >= 0 && row < input->height && col >= 0 && col < input->width) {
                        r += input->image_pixels[row][col][0];
                        g += input->image_pixels[row][col][1];
                        b += input->image_pixels[row][col][2];
                        count++;
                    }
                }
            }
            smooth->image_pixels[i][j][0] = r / count;
            smooth->image_pixels[i][j][1] = g / count;
            smooth->image_pixels[i][j][2] = b / count;
        }
    }
}

// ---------- Stage 2 ----------
void S2_find_details(image_t* input, image_t* smooth, image_t* details) {
    for (int i = 0; i < input->height; i++) {
        for (int j = 0; j < input->width; j++) {
            for (int c = 0; c < 3; c++) {
                int diff = (int)input->image_pixels[i][j][c] - (int)smooth->image_pixels[i][j][c];
                diff = std::clamp(diff, 0, 255);
                details->image_pixels[i][j][c] = (uint8_t)diff;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: ./part3_1_client.out <server_IP> <port> <input.ppm>\n";
        return 1;
    }

    const char* server_ip = argv[1];
    int port = stoi(argv[2]);
    const char* input_file = argv[3];

    image_t* input = read_ppm_file((char*)input_file);
    image_t* smooth = allocate_image(input->width, input->height);
    image_t* details = allocate_image(input->width, input->height);
    image_t* output = allocate_image(input->width, input->height);

    auto start_total = high_resolution_clock::now();

    S1_smoothen(input, smooth);
    S2_find_details(input, smooth, details);

    // Simple sharpen (S3) before sending
    for (int i = 0; i < input->height; i++) {
        for (int j = 0; j < input->width; j++) {
            for (int c = 0; c < 3; c++) {
                int val = input->image_pixels[i][j][c] + details->image_pixels[i][j][c];
                val = std::clamp(val, 0, 255);
                output->image_pixels[i][j][c] = (uint8_t)val;
            }
        }
    }

    auto end_total = high_resolution_clock::now();
    cout << "Local S1 + S2 + S3 done in "
         << duration_cast<milliseconds>(end_total - start_total).count()
         << " ms\n";

    // Connect to server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }
    cout << "Connected to server.\n";

    // Send width & height first
    int width = input->width;
    int height = input->height;
    write(sock, &width, sizeof(width));
    write(sock, &height, sizeof(height));

    // Send image data
    int total_pixels = width * height * 3;
    vector<uint8_t> buffer(total_pixels);
    int idx = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int c = 0; c < 3; c++) {
                buffer[idx++] = output->image_pixels[i][j][c];
            }
        }
    }

    write(sock, buffer.data(), total_pixels);
    cout << "Sent sharpened image data to server.\n";

    close(sock);
    return 0;
}