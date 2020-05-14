/**   @file    VideoClipScheduler.h
*    @note    All Right Reserved.
*    @brief   VideoClip ��������� ���������á�
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
#include "VideoClipWork.h"
class VideoClipScheduler
{
public:
    virtual ~VideoClipScheduler();

    static VideoClipScheduler* GetInstance();

    /**************************************************************************
    * name          : SetMsgCallback
    * description   : ������Ϣ�ص������� ����������ɣ�����������Ϣ��
    * input         : pCB �ص�����
    *                 pUser �û�����
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int SetMsgCallback(VideoClipMsgCallback pCB, void* pUser);

    /**************************************************************************
    * name          : Start
    * description   : ������
    * input         : NA
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int Start();

    /**************************************************************************
    * name          : Stop
    * description   : ֹͣ��
    * input         : NA
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int Stop();

    /**************************************************************************
    * name          : AddOneTask
    * description   : ���һ����Ƶƴ������
    * input         : nTaskId ����id �����ϱ�����״̬����ѯ����״̬
    *                 lstFileList ��ƴ�Ӻ�ȥˮӡ����Ƶ�ļ��б�
    *                 strOutputPath ����������Ƶ�ļ�·���������ļ�����
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int AddOneTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : StopTask
    * description   : ��ͣһ������
    * input         : nTaskId ����id
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int StopTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : StartTask
    * description   : �ָ�һ����ͣ������
    * input         : nTaskId ����id
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int StartTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : DelTask
    * description   : ɾ��һ������
    * input         : nTaskId ����id
    * output        : NA
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int DelTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : GetTaskInfo
    * description   : ��ȡ����״̬
    * input         : nTaskId ����id
    * output        : stTaskState ������Ϣ
    * return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
    * remark        : NA
    **************************************************************************/
    int GetTaskInfo(UINT64 nTaskId, VIDEO_CLIP_TASK_STATE_T& stTaskState);
private:
    VideoClipScheduler();

    /**************************************************************************
    * name          : CheckTask
    * description   : �������߳�
    * input         : NA
    * output        : NA
    * return        : NA
    * remark        : NA
    **************************************************************************/
    void CheckTask();

public:
    static VideoClipScheduler* m_Instance;
    std::map<UINT64, VideoClipWork*> mapTaskList;
private:
    VideoClipMsgCallback m_pCB;
    void* m_pUser;
    boost::thread *m_pCheckThread;     //�����߳̾��
    bool          m_bExitCheckThread; //�߳��˳���־
    boost::mutex  m_mutexTaskMap;        //mapTaskList������
};

