/*
 * FFmpeg.cpp
 *
 *  Created on: 2014年2月25日
 *      Author: ny
 */

#include "FFmpeg.h"

#include <ctime>

FFmpeg::FFmpeg()
{
    pCodecCtx = NULL;
    videoStream=-1;

}

FFmpeg::~FFmpeg()
{
    sws_freeContext(pSwsCtx);
}

int FFmpeg::initial(QString & url)
{
    int err;
    rtspURL=url;
    AVCodec *pCodec;
    AVDictionary* options = nullptr;

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    pFrame=av_frame_alloc();

    av_dict_set(&options, "rtsp_transport", "udp", 0);
    av_dict_set(&options, "stimeout", "5000000", 0);
    av_dict_set(&options, "buffer_size", "4096000", 0);
    av_dict_set(&options, "fflags", "nobuffer", 0);

    err = avformat_open_input(&pFormatCtx, rtspURL.toStdString().c_str(), NULL,
                              &options);
    if (err < 0)
    {
        printf("Can not open this file");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx,NULL) < 0)
    {
        printf("Unable to get stream info");
        return -1;
    }
    int i = 0;
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1)
    {
        printf("Unable to find video stream");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    width=pCodecCtx->width;
    height=pCodecCtx->height;
    avpicture_alloc(&picture, AV_PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height);
    printf("codec_id %d", pCodecCtx->codec_id);
    // if (AV_CODEC_ID_H264 == pCodecCtx->codec_id)
    //     pCodec = avcodec_find_decoder_by_name("h264_cuvid");
    // else if (AV_CODEC_ID_H265 == pCodecCtx->codec_id)
    //     pCodec = avcodec_find_decoder_by_name("hevc_cuvid");
    // else
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    pSwsCtx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width,
            height, AV_PIX_FMT_RGB24,
            SWS_BICUBIC, 0, 0, 0);

    if (pCodec == NULL)
    {
        printf("Unsupported codec");
        return -1;
    }
    printf("video size : width=%d height=%d \n", pCodecCtx->width,
           pCodecCtx->height);

    // Set low-latency options
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
    av_opt_set_int(pCodecCtx->priv_data, "threads", 1, 0);
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Unable to open codec");
        return -1;
    }
    printf("initial successfully");
    return 0;
}

int FFmpeg::h264Decodec()
{
    int frameFinished=0;
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if(packet.stream_index==videoStream)
        {
            int64_t now = 0;
            get_current_steady_time_nano(&now);
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            if (frameFinished)
            {
                // printf("***************ffmpeg decodec*******************\n");
                mutex.lock();
                int rs = sws_scale(pSwsCtx, (const uint8_t* const *) pFrame->data,
                                   pFrame->linesize, 0,
                                   height, picture.data, picture.linesize);
                mutex.unlock();
                if (rs == -1)
                {
                    printf("__________Can open to change to des imag_____________e\n");
                    return -1;
                }
                ++frame_cnt;
            }
            int64_t now1 = 0;
            get_current_steady_time_nano(&now1);
            printf("[%ld] key %d, dec %ld, pts %ld\n",
                frame_cnt, pFrame->key_frame, now1 - now, packet.pts);
        }
    }
    return 1;

}

#define IRS_S_TO_NS(seconds) ((seconds) * (1000LL * 1000LL * 1000LL))
#define __WOULD_BE_NEGATIVE(seconds, subseconds) \
  (seconds < 0 || (subseconds < 0 && seconds == 0))

bool get_current_steady_time_nano(int64_t* now) {
  // If clock_gettime is available or on OS X, use a timespec.
  struct timespec timespec_now;
  // Otherwise use clock_gettime.
#if defined(CLOCK_MONOTONIC_RAW)
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec_now);
#else   // defined(CLOCK_MONOTONIC_RAW)
  clock_gettime(CLOCK_MONOTONIC, &timespec_now);
#endif  // defined(CLOCK_MONOTONIC_RAW)
  if (__WOULD_BE_NEGATIVE(timespec_now.tv_sec, timespec_now.tv_nsec)) {
    return false;
  }
  *now = IRS_S_TO_NS(static_cast<int64_t>(timespec_now.tv_sec)) +
         timespec_now.tv_nsec;
  return true;
}
