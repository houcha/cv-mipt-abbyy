#pragma once

#include "bitmap/bitmap_image.hpp"


template <int BLOCK_SIZE>
struct Block {
  void Read(const bitmap_image& image, size_t up_left_x, size_t up_left_y) {
    x_ = up_left_x;
    y_ = up_left_y;

    for (size_t y = 0; y < BLOCK_SIZE; ++y) {
      for (size_t x = 0; x < BLOCK_SIZE; ++x) {
        image.get_pixel(x_ + x, y_ + y, data_[y*BLOCK_SIZE + x]);
      }
    }
  }

  void Write(bitmap_image& image) {
    for (size_t y = 0; y < BLOCK_SIZE; ++y) {
      for (size_t x = 0; x < BLOCK_SIZE; ++x) {
        image.set_pixel(x_ + x, y_ + y, data_[y*BLOCK_SIZE + x]);
      }
    }
  }

  void Print() {
    for (size_t y = 0; y < BLOCK_SIZE; ++y) {
      for (size_t x = 0; x < BLOCK_SIZE; ++x) {
        rgb_t d = data_[y*BLOCK_SIZE + x];
        printf("%3d,%3d,%3d\t", d.red, d.green, d.blue);
      }
      printf("\n\n");
    }
  }

  inline rgb_t* Data() {
    return data_;
  }

  inline size_t Size() {
    return BLOCK_SIZE;
  }

  size_t x_;
  size_t y_;
  rgb_t data_[BLOCK_SIZE * BLOCK_SIZE];
};


