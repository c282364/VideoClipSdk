/**   @file    VideoClipWork.h
*    @note    All Right Reserved.
*    @brief   VideoClip 任务处理类。
*
*    @author   chenxiaoyan
*    @date     2019/12/20
*
*    @note    下面的note和warning为可选项目
*    @note
*    @note    历史记录：
*    @note    V0.0.1  创建
*
*    @warning 这里填写本文件相关的警告信息
*/
#pragma once
#include "CommDef.h"
#include "boost/smart_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "Watermark.h"
extern "C"
{
    //封装格式
#include "libavformat/avformat.h"
    //解码
#include "libavcodec/avcodec.h"
    //缩放
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"

}
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>
class VideoClipWork
{
public:
    VideoClipWork();
    virtual ~VideoClipWork();

    /**************************************************************************
    * name          : AddTask
    * description   : 启动一个视频拼接任务。
    * input         : nTaskId 任务id 用于上报任务状态，查询任务状态
    *                 lstFileList 待拼接和去水印的视频文件列表
    *                 strOutputPath 输出处理后视频文件路径【带有文件名】
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int AddTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : StopTask
    * description   : 暂停一个任务。
    * input         : NA
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int StopTask();

    /**************************************************************************
    * name          : StartTask
    * description   : 恢复一个暂停的任务。
    * input         : NA
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int StartTask();

    /**************************************************************************
    * name          : DelTask
    * description   : 删除一个任务。
    * input         : NA
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int DelTask();

    ///**************************************************************************
    //* name          : GetTaskStat
    //* description   : 获取任务状态任务。
    //* input         : NA
    //* output        : nTaskStat 任务状态 1：任务处理中， 2：任务已暂停， 3：任务已完成或者任务不存在 小于零 失败
    //* return        : 0表示成功 小于零失败 具体见错误码定义
    //* remark        : NA
    //**************************************************************************/
    //int GetTaskStat(int& nTaskStat);
private:
    /**************************************************************************
    * name          : VideoClip
    * description   : 视频拼接处理函数
    * input         : lstFileList 待拼接和去水印的视频文件列表
    *                 strOutputPath 输出处理后视频文件路径【带有文件名】
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void VideoClip(std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : add_stream
    * description   : 添加输出流
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    AVStream *AddStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, AVStream *in_stream);

    /**************************************************************************
    * name          : open_video
    * description   : 打开输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int OpenVideo(AVFormatContext *oc, AVCodec *codec, AVStream *st);

    /**************************************************************************
    * name          : CreateMp4
    * description   : 创建输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int CreateMp4(const char* filename, AVStream *in_stream);

    /**************************************************************************
    * name          : CreateMp4
    * description   : 创建输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int CreateMp4(const char* filename, AVStream *video_stream, AVStream *audio_stream);

    /**************************************************************************
    * name          : WriteVideo
    * description   : 数据写入输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void WriteVideo(void* data, int nLen, AVStream *in_stream);

    /**************************************************************************
    * name          : WriteAudio
    * description   : 数据写入输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void WriteAudio(void* data, int nLen, int nDuration);

    /**************************************************************************
    * name          : CloseMp4
    * description   : 关闭输出文件
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void CloseMp4();

public:
    int m_nTaskStat; //任务状态 1：任务处理中， 2：任务已暂停， 3：任务已完成或者任务不存在 小于零错误
    int m_nCurProcessFileNo;
    int m_nCurProcessFileSec;
    boost::thread *m_pTaskThread;     //任务线程句柄
    bool          m_bExitTaskUpdate; //线程退出标志
    bool          m_bStopTaskUpdate; //线程暂停标志
    bool          m_bOpenFile;
    UINT64        m_nTaskId;
    AVFormatContext* m_pOc;
    int m_nWaitkey;
    int m_nVideoPtsInc;
    int m_nAudioPtsInc;
    int m_nOutPutVideoIndex;
    int m_nOutPutAudioIndex;
    int m_nCurFrameRate;
};

