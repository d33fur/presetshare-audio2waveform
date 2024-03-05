#include <iostream>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main() {
  // Initialize FFmpeg
  av_log_set_level(AV_LOG_INFO);
  avformat_network_init();

  AVFrame* frame = av_frame_alloc();
  if (!frame) {
    std::cout << "Error allocating the frame" << std::endl;
    return 1;
  }

  // Open the input file
  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(&formatContext, "01 Push Me to the Floor.wav", nullptr, nullptr) != 0) {
    av_frame_free(&frame);
    std::cout << "Error opening the file" << std::endl;
    return 1;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Error finding the stream info" << std::endl;
    return 1;
  }

  // Find the audio stream
  AVStream* audioStream = nullptr;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audioStream = formatContext->streams[i];
      break;
    }
  }

  if (!audioStream) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not find any audio stream in the file" << std::endl;
    return 1;
  }

  // Find the decoder for the audio stream
  const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
  if (!codec) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Couldn't find a proper decoder" << std::endl;
    return 1;
  }

  // Open the decoder
  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not allocate the decoder context" << std::endl;
    return 1;
  }

  if (avcodec_parameters_to_context(codecContext, audioStream->codecpar) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not copy the codec parameters to the decoder context" << std::endl;
    return 1;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
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

  // Read the packets in a loop
  while (av_read_frame(formatContext, &packet) >= 0) {
    if (packet.stream_index == audioStream->index) {
      // Try to decode the packet into a frame
      int frameFinished = 0;
      avcodec_send_packet(codecContext, &packet);
      avcodec_receive_frame(codecContext, frame);

      // Some frames rely on multiple packets, so we have to make sure the frame is finished before
      // we can use it
      if (frameFinished) {
        // frame now has usable audio data in it. How it's stored in the frame depends on the format of
        // the audio. If it's packed audio, all the data will be in frame->data[0]. If it's in planar format,
        // the data will be in frame->data and possibly frame->extended_data. Look at frame->data, frame->nb_samples,
        // frame->linesize, and other related fields on the FFmpeg docs. I don't know how you're actually using
        // the audio data, so I won't add any junk here that might confuse you. Typically, if I want to find
        // documentation on an FFmpeg structure or function, I just type "<name> doxygen" into google (like
        // "AVFrame doxygen" for AVFrame's docs)
      }
    }

    // You *must* call av_packet_unref() after each call to av_read_frame() or else you'll leak memory
    av_packet_unref(&packet);
  }

  // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
  // is set, there can be buffered up frames that need to be flushed, so we'll do that
  if (codecContext->codec->capabilities & AV_CODEC_CAP_DELAY) {
    // Decode all the remaining frames in the buffer, until the end is reached
    int frameFinished = 0;
    while (avcodec_receive_frame(codecContext, frame) >= 0 && frameFinished) {
    }
  }

  // Clean up!
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  return 0;
}




