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

void Normalize(cv::Mat& drt) {
  double minVal;
  double maxVal;
  cv::Point minLoc;
  cv::Point maxLoc;
  cv::minMaxLoc(drt, &minVal, &maxVal, &minLoc, &maxLoc);

  for (int theta = 0; theta < drt.rows; ++theta) {
    for (int d = 0; d < drt.cols; ++d) {
      const int val = int((double)(drt.at<int>(theta, d) - minVal) / (maxVal - minVal) * 255.);
      assert(0 <= val && val <= 255);
      drt.at<int>(theta, d) = val;
    }
  }
}


FastHoughTransformer::FastHoughTransformer(const cv::Mat& img):
  n(RoundToNextPowerOf2(std::max(img.cols, img.rows))),
  ticksInDegree(n/(DegreesInPi/2))
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

  const int nSqrt2 = int(n * sqrt(2.));

  cv::Mat drtNew(
      (DegreesInPi/2) * ticksInDegree,
      nSqrt2/2 + nSqrt2, // count positive and negative d
      CV_32S,
      cv::Scalar(0)
  );
  cv::swap(drt, drtNew);

  // Split into channels
  std::vector<cv::Mat> channels;
  cv::split(imgPadded, channels);

  // Build discrete Radon transformation table R(theta, d)
  for (const cv::Mat& channel : channels) {
    FastHoughTransform(channel);
  }

  Normalize(drt);
}


void FastHoughTransformer::FastHoughTransform(const cv::Mat& I) {
  const int nSqrt2 = int(n * sqrt(2.));

  cv::Mat It = I.t();
  cv::Mat Iclockwise(n, n, I.type());
  cv::rotate(I, Iclockwise, cv::ROTATE_90_CLOCKWISE);
  cv::Mat IclockwiseT = Iclockwise.t();

  // from 3pi/4 to pi/2
  cv::Mat Rclockwise = FastHoughTransformFrom0ToPiDiv4(Iclockwise);

  for (int theta = 0; theta < drt.rows/2; ++theta) {
    for (int d = -nSqrt2/2; d < n; ++d) {
      const double thetaRad = (double)theta * (Pi/2) / drt.rows;
      int x = int(round(abs(d) / cos(thetaRad)));
      if (n <= x) {
        continue;
      }
      if (d < 0) {
        x = 2*n - x;
      }
      const int y = int(round(n * tan(thetaRad)));
      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(drt.rows/2-1 - theta, nSqrt2/2 + d) += Rclockwise.at<int>(y, x);
    }
  }

  // from pi/4 to pi/2
  cv::Mat Rt = FastHoughTransformFrom0ToPiDiv4(It);

  for (int theta = 0; theta < drt.rows/2; ++theta) {
    for (int d = 0; d < nSqrt2; ++d) {
      const double thetaRad = (double)theta * (Pi/2) / drt.rows;
      //const int x = (n - int(d / cos(thetaRad)) + 2*n) % (2*n);
      const int y = int(round(n * tan(thetaRad)));

      int x = n - int(round(d / cos(thetaRad)));
      if (x < 0) {
        if (y + x < 0) {
          continue;
        }
        x = 2*n + x;
      }

      assert(0 <= x && x < 2*n && 0 <= y && y < n);
      drt.at<int>(drt.rows/2 + theta, nSqrt2/2 + d) += Rt.at<int>(y, x);
    }
  }
}

cv::Mat FastHoughTransformer::FastHoughTransformFrom0ToPiDiv4(const cv::Mat& I) {
  cv::Mat R(n, 2*n, CV_32S, cv::Scalar(0));
  I.copyTo(R(cv::Rect(0, 0, n, n)));

  cv::Mat Rnew(n, 2*n, CV_32S, cv::Scalar(0));

  for (int i = 1; i <= log2(n); ++i) {
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
    cv::swap(R, Rnew);
  }
  return R;
}


int FastHoughTransformer::GetDrtMaxDistortionAngle() const {
  double maxDispersion = 0.;
  int slopeAngle = 0;

  const int angleStep = 1;

  for (int angle = -DegreesInPi/4; angle < DegreesInPi/4; angle += angleStep) {
    uint64_t sum = 0;
    uint64_t sumSquared = 0;
    int count = 0;

    for (int theta = 0; theta < ticksInDegree * angleStep; ++theta) {
      for (int d = 0; d < drt.cols; ++d) {
        const uint64_t val = (uint64_t)drt.at<int>((angle + DegreesInPi/4)*ticksInDegree + theta, d);
        count += val == 0 ? 0 : 1;
        sum += val;
        sumSquared += val*val;
      }
    }

    if (count == 0) {
      continue;
    }

    const double mean = (double)sum/count;
    const double dispersion = (double)sumSquared/count - mean*mean;
    assert(0 <= dispersion);

    if (maxDispersion < dispersion) {
      maxDispersion = dispersion;
      slopeAngle = angle;
    }
  }

  return slopeAngle;
}


void FastHoughTransformer::WriteDrt(const char* fileName) const {
  cv::imwrite(fileName, drt);
}
