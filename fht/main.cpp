#include <cstdio>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>


const int DegreesInPi = 180;
const double Pi = 3.14159265;


bool IsPowerOf2(int n) {
  return (n & (n - 1)) == 0;
}

int RoundToNextPowerOf2(int x) {
  return pow(2, ceil(log2(double(x))));
}

int DegreeToGridTicks(double deg, int ticksInGrid) {
  return int(deg * ticksInGrid / DegreesInPi);
}


/// FHT from -pi/2 to -pi/4
cv::Mat FHT_mPi2_to_mPi4(const cv::Mat& I) {
  const int n = I.rows;

  cv::Mat R(n, 2*n, CV_32F, cv::Scalar(0.));
  I.copyTo(R(cv::Rect(0, 0, n, n)));

  for (int i = 1; i <= log2(n); ++i) {
    cv::Mat Rnew(n, 2*n, CV_32F, cv::Scalar(0.));

    const int yStep = pow(2, i);
    const int yStepPrev = pow(2, i-1);

    // TODO: does x direction matter?
    for (int y = 0; y < n; y += yStep) {
      for (int x = 0; x < 2*n; ++x) {
        for (int a = 0; a < yStep; ++a) {
          const float upStick = R.at<float>(y + a/2, x);
          const float downStick = R.at<float>(y + yStepPrev + a/2, (x + (a+1)/2) % (2*n));

          Rnew.at<float>(y + a, x) = upStick + downStick;
        }
      }
    }
    R = Rnew;
  }
  return R;
}

void FastHoughTransform(const cv::Mat& I, cv::Mat& drt) {
  assert(I.rows == I.cols);
  assert(IsPowerOf2(I.rows));
  const int n = I.rows;

  cv::Mat It = I.t();
  cv::Mat Iclockwise(n, n, I.type());
  cv::rotate(I, Iclockwise, cv::ROTATE_90_CLOCKWISE);
  cv::Mat IclockwiseT = Iclockwise.t();

  // from -pi/2 to -pi/4
  cv::Mat R = FHT_mPi2_to_mPi4(I);

  for (int x = 0; x < n; ++x) {
    for (int a = 0; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      // some geometry behind this
      const int d = int((x + a) * cos(thetaRads));
      //const int d = int(x * cos(thetaRads));

      const double thetaDegs = DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += R.at<float>(a, x);
    }
  }

  for (int x = n; x < 2*n; ++x) {
    for (int a = 2*n - x - 1; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int((x + a - 2*n) * cos(thetaRads));

      const double thetaDegs = DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += R.at<float>(a, x);
    }
  }

  // from -pi/4 to 0
  cv::Mat Rt = FHT_mPi2_to_mPi4(It);

  for (int x = 0; x < n; ++x) {
    for (int a = 0; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int((n - x) * cos(thetaRads));
      //const int d = int(x * cos(thetaRads));

      const double thetaDegs = DegreesInPi/2 - DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += Rt.at<float>(a, x);
    }
  }

  for (int x = n; x < 2*n; ++x) {
    for (int a = 2*n - x - 1; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int((3*n - x) * cos(thetaRads));
      //const int d = int(x * cos(thetaRads));

      const double thetaDegs = DegreesInPi/2 - DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += Rt.at<float>(a, x);
    }
  }

  // from 0 to pi/4
  cv::Mat Rclockwise = FHT_mPi2_to_mPi4(Iclockwise);

  for (int x = 0; x < n; ++x) {
    for (int a = 0; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int(x * cos(thetaRads));
      //const int d = int((n-x) * cos(thetaRads));

      const double thetaDegs = DegreesInPi/2 + DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += Rclockwise.at<float>(a, x);
    }
  }

  for (int x = n; x < 2*n; ++x) {
    for (int a = 2*n - x - 1; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int((2*n - x) * cos(thetaRads));

      const double thetaDegs = DegreesInPi/2 + DegreesInPi * thetaRads/Pi;
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows);
      drt.at<float>(theta, d) += Rclockwise.at<float>(a, x);
    }
  }

  // from pi/4 to pi/2
  cv::Mat RclockwiseT = FHT_mPi2_to_mPi4(IclockwiseT);

  for (int x = 0; x < n; ++x) {
    for (int a = 0; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int(x * cos(thetaRads));
      //const int d = int((x + a) * cos(thetaRads));

      const double thetaDegs = (DegreesInPi - DegreesInPi * thetaRads/Pi);
      // avoid +90, since there's -90 for this case
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows) % drt.rows;
      drt.at<float>(theta, d) += RclockwiseT.at<float>(a, x);
    }
  }

  for (int x = n; x < 2*n; ++x) {
    for (int a = 2*n - x - 1; a < n; ++a) {
      const double thetaRads = atan2(a, n-1);
      const int d = int((2*n - x) * cos(thetaRads));

      const double thetaDegs = (DegreesInPi - DegreesInPi * thetaRads/Pi);
      // avoid +90, since there's -90 for this case
      const int theta = DegreeToGridTicks(thetaDegs, drt.rows) % drt.rows;
      drt.at<float>(theta, d) += RclockwiseT.at<float>(a, x);
    }
  }
}

