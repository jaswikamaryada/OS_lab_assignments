#include "libppm.h"
#include <iostream>
#include <semaphore.h>
#include <chrono>
#include <algorithm>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
using namespace chrono;


struct SharedData{
    uint8_t pixel_of_s1[3]; 
    uint8_t pixel_of_s2[3]; 
    sem_t sem_empty;    
    sem_t sem_s1_done;  
    sem_t sem_s2_done;   
};


void S1_smoothen(image_t* input,SharedData* shared_mem,int time_fd,int total_iterations) {
    auto starting_time=high_resolution_clock::now();

    for (int iter=0;iter<total_iterations; iter++) {
        auto iter_starting_time=high_resolution_clock::now();

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
                sem_wait(&shared_mem->sem_empty);
                shared_mem->pixel_of_s1[0]=r/count;
                shared_mem->pixel_of_s1[1]=g/count;
                shared_mem->pixel_of_s1[2]=b/count;
                sem_post(&shared_mem->sem_s1_done);
            }
        }

        auto end_of_iteration=high_resolution_clock::now();
        if (iter==0 || iter==total_iterations-1)
            cout << "[S1] Iteration "<<iter+1<<" time: "
                 << duration_cast<milliseconds>(end_of_iteration-iter_starting_time).count() << " ms\n";
    }

    auto end=high_resolution_clock::now();
    long long duration=duration_cast<milliseconds>(end-starting_time).count();
    write(time_fd,&duration,sizeof(duration));
    _exit(0);
}


void S2_find_details(image_t* input, SharedData* shared_mem, int time_fd, int total_iterations) {
    auto starting_time=high_resolution_clock::now();

    for (int iter=0;iter<total_iterations; iter++) {
        auto iter_starting_time=high_resolution_clock::now();
        for (int i=0;i<input->height;i++) {
            for (int j=0;j<input->width;j++) {
                sem_wait(&shared_mem->sem_s1_done);
                for (int c=0;c<3;c++){
                    int diff=(int)input->image_pixels[i][j][c]-(int)shared_mem->pixel_of_s1[c];
                    diff=std::clamp(diff,0,255);
                    shared_mem->pixel_of_s2[c]=(uint8_t)diff;
                }
                sem_post(&shared_mem->sem_s2_done);
            }
        }

        auto end_of_iteration=high_resolution_clock::now();
        if (iter==0 || iter==total_iterations-1)
            cout <<"[S2] Iteration "<<iter+1 << " time: "
                 <<duration_cast<milliseconds>(end_of_iteration-iter_starting_time).count() << " ms\n";
    }

    auto end=high_resolution_clock::now();
    long long duration=duration_cast<milliseconds>(end-starting_time).count();
    write(time_fd,&duration,sizeof(duration));
    _exit(0);
}

void S3_sharpen(image_t* input, SharedData* shared_mem,const char* output_path,int time_fd,int total_iterations) {
    auto starting_time=high_resolution_clock::now();
    image_t* output=allocate_image(input->width, input->height);

    for (int iter=0;iter<total_iterations;iter++) {
        auto iter_starting_time=high_resolution_clock::now();

        for (int i=0;i<input->height;i++) {
            for (int j=0;j<input->width;j++) {
                sem_wait(&shared_mem->sem_s2_done);
                for (int c=0;c<3;c++) {
                    int valueof_pixel=(int)input->image_pixels[i][j][c]+(int)shared_mem->pixel_of_s2[c];
                    valueof_pixel=std::clamp(valueof_pixel,0,255);
                    output->image_pixels[i][j][c]=(uint8_t)valueof_pixel;
                }
                sem_post(&shared_mem->sem_empty);
            }
        }

        auto end_of_iteration=high_resolution_clock::now();
        if (iter==0 || iter==total_iterations-1)
            cout <<"[S3] Iteration "<<iter+1<< " time: "
                 << duration_cast<milliseconds>(end_of_iteration-iter_starting_time).count() << " ms\n";
    }

    write_ppm_file((char*)output_path,output);

    auto end=high_resolution_clock::now();
    long long duration_time=duration_cast<milliseconds>(end-starting_time).count();
    write(time_fd,&duration_time,sizeof(duration_time));
    _exit(0);
}

int main(int argc,char* argv[]) {
    if (argc<3) {
        cerr <<"Usage: ./part2C.out <input.ppm> <output.ppm>\n";
        return 1;
    }

    int total_iterations;
    cout<<"Enter number of iterations: ";
    cin>>total_iterations;
    if (total_iterations<=0) {
        cerr<<"ENter the correct number of iterations.total of iterations must be > 0\n";
        return 1;
    }

    image_t* input = read_ppm_file(argv[1]);

   
    SharedData* shared_mem=(SharedData*)mmap(NULL,sizeof(SharedData),
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(&shared_mem->sem_empty,1,1);
    sem_init(&shared_mem->sem_s1_done,1,0);
    sem_init(&shared_mem->sem_s2_done,1,0);

    int tpipe1[2],tpipe2[2],tpipe3[2];
    pipe(tpipe1); pipe(tpipe2); pipe(tpipe3);

    auto starting_time_total=high_resolution_clock::now();

    pid_t pid1 = fork();
    if (pid1==0) S1_smoothen(input,shared_mem,tpipe1[1],total_iterations);

    pid_t pid2=fork();
    if (pid2==0) S2_find_details(input, shared_mem, tpipe2[1],total_iterations);

    pid_t pid3=fork();
    if (pid3==0) S3_sharpen(input,shared_mem,argv[2],tpipe3[1],total_iterations);

    long long t1=0,t2=0,t3=0;
    read(tpipe1[0],&t1,sizeof(t1));
    read(tpipe2[0],&t2,sizeof(t2));
    read(tpipe3[0],&t3,sizeof(t3));

    waitpid(pid1,nullptr,0);
    waitpid(pid2,nullptr,0);
    waitpid(pid3,nullptr,0);

    auto end_total=high_resolution_clock::now();

    cout<<"\nProcess-based Pipeline(Shared Memory+Semaphores)";
    cout<<"Iterations: " << total_iterations << "\n";
    cout<<"Total time taken for S1 in all"<< " " << total_iterations << " " << "iterations is " << " " << t1 << " ms\n";
    cout<<"Total time taken for S1 in all"<< " " << total_iterations << " " << "iterations is " << " " << t2 << " ms\n";
    cout<<"Total time taken for S1 in all"<< " " << total_iterations << " " << "iterations is " << " " << t3 << " ms\n";
    cout<<"\nTotal time taken: "
         << duration_cast<milliseconds>(end_total-starting_time_total).count()
         << " ms\n" << endl;
    cout <<"\nExecutionnn of Multi-processes pipeline (Shared Memory+Semaphores) completed\n";
    cout <<"Output written to: "<<argv[2]<<endl;

    sem_destroy(&shared_mem->sem_empty);
    sem_destroy(&shared_mem->sem_s1_done);
    sem_destroy(&shared_mem->sem_s2_done);
    munmap(shared_mem, sizeof(SharedData));

    return 0;
}