#ifndef PROCESSING_HPP
#define PROCESSING_HPP

#include <string>
#include <vector>

void readAudioData(const std::string &filename, std::vector<float> &data,
                   int &sampleRate);

void createWaveformImage(const std::vector<float> &data,
                         const std::string &outputFile);

#endif // PROCESSING_HPP
