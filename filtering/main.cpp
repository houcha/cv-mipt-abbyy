#include <cstdio>
#include <algorithm>
#include <opencv2/opencv.hpp>


typedef void (*FilterImpl)(const cv::Mat&, cv::Mat&, int);


void Filter(const cv::Mat& inputMatrix, cv::Mat& outputMatrix, int windowRadius,
            FilterImpl filter) {

  // Add border padding
  cv::Mat inputMatrixPadded(
      inputMatrix.rows + 2*windowRadius,
      inputMatrix.cols + 2*windowRadius,
      inputMatrix.type()
  );
  cv::copyMakeBorder(
      inputMatrix,
      inputMatrixPadded,
      windowRadius,
      windowRadius,
      windowRadius,
      windowRadius,
      cv::BORDER_REPLICATE
  );

  // Split into channels
  std::vector<cv::Mat> inputChannels;
  cv::split(inputMatrixPadded, inputChannels);
  // output matrix channels
  std::vector<cv::Mat> outputChannels(inputChannels.size());

  for (int i = 0; i < inputChannels.size(); ++i) {
    filter(inputChannels[i], outputChannels[i], windowRadius);
  }

  cv::merge(outputChannels, outputMatrix);
}

void NaiveFiltering(const cv::Mat& x, cv::Mat& y, int windowRadius) {
  assert(x.type() == CV_8UC1);

  y = cv::Mat::zeros(x.rows - 2*windowRadius, x.cols - 2*windowRadius, x.type());

  // in order to save buffer allocations
  std::vector<uchar> windowContig;
  windowContig.reserve((2*windowRadius + 1) * (2*windowRadius + 1));

  for (int i = 0; i < y.rows; ++i) {
    for (int j = 0; j < y.cols; ++j) {
      // no data is copied
      cv::Mat window = x(
          cv::Range(i, i + 2*windowRadius + 1),
          cv::Range(j, j + 2*windowRadius + 1)
      );

      for (int row = 0; row < window.rows; ++row) {
        windowContig.insert(
            windowContig.end(),
            window.ptr<uchar>(row),
            window.ptr<uchar>(row) + window.cols
        );
      }
      std::sort(windowContig.begin(), windowContig.end());

      y.at<uchar>(i, j) = windowContig[windowContig.size()/2];

      windowContig.clear();
    }
  }
}

void HuangFiltering(const cv::Mat& X, cv::Mat& Y, int windowRadius) {
  
}

void FastFiltering(const cv::Mat& X, cv::Mat& Y, int windowRadius) {
}


int main(int argc, char** argv) {
  if (argc != 5) {
    printf("use: program.exe <input_image_path> <output_image_path> <filtering_type (one of: --naive, --huang, --fast, --opencv)> <window_size>\n");
    return 1;
  }

  const char* input = argv[1];
  const char* output = argv[2];
  const char* filteringType = argv[3];
  const int windowRadius = std::atoi(argv[4]);

  cv::Mat inputMatrixRgb = cv::imread(input);
  // Convert to ycrcb
  cv::Mat inputMatrixYcrcb;
  cv::cvtColor(inputMatrixRgb, inputMatrixYcrcb, cv::COLOR_BGR2YCrCb);

  cv::Mat outputMatrixYcrcb;

  if (std::strcmp(filteringType, "--naive") == 0) {
    Filter(inputMatrixYcrcb, outputMatrixYcrcb, windowRadius, &NaiveFiltering);
  } else if (std::strcmp(filteringType, "--huang") == 0) {
    Filter(inputMatrixYcrcb, outputMatrixYcrcb, windowRadius, &HuangFiltering);
  } else if (std::strcmp(filteringType, "--fast") == 0) {
    Filter(inputMatrixYcrcb, outputMatrixYcrcb, windowRadius, &FastFiltering);
  } else if (std::strcmp(filteringType, "--opencv") == 0) {
    cv::medianBlur(inputMatrixYcrcb, outputMatrixYcrcb, 2 * windowRadius + 1);
  } else {
    printf("Incorrect filtering type, expected one of: --naive, --huang, --fast, --opencv\n");
    return 2;
  }

  cv::Mat outputMatrixRgb;
  cv::cvtColor(outputMatrixYcrcb, outputMatrixRgb, cv::COLOR_YCrCb2BGR);

  cv::imwrite(output, outputMatrixRgb);

  return 0;
}
