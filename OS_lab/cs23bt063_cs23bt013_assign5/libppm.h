#ifndef LIBPPM_H
#define LIBPPM_H

#include <cstdint>

struct image_t
{
    int width;
    int height;
    uint8_t*** image_pixels;
};

// Reads a PPM file and returns a pointer to an image_t structure
struct image_t* read_ppm_file(char* path_to_input_file);

// Writes an image_t structure to a PPM file
void write_ppm_file(char* path_to_output_file, struct image_t* image);

// Helper to allocate an image of given width and height
struct image_t* allocate_image(int width, int height);

#endif
