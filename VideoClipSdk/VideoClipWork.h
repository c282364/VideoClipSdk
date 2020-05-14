/**   @file    VideoClipWork.h
*    @note    All Right Reserved.
*    @brief   VideoClip �������ࡣ
*
*    @author   chenxiaoyan
*    @date     2019/12/20
*
*    @note    �����note��warningΪ��ѡ��Ŀ
*    @note
*    @note    ��ʷ��¼��
*    @note    V0.0.1  ����
*
*    @warning ������д���ļ���صľ�����Ϣ
*/
#pragma once
#include "CommDef.h"
#include "boost/smart_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "Watermark.h"
extern "C"
{
    //��װ��ʽ
#include "libavformat/avformat.h"
    //����
#include "libavcodec/avcodec.h"
    //����
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
    * description   : ����һ����Ƶƴ������
    * input         : nTaskId ����id �����ϱ�����״̬����ѯ����״̬
    *                 lstFileList ��ƴ�Ӻ�ȥˮӡ����Ƶ�ļ��б�
    *                 strOutputPath ����������Ƶ�ļ�·���������ļ�����
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int AddTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : StopTask
    * description   : ��ͣһ������
    * input         : NA
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int StopTask();

    /**************************************************************************
    * name          : StartTask
    * description   : �ָ�һ����ͣ������
    * input         : NA
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int StartTask();

    /**************************************************************************
    * name          : DelTask
    * description   : ɾ��һ������
    * input         : NA
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int DelTask();

    ///**************************************************************************
    //* name          : GetTaskStat
    //* description   : ��ȡ����״̬����
    //* input         : NA
    //* output        : nTaskStat ����״̬ 1���������У� 2����������ͣ�� 3����������ɻ������񲻴��� С���� ʧ��
    //* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    //* remark        : NA
    //**************************************************************************/
    //int GetTaskStat(int& nTaskStat);
private:
    /**************************************************************************
    * name          : VideoClip
    * description   : ��Ƶƴ�Ӵ�����
    * input         : lstFileList ��ƴ�Ӻ�ȥˮӡ����Ƶ�ļ��б�
    *                 strOutputPath ����������Ƶ�ļ�·���������ļ�����
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void VideoClip(std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : add_stream
    * description   : ��������
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    AVStream *AddStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, AVStream *in_stream);

    /**************************************************************************
    * name          : open_video
    * description   : ������ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int OpenVideo(AVFormatContext *oc, AVCodec *codec, AVStream *st);

    /**************************************************************************
    * name          : CreateMp4
    * description   : ��������ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int CreateMp4(const char* filename, AVStream *in_stream);

    /**************************************************************************
    * name          : CreateMp4
    * description   : ��������ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    int CreateMp4(const char* filename, AVStream *video_stream, AVStream *audio_stream);

    /**************************************************************************
    * name          : WriteVideo
    * description   : ����д������ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void WriteVideo(void* data, int nLen, AVStream *in_stream);

    /**************************************************************************
    * name          : WriteAudio
    * description   : ����д������ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void WriteAudio(void* data, int nLen, int nDuration);

    /**************************************************************************
    * name          : CloseMp4
    * description   : �ر�����ļ�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void CloseMp4();

public:
    int m_nTaskStat; //����״̬ 1���������У� 2����������ͣ�� 3����������ɻ������񲻴��� С�������
    int m_nCurProcessFileNo;
    int m_nCurProcessFileSec;
    boost::thread *m_pTaskThread;     //�����߳̾��
    bool          m_bExitTaskUpdate; //�߳��˳���־
    bool          m_bStopTaskUpdate; //�߳���ͣ��־
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

