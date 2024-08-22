#include "processing.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sndfile.h>
#include <vector>

void readAudioData(const std::string &filename, std::vector<float> &data,
                   int &sampleRate) {
  SF_INFO sfInfo;
  SNDFILE *file = sf_open(filename.c_str(), SFM_READ, &sfInfo);

  sampleRate = sfInfo.samplerate;
  int numSamples = sfInfo.frames * sfInfo.channels;
  data.resize(numSamples);

  sf_readf_float(file, data.data(), sfInfo.frames);
  sf_close(file);

  if (sfInfo.channels > 1) {
    std::vector<float> monoData(sfInfo.frames);
    for (int i = 0; i < sfInfo.frames; ++i) {
      monoData[i] = 0.0f;
      for (int c = 0; c < sfInfo.channels; ++c) {
        monoData[i] += data[i * sfInfo.channels + c];
      }
      monoData[i] /= sfInfo.channels;
    }
    data = monoData;
  }
}

void createWaveformImage(const std::vector<float> &data, const std::string &outputFile,
                         int input_width, int input_height, int output_width,
                         int output_height, bool filled, int line) {
  int width = input_width;
  int height = input_height;
  cv::Mat highResImage = cv::Mat::zeros(height, width, CV_8UC3);

  cv::Scalar bgColor(46, 43, 39);
  cv::Scalar lineColor(124, 95, 100);

  highResImage.setTo(bgColor);

  float center = height / 2.0f;
  int step = std::max(1, static_cast<int>(data.size() / width));

  for(int x = 0; x < width; ++x) {
    int start = x * step;
    int end = std::min(static_cast<int>(data.size()), (x + 1) * step);

    auto min_it = std::min_element(data.begin() + start, data.begin() + end);
    auto max_it = std::max_element(data.begin() + start, data.begin() + end);
    float min_val = *min_it;
    float max_val = *max_it;

    int min_y = static_cast<int>(center * (1 - min_val));
    int max_y = static_cast<int>(center * (1 - max_val));

    cv::line(highResImage, cv::Point(x, min_y), cv::Point(x, max_y), lineColor, line, cv::LINE_AA);

    if(filled) {
      cv::line(highResImage, cv::Point(x, min_y), cv::Point(x, center), lineColor, line, cv::LINE_AA);
    }
  }

  cv::Mat image;
  cv::resize(highResImage, image, cv::Size(output_width, output_height), 0, 0, cv::INTER_AREA);

  cv::imwrite(outputFile, image);
}
