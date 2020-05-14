#include "VideoClipScheduler.h"
#include "log.h"

VideoClipScheduler *VideoClipScheduler::m_Instance = NULL;

VideoClipScheduler::VideoClipScheduler()
{
    m_pCB = NULL;
    m_pUser = NULL;
    m_bExitCheckThread = false;
}

VideoClipScheduler::~VideoClipScheduler()
{
    Stop();
    if (NULL != m_Instance)
    {
        delete m_Instance;
        m_Instance = NULL;
    }
}

VideoClipScheduler* VideoClipScheduler::GetInstance()
{
    if (NULL == m_Instance)
    {
        m_Instance = new VideoClipScheduler;
    }
    return m_Instance;
}

/**************************************************************************
* name          : SetMsgCallback
* description   : ������Ϣ�ص������� ����������ɣ�����������Ϣ��
* input         : pCB �ص�����
*                 pUser �û�����
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::SetMsgCallback(VideoClipMsgCallback pCB, void* pUser)
{
    m_pCB = pCB;
    m_pUser = pUser;
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : Start
* description   : ������
* input         : NA
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::Start()
{
    m_bExitCheckThread = false;
    //��������߳�
    m_pCheckThread = new(std::nothrow) boost::thread(boost::bind(&VideoClipScheduler::CheckTask, this));
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : Stop
* description   : ֹͣ��
* input         : NA
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::Stop()
{
    //ֹͣ����߳�
    if (NULL != m_pCheckThread)
    {
        if (m_pCheckThread->joinable())
        {
            m_bExitCheckThread = true;
            m_pCheckThread->join();
        }
        delete m_pCheckThread;
        m_pCheckThread = NULL;
    }

    {
        boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
        //ɾ����������
        std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.begin();
        for (; itor != mapTaskList.end();)
        {
            VideoClipWork* pTask = itor->second;
            pTask->DelTask();
            delete pTask;
            itor = mapTaskList.erase(itor);
        }
    }

    return VIDEOCLIP_ERRCODE_SUCCESS;
}

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
int VideoClipScheduler::AddOneTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath)
{
    VideoClipWork* pTask = NULL;
    {
        boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
        std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.find(nTaskId);
        if (itor != mapTaskList.end())
        {
            IF_LOG_E("AddOneTask ʧ��! , ����id�Ѿ�����, ID:" << nTaskId);
            return VIDEOCLIP_ERRCODE_TASK_ID_EXIST;
        }

        int nSize = mapTaskList.size();
        if (nSize >= VIDEOCLIP_MAX_TASK_NUM)
        {
            IF_LOG_E("AddOneTask failed! ����������������, ��ǰ������: " << nSize << "����������鿴commdef.h����VIDEOCLIP_MAX_TASK_NUM, ��ǰ���������������: " << VIDEOCLIP_MAX_TASK_NUM);
            return VIDEOCLIP_ERRCODE_TOO_MANY_TASK;
        }

        pTask = new(std::nothrow) VideoClipWork;
        if (NULL == pTask)
        {
            IF_LOG_E("AddOneTask, new VideoClipWork failed! nTaskId: " << nTaskId);
            return VIDEOCLIP_ERRCODE_MALLOC_FAILED;
        }
        mapTaskList[nTaskId] = pTask;
    }
    return pTask->AddTask(nTaskId, lstFileList, strOutputPath);
}

/**************************************************************************
* name          : StopTask
* description   : ��ͣһ������
* input         : nTaskId ����id
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::StopTask(UINT64 nTaskId)
{
    VideoClipWork* pTask = NULL;
    {
        boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
        std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.find(nTaskId);
        if (itor == mapTaskList.end())
        {
            IF_LOG_E("StopTask failed! task id is not exist, nTaskId: " << nTaskId);
            return VIDEOCLIP_ERRCODE_TASK_NONEXISTENT;
        }

        pTask = itor->second;
    }
    pTask->StopTask();
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : StartTask
* description   : �ָ�һ����ͣ������
* input         : nTaskId ����id
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::StartTask(UINT64 nTaskId)
{
    VideoClipWork *pTask = NULL;
    {
        boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
        std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.find(nTaskId);
        if (itor == mapTaskList.end())
        {
            IF_LOG_E("StartTask failed! task id is not exist, nTaskId: " << nTaskId);
            return VIDEOCLIP_ERRCODE_TASK_NONEXISTENT;
        }
        pTask = itor->second;
    }
    pTask->StartTask();
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : DelTask
* description   : ɾ��һ������
* input         : nTaskId ����id
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::DelTask(UINT64 nTaskId)
{
    VideoClipWork* pTask = NULL;
    {
        boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
        std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.find(nTaskId);
        if (itor == mapTaskList.end())
        {
            IF_LOG_E("DelTask failed! task id is not exist, nTaskId: " << nTaskId);
            return VIDEOCLIP_ERRCODE_TASK_NONEXISTENT;
        }
        pTask = itor->second;
        mapTaskList.erase(itor);
    }

    pTask->DelTask();
    delete pTask;
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : GetTaskInfo
* description   : ��ȡ����״̬
* input         : nTaskId ����id
* output        : stTaskState ������Ϣ
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipScheduler::GetTaskInfo(UINT64 nTaskId, VIDEO_CLIP_TASK_STATE_T& stTaskState)
{
    boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
    std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.find(nTaskId);
    if (itor == mapTaskList.end())
    {
        IF_LOG_E("GetTaskInfo failed! task id is not exist, nTaskId: " << nTaskId);
        return VIDEOCLIP_ERRCODE_TASK_NONEXISTENT;
    }
    VideoClipWork* pTask = itor->second;
    stTaskState.nTaskState = pTask->m_nTaskStat;
    stTaskState.nCurProcessFileNo = pTask->m_nCurProcessFileNo;
    stTaskState.nCurProcessFileSec = pTask->m_nCurProcessFileSec;
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : CheckTask
* description   : �������߳�
* input         : NA
* output        : NA
* return        : NA
* remark        : NA
**************************************************************************/
void VideoClipScheduler::CheckTask()
{
    IF_LOG_I("CheckTask thread begin");
    do 
    {
        {
            boost::unique_lock<boost::mutex> mutexTaskMapLocker(m_mutexTaskMap);
            std::map<UINT64, VideoClipWork*>::iterator itor = mapTaskList.begin();
            for (; itor != mapTaskList.end();)
            {
                VideoClipWork* pTask = itor->second;
                int nTaskStat = pTask->m_nTaskStat;
                if (nTaskStat < 0)
                {
                    IF_LOG_I("�������, ����id: " << itor->first << "����״̬��: " << nTaskStat);
                    if (m_pCB)
                    {
                        m_pCB(VIDEOCLIP_TASK_ERROR, nTaskStat, itor->first, "", this);
                    }
                    pTask->DelTask();
                    delete pTask;
                    itor = mapTaskList.erase(itor);
                }
                else if (VIDEOCLIP_TASK_IS_COMPLETE == nTaskStat)
                {
                    IF_LOG_I("�������, ����id: " << itor->first);
                    if (m_pCB)
                    {
                        m_pCB(VIDEOCLIP_TASK_COMPLETE, nTaskStat, itor->first, "", this);
                    }
                    pTask->DelTask();
                    delete pTask;
                    itor = mapTaskList.erase(itor);
                }
                else
                {
                    ++itor;
                }
            }
        }
        boost::this_thread::sleep(boost::posix_time::seconds(2));
    } while (!m_bExitCheckThread);
    IF_LOG_I("CheckTask thread end");
}

