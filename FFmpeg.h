/*
 * FFmpeg.h
 *
 *  Created on: 2014年2月25日
 *      Author: ny
 */

#ifndef FFMPEG_H_
#define FFMPEG_H_
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
#include <QReadWriteLock>
#include <QMutex>
#include <QString>
class FFmpeg
{
public:
	FFmpeg();
    int initial(QString & url);
    int h264Decodec();
	virtual ~FFmpeg();
    friend class Video;
private:
    AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVFrame *pFrame;
    AVPacket packet;
    AVPicture  picture;
    SwsContext * pSwsCtx;
    int videoStream;
    int width;
    int height;
    QMutex mutex;
    QString rtspURL;

    int64_t frame_cnt = 0;
};

bool get_current_steady_time_nano(int64_t* now);

#endif /* FFMPEG_H_ */