int GetImgSlopeAngle(const cv::Mat& img) {
  const int n = RoundToNextPowerOf2(std::max(img.cols, img.rows));

  // Pad
  cv::Mat imgPadded(n, n, img.type());
  cv::copyMakeBorder(
      img,
      imgPadded,
      0,              // top
      n - img.rows,   // bottom
      0,              // left
      n - img.cols,   // right
      cv::BORDER_CONSTANT,
      0
  );
  // Split into channels
  std::vector<cv::Mat> channels;
  cv::split(imgPadded, channels);

  // discrete Radon transform
  //const int ticksInDegree = n / 45;
  int ticksInDegree = 1;
  while(DegreesInPi * ticksInDegree < n) {
    ++ticksInDegree;
  }

  cv::Mat drt(DegreesInPi * ticksInDegree, n, CV_32F, cv::Scalar(0.));

  for (const cv::Mat& channel : channels) {
    FastHoughTransform(channel, drt);
  }

  double minVal;
  double maxVal;
  cv::Point minLoc;
  cv::Point maxLoc;
  minMaxLoc( drt, &minVal, &maxVal, &minLoc, &maxLoc );

  cv::Mat drtu(drt.size(), CV_8U);
  for (int theta = 0; theta < drt.rows; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      drtu.at<uchar>(theta, d) = int((drt.at<float>(theta, d) - minVal) / (maxVal - minVal) * 255.);
      assert(0 <= drtu.at<uchar>(theta, d) && drtu.at<uchar>(theta, d) <= 255);
    }
  }
  cv::imwrite("drt.jpg", drtu);

  float maxDistortion = 0;
  int slopeAngle = 0;
  for (int theta = 0; theta < drt.rows; ++theta) {

    float currentDistortion = 0;
    for (int d = 0; d < drt.cols; ++d) {
      const int val = drt.at<float>(theta, d);
      currentDistortion += val*val;
    }

    if (maxDistortion < currentDistortion) {
      maxDistortion = currentDistortion;
      slopeAngle = theta / ticksInDegree - DegreesInPi/2;
    }
  }

  return slopeAngle;
}


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("use: %s <input_image_path> <output_folder_path>\n", argv[0]);
    return 1;
  }

  const char* input = argv[1];
  const char* output = argv[2];

  cv::Mat src = cv::imread(input);
  const int slopeAngle = GetImgSlopeAngle(src);

  printf("Slope angle: %d\n", slopeAngle);

  cv::Point2f center(src.cols/2., src.rows/2.);
  cv::Mat transormMatrix = cv::getRotationMatrix2D(center, -slopeAngle, 1.0);

  cv::Mat dst(src);
  cv::warpAffine(src, dst, transormMatrix, src.size(), cv::INTER_LINEAR);

  cv::imwrite(output, dst);
  return 0;
}
