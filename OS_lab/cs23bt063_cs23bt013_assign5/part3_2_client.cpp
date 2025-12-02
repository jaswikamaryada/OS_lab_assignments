#include "libppm.h"
#include <iostream>
#include <sys/socket.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


using namespace std;
using namespace chrono;


void S1_smoothen(image_t* input,image_t* smooth) {
    for (int i=0;i<input->height;i++) {
        for (int j=0;j<input->width;j++) {
            int r=0,g=0,b=0,count=0;
            for (int row=i-1;row<=i+1;row++) {
                for (int col=j-1;col<=j+1;col++) {
                    if (row>=0 && row<input->height && col>=0 && col<input->width) {
                        r+=input->image_pixels[row][col][0];
                        g+=input->image_pixels[row][col][1];
                        b+=input->image_pixels[row][col][2];
                        count++;
                    }
                }
            }
            smooth->image_pixels[i][j][0]=r/count;
            smooth->image_pixels[i][j][1]=g/count;
            smooth->image_pixels[i][j][2]=b/count;
        }
    }
}

int main(int argc,char* argv[]) {
    if (argc<4) {
        cerr<<"Usage: ./part3_2_client.out <server_IP> <port> <input.ppm>\n";
        return 1;
    }

    const char* server_ip_address=argv[1];
    int port_number=stoi(argv[2]);
    const char* input_file=argv[3];

    image_t* input=read_ppm_file((char*)input_file);
    image_t* smooth=allocate_image(input->width,input->height);

    auto start=high_resolution_clock::now();
    S1_smoothen(input,smooth);
    auto end=high_resolution_clock::now();

    cout <<"S1 completed in "
         <<duration_cast<milliseconds>(end-start).count()<< " ms\n";

    int sock=socket(AF_INET,SOCK_STREAM,0);
    if (sock<0){
        perror("socket");
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port_number);
    inet_pton(AF_INET,server_ip_address,&server_address.sin_addr);

    if (connect(sock,(struct sockaddr*)&server_address,sizeof(server_address))<0) {
        perror("connect");
        return 1;
    }

    cout<<"Connected to server at "<<server_ip_address<< ":"<<port_number<<endl;

    
    int width=input->width;
    int height=input->height;
    write(sock,&width,sizeof(width));
    write(sock,&height,sizeof(height));

    int total_pixels=width*height*3;
    vector<uint8_t>buffer(total_pixels);
    int idx=0;
    for (int i=0;i<height;i++) {
        for (int j=0;j<width;j++) {
            for (int c=0;c<3;c++) {
                buffer[idx++]=smooth->image_pixels[i][j][c];
            }
        }
    }

    write(sock,buffer.data(),total_pixels);
    cout<<"Sent smoothened image to server.\n";

    close(sock);
    return 0;
}