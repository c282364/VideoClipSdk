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
* description   : 设置消息回调函数。 包括任务完成，任务错误等消息。
* input         : pCB 回调函数
*                 pUser 用户数据
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
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
* description   : 启动。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
int VideoClipScheduler::Start()
{
    m_bExitCheckThread = false;
    //启动检测线程
    m_pCheckThread = new(std::nothrow) boost::thread(boost::bind(&VideoClipScheduler::CheckTask, this));
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : Stop
* description   : 停止。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
int VideoClipScheduler::Stop()
{
    //停止检测线程
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
        //删除所有任务
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
* description   : 添加一个视频拼接任务。
* input         : nTaskId 任务id 用于上报任务状态，查询任务状态
*                 lstFileList 待拼接和去水印的视频文件列表
*                 strOutputPath 输出处理后视频文件路径【带有文件名】
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
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
            IF_LOG_E("AddOneTask 失败! , 任务id已经存在, ID:" << nTaskId);
            return VIDEOCLIP_ERRCODE_TASK_ID_EXIST;
        }

        int nSize = mapTaskList.size();
        if (nSize >= VIDEOCLIP_MAX_TASK_NUM)
        {
            IF_LOG_E("AddOneTask failed! 任务数量超过上限, 当前任务数: " << nSize << "任务上限请查看commdef.h定义VIDEOCLIP_MAX_TASK_NUM, 当前允许最大任务数量: " << VIDEOCLIP_MAX_TASK_NUM);
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
* description   : 暂停一个任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
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
* description   : 恢复一个暂停的任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
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
* description   : 删除一个任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
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
* description   : 获取任务状态
* input         : nTaskId 任务id
* output        : stTaskState 任务信息
* return        : 0表示成功 小于零失败 具体见错误码定义
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
* description   : 任务检测线程
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
                    IF_LOG_I("任务出错, 任务id: " << itor->first << "任务状态码: " << nTaskStat);
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
                    IF_LOG_I("任务完成, 任务id: " << itor->first);
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

