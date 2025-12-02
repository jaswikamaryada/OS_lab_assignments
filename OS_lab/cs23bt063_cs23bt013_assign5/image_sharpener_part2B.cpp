#include "libppm.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace chrono;


void S1_smoothening_image(image_t* input,int pipefd[2],int timepipe[2],int iterations) {
    auto start=high_resolution_clock::now();
    close(pipefd[0]);

    for (int iter=0;iter<iterations;iter++) {
        auto starting_iteration=high_resolution_clock::now();
        for (int i=0;i<input->height;i++) {
            for (int j=0;j<input->width;j++) {
                uint8_t pixel_array[3]={0};
                int r=0,g=0,b=0,count = 0;
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
                pixel_array[0]=r/count;
                pixel_array[1]=g/count;
                pixel_array[2]=b/count;

                if(iter==iterations-1)
                   write(pipefd[1],pixel_array,3);
            }
        }

        auto ending_iteration=high_resolution_clock::now();
        if (iter==0 || iter==iterations-1) {
            cout<<"[S1] Iteration "<<iter+1<<" time: "
                 << duration_cast<milliseconds>(ending_iteration-starting_iteration).count()<< " ms\n";
        }
    }

    close(pipefd[1]);
    auto end=high_resolution_clock::now();
    long long duration=duration_cast<milliseconds>(end-start).count();
    close(timepipe[0]);
    write(timepipe[1],&duration,sizeof(duration));
    close(timepipe[1]);
    _exit(0);
}


void S2_finding_details(image_t* input,int pipe_in[2],int pipe_out[2],int timepipe[2],int iterations) {
    auto start=high_resolution_clock::now();
    close(pipe_in[1]);
    close(pipe_out[0]);

    uint8_t smoothening_pixels[3],detail_pixel[3];

    for (int iter=0;iter<iterations;iter++) {
        auto starting_iteration=high_resolution_clock::now();

        for (int i=0;i<input->height;i++) {
            for (int j=0;j<input->width;j++) {
                if (iter==iterations-1)
                    read(pipe_in[0],smoothening_pixels,3);

                for (int c=0;c<3;c++) {
                    int diff=(int)input->image_pixels[i][j][c]-(int)smoothening_pixels[c];
                    diff=std::clamp(diff,0,255);
                    detail_pixel[c]=(uint8_t)diff;
                }

                if (iter==iterations-1)
                    write(pipe_out[1],detail_pixel,3);
            }
        }

        auto ending_iteration=high_resolution_clock::now();
        if (iter == 0 || iter==iterations-1) {
            cout << "[S2] Iteration " <<iter+1<<" time: "
                 <<duration_cast<milliseconds>(ending_iteration-starting_iteration).count() << " ms\n";
        }
    }

    close(pipe_in[0]);
    close(pipe_out[1]);

    auto end = high_resolution_clock::now();
    long long duration=duration_cast<milliseconds>(end - start).count();
    close(timepipe[0]);
    write(timepipe[1], &duration, sizeof(duration));
    close(timepipe[1]);
    _exit(0);
}


void S3_sharpen(image_t* input,int pipe_in[2],const char* path_of_output,int timepipe[2],int iterations) {
    auto start = high_resolution_clock::now();
    close(pipe_in[1]);

    image_t* output = allocate_image(input->width, input->height);
    uint8_t detail_pixel[3];

    for (int iter=0;iter<iterations;iter++) {
        auto starting_iteration=high_resolution_clock::now();

        for (int i=0;i<input->height;i++) {
            for (int j=0;j<input->width;j++) {
                if (iter==iterations-1)
                    read(pipe_in[0],detail_pixel,3);

                for (int c=0;c<3;c++) {
                    int val=(int)input->image_pixels[i][j][c]+(int)detail_pixel[c];
                    val = std::clamp(val, 0, 255);
                    output->image_pixels[i][j][c] = (uint8_t)val;
                }
            }
        }

        auto ending_iteration=high_resolution_clock::now();
        if (iter==0 || iter==iterations-1) {
            cout << "[S3] Iteration "<<iter+1<<" time: "
                 << duration_cast<milliseconds>(ending_iteration-starting_iteration).count() << " ms\n";
        }
    }

    close(pipe_in[0]);
    write_ppm_file((char*)path_of_output,output);

    auto end=high_resolution_clock::now();
    long long duration=duration_cast<milliseconds>(end - start).count();
    close(timepipe[0]);
    write(timepipe[1], &duration, sizeof(duration));
    close(timepipe[1]);
    _exit(0);
}


int main(int argc,char* argv[]) {
    if (argc<3) {
        cerr<<"Usage: ./part2B.out <input.ppm> <output.ppm>\n";
        return 1;
    }

    int iterations;
    cout<<"Enter number of iterations: ";
    cin>>iterations;
    if (iterations<=0) {
        cerr<<"iterations must be > 0\n";
        return 1;
    }

    image_t* input=read_ppm_file(argv[1]);
    int pipe1[2], pipe2[2], time1[2], time2[2], time3[2];
    pipe(pipe1);
    pipe(pipe2);
    pipe(time1);
    pipe(time2);
    pipe(time3);

    auto start_total=high_resolution_clock::now();

    pid_t pid1=fork();
    if (pid1==0) S1_smoothening_image(input,pipe1,time1,iterations);

    pid_t pid2=fork();
    if (pid2 == 0) S2_finding_details(input,pipe1,pipe2,time2,iterations);

    pid_t pid3 = fork();
    if (pid3 == 0) S3_sharpen(input,pipe2,argv[2],time3,iterations);

    close(time1[1]);
    close(time2[1]);
    close(time3[1]);

    long long t1=0,t2=0,t3=0;
    read(time1[0],&t1,sizeof(t1));
    read(time2[0],&t2,sizeof(t2));
    read(time3[0],&t3,sizeof(t3));

    waitpid(pid1,nullptr,0);
    waitpid(pid2,nullptr,0);
    waitpid(pid3,nullptr,0);

    auto end_total=high_resolution_clock::now();

    cout<<"Executing multiple processes that communicates via pipes \n";
    cout<<"iterations: "<<iterations<<"\n";
    cout<<"Total time taken for S1 in all"<< " " << iterations << " " << "iterations is " << " " << t1 << " ms\n";
    cout<<"Total time taken for S2 in all "<< " " << iterations << " " << "iterations is " << " " << t2 << " ms\n";
    cout<<"Total time taken for S3 in all"<< " " << iterations << " " << "iterations is " << " " << t3 << " ms\n";
    cout<<endl;
    cout<<"Total TIme Taken: "
         <<duration_cast<milliseconds>(end_total-start_total).count()
         <<" ms\n";
    cout<<"Exectuion completed.\n";
    cout<<"Output written to: "<<argv[2]<<endl;

    return 0;
}