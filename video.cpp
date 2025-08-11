#include "video.h"
#include "ui_video.h"
#include <QPainter>
Video::Video(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Video)
{
    ui->setupUi(this);
    ffmpeg=NULL;
}

Video::~Video()
{
    delete ui;
}
void Video::setFFmpeg(FFmpeg *ff)
{
    ffmpeg=ff;
}

void Video::paintEvent(QPaintEvent *)
{
    static int64_t last_frame = -1;
    if(ffmpeg->picture.data!=NULL)
    {
     QPainter painter(this);
    if(ffmpeg->mutex.tryLock(1000))
    {
        int64_t now = 0;
        get_current_steady_time_nano(&now);

        QImage image=QImage(ffmpeg->picture.data[0],ffmpeg->width,ffmpeg->height,QImage::Format_RGB888);
        QPixmap  pix =  QPixmap::fromImage(image);
        painter.drawPixmap(0, 0, 1920, 1080, pix);
        update();
        ffmpeg->mutex.unlock();

        int64_t now1 = 0;
        get_current_steady_time_nano(&now1);
        if (0 && last_frame != ffmpeg->frame_cnt) {
            printf("[%ld] key %d, paint %ld\n",
                ffmpeg->frame_cnt, ffmpeg->pFrame->key_frame, now1 - now);
        }
        last_frame = ffmpeg->frame_cnt;
    }
    }
}
