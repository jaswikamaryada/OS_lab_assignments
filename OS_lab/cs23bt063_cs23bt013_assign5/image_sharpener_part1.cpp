#include <iostream>
#include <cstdint>
#include <chrono>
#include "libppm.h"
#include <algorithm>

using namespace std;
using namespace std::chrono;


struct image_t* S1_smoothen_image(struct image_t *input_image)
{
    struct image_t* image=new image_t;
    image->height=input_image->height;
    image->width=input_image->width;
    image->image_pixels=new uint8_t**[image->height];
    for (int i=0;i<image->height;i++) {
        image->image_pixels[i]=new uint8_t*[image->width];
        for (int j=0;j<image->width;j++)
            image->image_pixels[i][j]=new uint8_t[3];
    }

    for (int i=0;i<image->height;i++) {
        for (int j=0;j<image->width;j++) {
            int r=0,g=0,b=0,count=0;
            for (int row=i-1;row<=i+1;row++) {
                for (int col=j-1;col<=j+1;col++) {
                    if (row>=0 && row<image->height && col>=0 && col<image->width) {
                        r+=input_image->image_pixels[row][col][0];
                        g+=input_image->image_pixels[row][col][1];
                        b+=input_image->image_pixels[row][col][2];
                        count++;
                    }
                }
            }
            image->image_pixels[i][j][0]=r/count;
            image->image_pixels[i][j][1]=g/count;
            image->image_pixels[i][j][2]=b/count;
        }
    }
    return image;
}

// ---------- Stage 2: Find Details ----------
struct image_t* S2_find_details(struct image_t *input_image,struct image_t *smoothened_image)
{
    struct image_t* details = new image_t;
    details->height=input_image->height;
    details->width=input_image->width;
    details->image_pixels=new uint8_t**[details->height];
    for (int i=0;i<details->height;i++) {
        details->image_pixels[i] = new uint8_t*[details->width];
        for (int j=0;j<details->width;j++)
            details->image_pixels[i][j]=new uint8_t[3];
    }

    for (int i=0;i<details->height;i++) {
        for (int j=0;j<details->width;j++) {
            for (int c=0;c<3;c++) {
                int diff = (int)input_image->image_pixels[i][j][c] -
                           (int)smoothened_image->image_pixels[i][j][c];
                diff=std::clamp(diff,0,255);
                details->image_pixels[i][j][c]=(uint8_t)diff;
            }
        }
    }
    return details;
}


struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
    struct image_t* sharpening_image = new image_t;
    sharpening_image->height=input_image->height;
    sharpening_image->width=input_image->width;
    sharpening_image->image_pixels=new uint8_t**[sharpening_image->height];
    for (int i=0;i<sharpening_image->height;i++) {
        sharpening_image->image_pixels[i] = new uint8_t*[sharpening_image->width];
        for (int j=0;j<sharpening_image->width;j++)
            sharpening_image->image_pixels[i][j]=new uint8_t[3];
    }

    for (int i=0;i<sharpening_image->height;i++) {
        for (int j=0;j<sharpening_image->width;j++) {
            for (int c=0;c<3;c++) {
                int val=(int)input_image->image_pixels[i][j][c] +
                          (int)details_image->image_pixels[i][j][c];
                val=std::clamp(val,0,255);
                sharpening_image->image_pixels[i][j][c]=(uint8_t)val;
            }
        }
    }
    return sharpening_image;
}


int main(int argc, char **argv)
{
    if (argc!=3) {
        cout<<"Enter Correct arguments:Usage:./part1.out <input_image.ppm> <output_image.ppm>\n";
        return 1;
    }

    
    struct image_t *input_image = read_ppm_file(argv[1]);

    
    auto start_time_s1 = high_resolution_clock::now();
    struct image_t *smoothened_image = S1_smoothen_image(input_image);
    auto end_time_s1 = high_resolution_clock::now();

    auto s2_start_time = high_resolution_clock::now();
    struct image_t *details_image = S2_find_details(input_image,smoothened_image);
    auto s2_end_time = high_resolution_clock::now();

    auto s3_start_time = high_resolution_clock::now();
    struct image_t *sharpening_image=S3_sharpen(input_image,details_image);
    auto s3_end_time = high_resolution_clock::now();

    write_ppm_file(argv[2],sharpening_image);

    cout<<"\n---- Computation-Only Time (Sequential) ----\n";
    cout<<"S1 Smoothen: "<<duration_cast<milliseconds>(end_time_s1-start_time_s1).count()<<" ms\n";
    cout<<"S2 Details: "<<duration_cast<milliseconds>(s2_end_time-s2_start_time).count()<<" ms\n";
    cout<<"S3 Sharpen: "<<duration_cast<milliseconds>(s3_end_time-s3_start_time).count()<<" ms\n";
    cout<<"Total Computation time: "<<duration_cast<milliseconds>(s3_end_time-start_time_s1).count()<<" ms\n";

    cout<<"\nSequential computation complete!\n";
    return 0;
}