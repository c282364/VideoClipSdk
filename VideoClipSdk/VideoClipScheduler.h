/**   @file    VideoClipScheduler.h
*    @note    All Right Reserved.
*    @brief   VideoClip 任务调度类 管理任务用。
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
#include "VideoClipWork.h"
class VideoClipScheduler
{
public:
    virtual ~VideoClipScheduler();

    static VideoClipScheduler* GetInstance();

    /**************************************************************************
    * name          : SetMsgCallback
    * description   : 设置消息回调函数。 包括任务完成，任务错误等消息。
    * input         : pCB 回调函数
    *                 pUser 用户数据
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int SetMsgCallback(VideoClipMsgCallback pCB, void* pUser);

    /**************************************************************************
    * name          : Start
    * description   : 启动。
    * input         : NA
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int Start();

    /**************************************************************************
    * name          : Stop
    * description   : 停止。
    * input         : NA
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int Stop();

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
    int AddOneTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

    /**************************************************************************
    * name          : StopTask
    * description   : 暂停一个任务。
    * input         : nTaskId 任务id
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int StopTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : StartTask
    * description   : 恢复一个暂停的任务。
    * input         : nTaskId 任务id
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int StartTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : DelTask
    * description   : 删除一个任务。
    * input         : nTaskId 任务id
    * output        : NA
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int DelTask(UINT64 nTaskId);

    /**************************************************************************
    * name          : GetTaskInfo
    * description   : 获取任务状态
    * input         : nTaskId 任务id
    * output        : stTaskState 任务信息
    * return        : 0表示成功 小于零失败 具体见错误码定义
    * remark        : NA
    **************************************************************************/
    int GetTaskInfo(UINT64 nTaskId, VIDEO_CLIP_TASK_STATE_T& stTaskState);
private:
    VideoClipScheduler();

    /**************************************************************************
    * name          : CheckTask
    * description   : 任务检测线程
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
    boost::thread *m_pCheckThread;     //任务线程句柄
    bool          m_bExitCheckThread; //线程退出标志
    boost::mutex  m_mutexTaskMap;        //mapTaskList操作锁
};

