#include <cstdio>
#include <algorithm>
#include "bitmap_image.hpp"
#include "block.hpp"
#include "ppg.hpp"


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: interpolation.out <inputFile.bmp> <outputFile.bmp>\n");
    return 1;
  }

  char* input_file = argv[1];
  char* output_file = argv[2];

  bitmap_image image(input_file);

  if (!image) {
    printf("Error - Failed to open: %s\n", input_file);
    return 2;
  }

  const uint32_t height = image.height();
  const uint32_t width  = image.width();

  Block<5> block5x5;

  // iterate over red/blue pixels
  for (size_t y = 0; y + block5x5.Size() <= height; ++y) {
    for (size_t x = y % 2; x + block5x5.Size() <= width; x += 2) {
      block5x5.Read(image, x, y);

      if (y % 2 == 0) { // red center
        interpolate_green_for_red(block5x5.Data());
      } else {          // blue center
        interpolate_green_for_blue(block5x5.Data());
      }

      block5x5.Write(image);
    }
  }

  Block<3> block3x3;

  // iterate over green pixels
  for (size_t y = 0; y + block3x3.Size() <= height; ++y) {
    for (size_t x = (y+1) % 2; x + block3x3.Size() <= width; x += 2) {
      block3x3.Read(image, x, y);

      interpolate_RB_for_green(block3x3.Data());

      block3x3.Write(image);
    }
  }

  // iterate over red/blue pixels
  for (size_t y = 0; y + block5x5.Size() <= height; ++y) {
    for (size_t x = y % 2; x + block5x5.Size() <= width; x += 2) {
      block5x5.Read(image, x, y);

      if (y % 2 == 0) { // red center
        interpolate_blue_for_red(block5x5.Data());
      } else {          // blue center
        interpolate_red_for_blue(block5x5.Data());
      }

      block5x5.Write(image);
    }
  }

  image.save_image(output_file);

  return 0;
}

