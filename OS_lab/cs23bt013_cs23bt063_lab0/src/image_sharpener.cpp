#include <iostream>
#include <cstdint>
#include <chrono>
#include "libppm.h"

using namespace std;
using namespace std::chrono;

struct image_t* S1_smoothen(struct image_t *input_image)
{
    struct image_t* image=new struct image_t;
    image->height=input_image->height;
    image->width=input_image->width;
    image->image_pixels=new uint8_t**[image->height];
    for(int i=0; i<image->height; i++)
    {
        image->image_pixels[i] = new uint8_t*[image->width];
        for(int j=0; j< image->width; j++)
            image->image_pixels[i][j] = new uint8_t[3];
    }
    for(int i= 0;i< image->height;i++)
    {
        for(int j=0;j< image->width;j++)
        {
            int r=0, g=0, b=0;
            int count=0;
            for(int row=i-1;row<= i+1;row++)
            {
                for(int col=j-1;col<=j+1;col++)
                {
                    if(row>=0 && row< image->height && col>= 0 && col< image->width)
                    {
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

struct image_t* S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
    struct image_t* details=new struct image_t;
    details->height=input_image->height;
    details->width=input_image->width;
    details->image_pixels=new uint8_t**[details->height];
    for(int i=0;i< details->height;i++)
    {
        details->image_pixels[i]=new uint8_t*[details->width];
        for(int j=0;j<details->width;j++)
            details->image_pixels[i][j]=new uint8_t[3];
    }

    for(int i= 0; i<details->height;i++)
    {
        for(int j=0; j<details->width;j++)
        {
            for(int c=0;c<3;c++)
            {
                int diff = (int)input_image->image_pixels[i][j][c] - 
                           (int)smoothened_image->image_pixels[i][j][c];
                diff = min(max(diff, 0), 255);
                details->image_pixels[i][j][c] = (uint8_t)diff;
            }
        }
    }
    return details;
}

struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image) 
{
    struct image_t* sharpened=new struct image_t;
    sharpened->height=input_image->height;
    sharpened->width=input_image->width;
    sharpened->image_pixels=new uint8_t**[sharpened->height];
    for(int i=0;i<sharpened->height;i++)
    {
        sharpened->image_pixels[i]=new uint8_t*[sharpened->width];
        for(int j=0;j <sharpened->width;j++)
            sharpened->image_pixels[i][j]=new uint8_t[3];
    }

    for(int i=0;i<sharpened->height;i++)
    {
        for(int j=0;j< sharpened->width;j++)
        {
            for(int c=0; c<3; c++)
            {
                int val=(int)input_image->image_pixels[i][j][c] + 
                          (int)details_image->image_pixels[i][j][c];
                val=min(max(val, 0), 255);
                sharpened->image_pixels[i][j][c]=(uint8_t)val;
            }
        }
    }
    return sharpened;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

    auto start_read = high_resolution_clock::now();
    struct image_t *input_image = read_ppm_file(argv[1]);
    auto end_read = high_resolution_clock::now();

    auto start_s1 = high_resolution_clock::now();
    struct image_t *smoothened_image = S1_smoothen(input_image);
    auto end_s1 = high_resolution_clock::now();

    auto start_s2 = high_resolution_clock::now();
    struct image_t *details_image = S2_find_details(input_image, smoothened_image);
    auto end_s2 = high_resolution_clock::now();

    auto start_s3 = high_resolution_clock::now();
    struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
    auto end_s3 = high_resolution_clock::now();

    auto start_write = high_resolution_clock::now();
    write_ppm_file(argv[2], sharpened_image);
    auto end_write = high_resolution_clock::now();

    cout << "Time (read): " << duration_cast<milliseconds>(end_read - start_read).count() << " ms\n";
    cout << "Time (S1 smoothen): " << duration_cast<milliseconds>(end_s1 - start_s1).count() << " ms\n";
    cout << "Time (S2 find details): " << duration_cast<milliseconds>(end_s2 - start_s2).count() << " ms\n";
    cout << "Time (S3 sharpen): " << duration_cast<milliseconds>(end_s3 - start_s3).count() << " ms\n";
    cout << "Time (write): " << duration_cast<milliseconds>(end_write - start_write).count() << " ms\n";

    return 0;
}