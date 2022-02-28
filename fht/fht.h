#pragma once
#include <opencv2/opencv.hpp>


class FastHoughTransformer {
  public:
    FastHoughTransformer(const cv::Mat& I);
    int GetDrtMaxDistortionAngle() const;

    void WriteDrt(const char* fileName) const;

  private:
    const int n;
    const int ticksInDegree;
    /// discrete Radon transformation table
    cv::Mat drt;

    /// Fill R_LgN(a, x) table for -pi/2 to pi/2
    void FastHoughTransform(const cv::Mat& I);
    /// Fill R_LgN(a, x) table for -pi/2 to -pi/4 starting upper left corner
    cv::Mat FastHoughTransformFrom0ToPiDiv4(const cv::Mat& I);
};

