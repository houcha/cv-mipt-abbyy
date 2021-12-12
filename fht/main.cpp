#include <cstdio>
#include <opencv2/opencv.hpp>
#include "fht.h"


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("use: %s <input_image_path> <output_folder_path>\n", argv[0]);
    return 1;
  }

  const char* input = argv[1];
  const char* output = argv[2];

  cv::Mat src = cv::imread(input);

  FastHoughTransformer fht(src, 20);
  fht.WriteDrt("drt.jpg");
  const int slopeAngle = fht.GetDrtMaxDistortionAngle();
  printf("Slope angle: %d\n", slopeAngle);

  cv::Point2f center(src.cols/2., src.rows/2.);
  cv::Mat transormMatrix = cv::getRotationMatrix2D(center, -slopeAngle, 1.0);

  cv::Mat dst(src);
  cv::warpAffine(src, dst, transormMatrix, src.size(), cv::INTER_LINEAR);

  cv::imwrite(output, dst);
  return 0;
}
