#include "libppm.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstring>

using namespace std;
using namespace chrono;

struct SharedData {
    vector<array<uint8_t,3>>smooth_buffer;
    vector<array<uint8_t,3>>detail_buffer;
    atomic<int>s1_index{0};
    atomic<int>s2_index{0};
};

void S1_smoothen(image_t* input,SharedData* shared_memory) {
    for (int i=0;i<input->height;i++) {
        for (int j=0;j<input->width;j++) {
            int idx=i*input->width+j;
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
            shared_memory->smooth_buffer[idx]={(uint8_t)(r/count),
                                       (uint8_t)(g/count),
                                       (uint8_t)(b/count)};
            shared_memory->s1_index.store(idx+1,memory_order_release);
        }
    }
}


void S2_find_details(image_t* input,SharedData* shared_memory) {
    int total=input->width*input->height;
    for (int idx=0;idx<total;idx++) {
        while (shared_memory->s1_index.load(memory_order_acquire)<=idx)
            this_thread::yield();

        int i=idx/input->width;
        int j=idx%input->width;
        array<uint8_t,3> smooth=shared_memory->smooth_buffer[idx];
        array<uint8_t,3> detail;

        for (int c=0;c<3;c++) {
            int diff=(int)input->image_pixels[i][j][c]-(int)smooth[c];
            diff=std::clamp(diff,0,255);
            detail[c]=(uint8_t)diff;
        }

        shared_memory->detail_buffer[idx]=detail;
        shared_memory->s2_index.store(idx+1,memory_order_release);
    }
}


void S3_sharpen(image_t* input,image_t* output,SharedData* shared_memory) {
    int total=input->width*input->height;
    for (int idx=0;idx<total;idx++) {
        while (shared_memory->s2_index.load(memory_order_acquire)<=idx)
            this_thread::yield();

        int i=idx/input->width;
        int j=idx%input->width;
        array<uint8_t,3> detail=shared_memory->detail_buffer[idx];

        for (int c=0;c<3;c++) {
            int val=(int)input->image_pixels[i][j][c]+(int)detail[c];
            val=std::clamp(val, 0, 255);
            output->image_pixels[i][j][c]=(uint8_t)val;
        }
    }
}


int main(int argc,char* argv[]) {
    if (argc<3) {
        cerr<< "Usage: ./part2A.out <input.ppm> <output.ppm>\n";
        return 1;
    }

    int total_iterations;
    cout<< "Enter number of iterations: ";
    cin>> total_iterations;
    if (total_iterations<=0) {
        cerr<<"total_iterations must be > 0\n";
        return 1;
    }

    image_t* input=read_ppm_file(argv[1]);
    image_t* output=allocate_image(input->width, input->height);

    SharedData* shared_memory = new SharedData;
    shared_memory->smooth_buffer.resize(input->width*input->height);
    shared_memory->detail_buffer.resize(input->width*input->height);

    auto starting_total_time=high_resolution_clock::now();

    for (int iter=0;iter<total_iterations;iter++) {
        shared_memory->s1_index.store(0);
        shared_memory->s2_index.store(0);

        auto starting_iteration=high_resolution_clock::now();

        thread t1(S1_smoothen,input,shared_memory);
        thread t2(S2_find_details,input,shared_memory);
        thread t3(S3_sharpen,input,output,shared_memory);

        t1.join();
        t2.join();
        t3.join();

        auto end_iter=high_resolution_clock::now();
        if (iter==0 || iter==total_iterations-1)
            cout<<"Iteration " << iter + 1 << "/" <<total_iterations
                 <<" time: " << duration_cast<milliseconds>(end_iter - starting_iteration).count() << " ms\n";
    }
        

    auto end_total=high_resolution_clock::now();
    write_ppm_file(argv[2], output);

    cout<< "\nThread-based Pipeline (Atomics)";
    cout<< "total_iterations: " << total_iterations << "\n";
    cout<< "Total TIme taken: "
         << duration_cast<milliseconds>(end_total - starting_total_time).count() << " ms\n";
    cout<<"Completed Execution.\n";
    cout<<"Output written to the file : " << argv[2] << endl;

    delete shared_memory;
    return 0;
}