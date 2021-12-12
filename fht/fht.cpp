#include <cmath>
#include <algorithm>
#include "fht.h"


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

double GridTicksToDegree(int ticks, int ticksInGrid) {
  return (double)ticks / ticksInGrid;
}


FastHoughTransformer::FastHoughTransformer(const cv::Mat& img, int thetaDetalization):
  n(RoundToNextPowerOf2(std::max(img.cols, img.rows))),
  ticksInDegree(thetaDetalization)
{
  // Pad
  cv::Mat imgPadded(n, n, img.type());
  cv::copyMakeBorder(
      img,
      imgPadded,
      0,            // top
      n - img.rows, // bottom
      0,            // left
      n - img.cols, // right
      cv::BORDER_CONSTANT,
      0
  );

  assert(imgPadded.rows == imgPadded.cols);
  assert(IsPowerOf2(imgPadded.rows));

  const int dPositiveMax = int(n * sqrt(2.));
  const int dNegativeMax = 0; //dPositiveMax/2.;

  cv::Mat drt(
      DegreesInPi * ticksInDegree,
      dPositiveMax + dNegativeMax,
      CV_32S,
      cv::Scalar(0)
  );
  cv::swap(this->drt, drt);

  // Split into channels
  std::vector<cv::Mat> channels;
  cv::split(imgPadded, channels);

  // Build discrete Radon transformation table R(theta, d)
  for (const cv::Mat& channel : channels) {
    FastHoughTransform(channel);
  }
}


void FastHoughTransformer::FastHoughTransform(const cv::Mat& I) {
  cv::Mat It = I.t();
  cv::Mat Iclockwise(n, n, I.type());
  cv::rotate(I, Iclockwise, cv::ROTATE_90_CLOCKWISE);
  cv::Mat IclockwiseT = Iclockwise.t();

  // from -pi/2 to -pi/4
  cv::Mat R = FastHoughTransformFromMinusPiDiv2ToMinusPiDiv4(I);

  for (int theta = 0; theta <= drt.rows/4; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      const double thetaRad = (double)theta * Pi / drt.rows;
      const int x = (int(d / cos(thetaRad) - n * tan(thetaRad)) + 2*n) % (2*n);
      const int y = int(n * tan(thetaRad));
      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(theta, d) += R.at<int>(y, x);
    }
  }

  // from -pi/4 to 0
  cv::Mat Rt = FastHoughTransformFromMinusPiDiv2ToMinusPiDiv4(It);

  for (int theta = 0; theta < drt.rows/4; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      const double thetaRad = (double)theta * Pi / drt.rows;
      const int x = (n - int(d / cos(thetaRad)) + 2*n) % (2*n);
      const int y = int(n * tan(thetaRad));
      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(drt.rows/2 - theta, d) += Rt.at<int>(y, x);
    }
  }

  // from 0 to pi/4
  cv::Mat Rclockwise = FastHoughTransformFromMinusPiDiv2ToMinusPiDiv4(Iclockwise);

  //const int dMax = int(drt.cols * sqrt(2.)/2.);
  for (int theta = 0; theta < drt.rows/4; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      const double thetaRad = (double)theta * Pi / drt.rows;
      const int x = (int(d / cos(thetaRad)) + 2*n) % (2*n);
      const int y = int(n * tan(thetaRad));
      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(drt.rows/2 + theta, d) += Rclockwise.at<int>(y, x);
    }
  }

  // from pi/4 to pi/2
  cv::Mat RclockwiseT = FastHoughTransformFromMinusPiDiv2ToMinusPiDiv4(IclockwiseT);

  for (int theta = 0; theta < drt.rows/4; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      const double thetaRad = (double)theta * Pi / drt.rows;
      const int x = (int(d / cos(thetaRad)) + 2*n) % (2*n);
      //const int x = int(d / cos(thetaRad));
      const int y = int(n * tan(thetaRad));
      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(drt.rows-1 - theta, d) += RclockwiseT.at<int>(y, x);
    }
  }
}

cv::Mat FastHoughTransformer::FastHoughTransformFromMinusPiDiv2ToMinusPiDiv4(const cv::Mat& I) {
  cv::Mat R(n, 2*n, CV_32S, cv::Scalar(0));
  I.copyTo(R(cv::Rect(0, 0, n, n)));

  for (int i = 1; i <= log2(n); ++i) {
    cv::Mat Rnew(n, 2*n, CV_32S, cv::Scalar(0));

    const int yStep = pow(2, i);
    const int yStepPrev = pow(2, i-1);

    for (int y = 0; y < n; y += yStep) {
      for (int x = 0; x < 2*n; ++x) {
        for (int a = 0; a < yStep; ++a) {
          const int upStick = R.at<int>(y + a/2, x);
          const int downStick = R.at<int>(y + yStepPrev + a/2, (x + (a+1)/2) % (2*n));

          Rnew.at<int>(y + a, x) = upStick + downStick;
        }
      }
    }
    R = Rnew;
  }
  return R;
}


int FastHoughTransformer::GetDrtMaxDistortionAngle() const {
  int maxDistortion = 0;
  int slopeAngle = 0;

  for (int theta = 0; theta < drt.rows; ++theta) {

    int currentDistortion = 0;
    for (int d = 0; d < drt.cols; ++d) {
      const int val = drt.at<int>(theta, d);
      currentDistortion += val*val;
    }

    if (maxDistortion < currentDistortion) {
      maxDistortion = currentDistortion;
      slopeAngle = theta / ticksInDegree - DegreesInPi/2;
    }
  }

  return slopeAngle;
}


void FastHoughTransformer::WriteDrt(const char* fileName) const {
  cv::Mat drtu(drt.size(), CV_8U);

  double minVal;
  double maxVal;
  cv::Point minLoc;
  cv::Point maxLoc;
  minMaxLoc( drt, &minVal, &maxVal, &minLoc, &maxLoc );

  for (int theta = 0; theta < drt.rows; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      drtu.at<uchar>(theta, d) = int((double)(drt.at<int>(theta, d) - minVal) / (maxVal - minVal) * 255.);
      assert(0 <= drtu.at<uchar>(theta, d) && drtu.at<uchar>(theta, d) <= 255);
    }
  }

  cv::imwrite(fileName, drtu);
}