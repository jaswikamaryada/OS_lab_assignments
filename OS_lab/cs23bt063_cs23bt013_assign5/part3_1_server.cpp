#include "libppm.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./part3_1_server.out <output.ppm> <port>\n";
        return 1;
    }

    const char* output_file = argv[1];
    int port = stoi(argv[2]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    listen(server_fd, 1);
    cout << "Waiting for connection on port " << port << "...\n";

    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("accept");
        return 1;
    }
    cout << "Client connected from " << inet_ntoa(address.sin_addr) << endl;

    // Receive width and height
    int width = 0, height = 0;
    read(new_socket, &width, sizeof(width));
    read(new_socket, &height, sizeof(height));
    cout << "Received image size: " << width << "x" << height << endl;

    // Allocate output image
    image_t* output = allocate_image(width, height);

    // Receive pixel data (already sharpened)
    int total_pixels = width * height * 3;
    vector<uint8_t> buffer(total_pixels);
    read(new_socket, buffer.data(), total_pixels);

    // Fill the output image
    int idx = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int c = 0; c < 3; c++) {
                output->image_pixels[i][j][c] = buffer[idx++];
            }
        }
    }

    write_ppm_file((char*)output_file, output);
    cout << "Image written to " << output_file << endl;

    close(new_socket);
    close(server_fd);
    return 0;
}