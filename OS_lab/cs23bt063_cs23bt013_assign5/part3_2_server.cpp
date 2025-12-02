#include "libppm.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstring>
using namespace std;
using namespace chrono;


void S2_find_details(image_t* input,image_t* smooth,image_t* details) {
    for (int i=0;i<input->height;i++) {
        for (int j=0;j<input->width;j++) {
            for (int c=0;c<3;c++) {
                int diff=(int)input->image_pixels[i][j][c]-(int)smooth->image_pixels[i][j][c];
                diff=std::clamp(diff,0,255);
                details->image_pixels[i][j][c]=diff;
            }
        }
    }
}


void S3_sharpen(image_t* input,image_t* details,image_t* output) {
    for (int i=0;i<input->height;i++) {
        for (int j=0;j<input->width;j++) {
            for (int c=0;c<3;c++) {
                int val=(int)input->image_pixels[i][j][c]+(int)details->image_pixels[i][j][c];
                val=std::clamp(val,0,255);
                output->image_pixels[i][j][c]=(uint8_t)val;
            }
        }
    }
}


int main(int argc,char* argv[]) {
    if (argc<4){
        cerr<<"Usage: ./part3_2_server.out <input.ppm> <output.ppm> <port>\n";
        return 1;
    }

    const char* input_file=argv[1];
    const char* output_file=argv[2];
    int port=stoi(argv[3]);

    image_t* input=read_ppm_file((char*)input_file);

    int server_sock=socket(AF_INET,SOCK_STREAM,0);
    if (server_sock<0) {
        perror("socket");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(port);

    if (bind(server_sock,(struct sockaddr*)&address,sizeof(address))<0){
        perror("bind");
        return 1;
    }

    listen(server_sock,1);
    cout<<"Waiting for the client on port"<<port<< "\n";

    int addrlen=sizeof(address);
    int connection_socket=accept(server_sock,(struct sockaddr*)&address,(socklen_t*)&addrlen);
    if (connection_socket<0){
        perror("accept");
        return 1;
    }
    cout << "Connected to client:"<<inet_ntoa(address.sin_addr) << endl;

    int height=0,width=0;
    read(connection_socket,&width,sizeof(width));
    read(connection_socket,&height,sizeof(height));
    cout << "Received smooth image size: "<<width<<"x"<<height<<endl;


    image_t* smooth=allocate_image(width,height);

    int total_pixels=width*height*3;
    vector<uint8_t>buffer(total_pixels);
    read(connection_socket,buffer.data(),total_pixels);

    int idx=0;
    for (int i=0;i<height;i++) {
        for (int j=0;j<width;j++) {
            for (int c=0;c<3;c++) {
                smooth->image_pixels[i][j][c]=buffer[idx++];
            }
        }
    }

    cout<<"Received smoothened image from client.\n";

    
    image_t* details=allocate_image(width,height);
    image_t* output=allocate_image(width,height);

    auto start = high_resolution_clock::now();
    S2_find_details(input,smooth,details);
    S3_sharpen(input,details,output);
    auto end=high_resolution_clock::now();

    cout <<"Overally S2+S3 completed in"
         <<duration_cast<milliseconds>(end-start).count()<<" ms\n";

    write_ppm_file((char*)output_file,output);
    cout<<"Output written to the file "<<output_file<<endl;

    close(connection_socket);
    close(server_sock);
    return 0;
}