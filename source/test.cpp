#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cout << "Usage: ./program_name file_path" << std::endl;
    return 1;
  }

  std::string filePath = argv[1];

  av_log_set_level(AV_LOG_INFO);
  avformat_network_init();

  AVFrame* frame = av_frame_alloc();
  if(!frame) {
    std::cout << "Error allocating the frame" << std::endl;
    return 1;
  }

  AVFormatContext* formatContext = nullptr;
  if(avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) != 0) {
    av_frame_free(&frame);
    std::cout << "Error opening the file" << std::endl;
    return 1;
  }

  if(avformat_find_stream_info(formatContext, nullptr) < 0) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Error finding the stream info" << std::endl;
    return 1;
  }

  AVStream* audioStream = nullptr;
  for(unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audioStream = formatContext->streams[i];
      break;
    }
  }

  if(!audioStream) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not find any audio stream in the file" << std::endl;
    return 1;
  }

  const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
  if(!codec) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Couldn't find a proper decoder" << std::endl;
    return 1;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if(!codecContext) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not allocate the decoder context" << std::endl;
    return 1;
  }

  if(avcodec_parameters_to_context(codecContext, audioStream->codecpar) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not copy the codec parameters to the decoder context" << std::endl;
    return 1;
  }

  if(avcodec_open2(codecContext, codec, nullptr) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not open the decoder" << std::endl;
    return 1;
  }

  std::cout << "This stream has " << codecContext->channels << " channels and a sample rate of "
            << codecContext->sample_rate << "Hz" << std::endl;
  std::cout << "The data is in the format " << av_get_sample_fmt_name(codecContext->sample_fmt) << std::endl;

  AVPacket packet;

  while(av_read_frame(formatContext, &packet) >= 0) {
    if(packet.stream_index == audioStream->index) {
      int frameFinished = avcodec_receive_frame(codecContext, frame);

      avcodec_send_packet(codecContext, &packet);
      avcodec_receive_frame(codecContext, frame);

      // Some frames rely on multiple packets, so we have to make sure the frame is finished before
      // we can use it
      if(frameFinished) {
        // frame now has usable audio data in it. How it's stored in the frame depends on the format of
        // the audio. If it's packed audio, all the data will be in frame->data[0]. If it's in planar format,
        // the data will be in frame->data and possibly frame->extended_data. Look at frame->data, frame->nb_samples,
        // frame->linesize, and other related fields on the FFmpeg docs. I don't know how you're actually using
        // the audio data, so I won't add any junk here that might confuse you. Typically, if I want to find
        // documentation on an FFmpeg structure or function, I just type "<name> doxygen" into google (like
        // "AVFrame doxygen" for AVFrame's docs)
          // Получить данные аудио из фрейма
        const uint8_t* data = frame->data[0];
        int numSamples = frame->nb_samples;

        std::vector<float> samples(numSamples);
        for(int i = 0; i < numSamples; ++i) {
          int32_t sample = *(const int32_t*)(data + i * 4);
          samples[i] = sample / (float)INT32_MAX;
        }

        static const int imageWidth = 5024;
        static const int imageHeight = 128;
        static cv::Mat image(imageHeight, imageWidth, CV_8UC3, cv::Scalar(46, 43, 39));

        static int offset = 0;
        for(int i = 0; i < numSamples; ++i) {
          int x = (offset + i) * imageWidth / codecContext->sample_rate;
          int y = imageHeight / 2 + samples[i] * imageHeight / 2;
          for (int j = 1; j <= 1; ++j) {
            if (y + j >= 0 && y + j < imageHeight) {
              image.at<cv::Vec3b>(y + j, x) = cv::Vec3b(96, 78, 77);
            }
          }
        }
        offset += numSamples;

        std::string filename = "aaaaaa_.jpg";
        cv::imwrite(filename, image);
      }
    }

    av_packet_unref(&packet);
  }

  // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
  // is set, there can be buffered up frames that need to be flushed, so we'll do that
  if(codecContext->codec->capabilities & AV_CODEC_CAP_DELAY) {
    // Decode all the remaining frames in the buffer, until the end is reached
    int frameFinished = 0;
    while (avcodec_receive_frame(codecContext, frame) >= 0 && frameFinished) {
    }
  }

  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  return 0;
}
