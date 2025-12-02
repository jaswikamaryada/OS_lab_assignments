#include "libppm.h"
#include <iostream>
#include <fstream>
#include <cstdint>  // needed for uint8_t

using namespace std;

uint8_t skip_blanks_comments_while_reading(ifstream *read_stream)
{
    uint8_t val;
    while (true)
    {
        val = read_stream->get();
        if (val == '#')
        {
            while (val != '\n')
            {
                val = read_stream->get();
            }
        }
        if (val == '\n' || val == ' ' || val == '\t')
        {
            continue;
        }
        else if (val != '#')
            return val;
    }
    return val;
}

struct image_t* read_ppm_file(char* path_to_input_file)
{
    ifstream read_stream(path_to_input_file, ios::binary | ios::in);

    if (read_stream.is_open())
    {
        struct image_t* image = new struct image_t;
        image->width = 0;
        image->height = 0;

        uint8_t val = skip_blanks_comments_while_reading(&read_stream); // 'P'
        val = read_stream.get(); //'6'

        // width
        val = skip_blanks_comments_while_reading(&read_stream);
        while (true)
        {
            if (val == ' ' || val == '\t' || val == '\n')
                break;
            image->width = image->width * 10 + (val - '0');
            val = read_stream.get();
        }

        // height
        val = skip_blanks_comments_while_reading(&read_stream);
        while (true)
        {
            if (val == ' ' || val == '\t' || val == '\n')
                break;
            image->height = image->height * 10 + (val - '0');
            val = read_stream.get();
        }

        // allocate memory
        image->image_pixels = new uint8_t**[image->height];
        for (int i = 0; i < image->height; i++)
        {
            image->image_pixels[i] = new uint8_t*[image->width];
            for (int j = 0; j < image->width; j++)
                image->image_pixels[i][j] = new uint8_t[3];
        }

        // maxval
        val = skip_blanks_comments_while_reading(&read_stream);
        while (true)
        {
            if (val == ' ' || val == '\t' || val == '\n')
                break;
            val = read_stream.get();
        }

        // get pixel values
        val = skip_blanks_comments_while_reading(&read_stream);
        for (int i = 0; i < image->height; i++)
        {
            for (int j = 0; j < image->width; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    image->image_pixels[i][j][k] = val;
                    val = read_stream.get(); // assuming maxval <= 255
                }
            }
        }

        read_stream.close();
        return image;
    }
    else
    {
        cerr << "failed to open file " << path_to_input_file << "\n\n";
        exit(1);
    }
}

void write_ppm_file(char *path_to_output_file, struct image_t* image)
{
    ofstream write_stream(path_to_output_file, ios::binary | ios::out);

    if (write_stream.is_open())
    {
        write_stream.write("P6\n", 3);
        std::string width_string = std::to_string(image->width);
        write_stream.write(width_string.c_str(), width_string.length());
        write_stream.write(" ", 1);
        std::string height_string = std::to_string(image->height);
        write_stream.write(height_string.c_str(), height_string.length());
        write_stream.write("\n255\n", 5);

        for (int i = 0; i < image->height; i++)
        {
            for (int j = 0; j < image->width; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    write_stream.put(image->image_pixels[i][j][k]);
                }
            }
        }

        write_stream.close();
    }
    else
    {
        cerr << "failed to open file " << path_to_output_file << "\n\n";
        exit(1);
    }
}

// Helper to allocate a blank image structure
struct image_t* allocate_image(int width, int height)
{
    struct image_t* img = new struct image_t;
    img->width = width;
    img->height = height;

    img->image_pixels = new uint8_t**[height];
    for (int i = 0; i < height; i++)
    {
        img->image_pixels[i] = new uint8_t*[width];
        for (int j = 0; j < width; j++)
        {
            img->image_pixels[i][j] = new uint8_t[3];
        }
    }

    return img;
}
