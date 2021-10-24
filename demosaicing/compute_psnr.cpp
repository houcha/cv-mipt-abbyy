#include <algorithm>
#include "bitmap_image.hpp"


double PSNR(const bitmap_image& original, const bitmap_image& result) {
  uint32_t height = original.height();
  uint32_t width  = original.width();

  uint64_t sum = 0;

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      rgb_t I = original.get_pixel(x, y);
      rgb_t K = result.get_pixel(x, y);

      sum += (I.red - K.red) * (I.red - K.red)
           + (I.green - K.green) * (I.green - K.green)
           + (I.blue - K.blue) * (I.blue - K.blue);
    }
  }

  double MSE = sum / (3. * height * width);

  uint8_t maxI = *std::max_element(
      original.data(),
      original.data() + width * height * original.bytes_per_pixel());

  double PSNR = 10 * std::log10(maxI * maxI / MSE);
  return PSNR;
}


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: a.out <original.bmp> <interpolated.bmp>\n");
    return 1;
  }

  bitmap_image original(argv[1]);
  bitmap_image interpolated(argv[2]);

  printf("%.1f\n", PSNR(original, interpolated));
}
