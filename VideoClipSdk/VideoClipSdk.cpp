// VideoClipSdk.cpp : Defines the exported functions for the DLL application.
//
#include "VideoClipSdk.h"
#include <stdio.h>
#include <stdlib.h>
extern "C"
{
    //封装格式
#include "libavformat/avformat.h"
    //解码
#include "libavcodec/avcodec.h"
    //缩放
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"

}

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>
#include "log.h"
#include "VideoClipScheduler.h"

#include <iostream>

#ifdef WIN32
#include "windows.h"
#else
#include <signal.h>
#include <unistd.h>
#endif

using namespace std;
using namespace cv;

/**************************************************************************
* name          : VIDEOCLIP_Init
* description   : 初始化 SDK
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Init()
{
    // 注册所有的编解码器
    av_register_all();
    VideoClipScheduler::GetInstance();
    //avfilter_register_all();

    //avformat_network_init();

    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : VIDEOCLIP_Fini
* description   : 反初始化 SDK
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Fini()
{
    VIDEOCLIP_Stop();
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : VIDEOCLIP_GetVedioDuration
* description   : 获取视频文件总时长
* input         : pcVedioPath 文件路径
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_GetVedioDuration(int* pnSec, const char* pcVedioPath)
{
    int nSecs = -1;
    AVFormatContext* pCtx = NULL;

    if (avformat_open_input(&pCtx, pcVedioPath, NULL, NULL) != 0)
    {
        IF_LOG_E("open video file failed! file: " << pcVedioPath);
        return VIDEOCLIP_ERRCODE_OPEN_INPUT_FAILED;
    }
    if (avformat_find_stream_info(pCtx, NULL) < 0)
    {
        IF_LOG_E("find stream info failed! file: " << pcVedioPath);
        avformat_close_input(&pCtx);
        return VIDEOCLIP_ERRCODE_FIND_STREAM_INFO_FAILED;
    }

    if (pCtx->duration != AV_NOPTS_VALUE){
        //int64_t duration = pCtx->duration + 5000;
        nSecs = pCtx->duration / AV_TIME_BASE;

        //return VIDEOCLIP_ERRCODE_SUCCESS;
    }

    avformat_close_input(&pCtx);
    *pnSec = nSecs;

    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : VIDEOCLIP_SetMsgCallback
* description   : 设置消息回调函数。 包括任务完成，任务错误等消息。
* input         : pCB 回调函数
*                 pUser 用户数据
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_SetMsgCallback(VideoClipMsgCallback pCB, void* pUser)
{
    VideoClipScheduler::GetInstance()->SetMsgCallback(pCB, pUser);
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : VIDEOCLIP_StartUp
* description   : 启动。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StartUp()
{
    return VideoClipScheduler::GetInstance()->Start();
}

/**************************************************************************
* name          : VIDEOCLIP_Stop
* description   : 停止。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Stop()
{
    return VideoClipScheduler::GetInstance()->Stop();
}

/**************************************************************************
* name          : VIDEOCLIP_AddOneTask
* description   : 添加一个视频拼接任务。
* input         : nTaskId 任务id 用于上报任务状态，查询任务状态
*                 lstFileList 待拼接和去水印的视频文件列表
*                 strOutputPath 输出处理后视频文件路径【带有文件名】
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_AddOneTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath)
{
    return VideoClipScheduler::GetInstance()->AddOneTask(nTaskId, lstFileList, strOutputPath);
}

/**************************************************************************
* name          : VIDEOCLIP_StopTask
* description   : 暂停一个任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StopTask(UINT64 nTaskId)
{
    return VideoClipScheduler::GetInstance()->StopTask(nTaskId);
}

/**************************************************************************
* name          : VIDEOCLIP_StartTask
* description   : 恢复一个暂停的任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StartTask(UINT64 nTaskId)
{
    return VideoClipScheduler::GetInstance()->StartTask(nTaskId);
}

/**************************************************************************
* name          : VIDEOCLIP_DelTask
* description   : 删除一个任务。
* input         : nTaskId 任务id
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_DelTask(UINT64 nTaskId)
{
    return VideoClipScheduler::GetInstance()->DelTask(nTaskId);
}

/**************************************************************************
* name          : VIDEOCLIP_GetTaskInfo
* description   : 获取任务状态
* input         : nTaskId 任务id
* output        : stTaskState 任务信息
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_GetTaskInfo(UINT64 nTaskId, VIDEO_CLIP_TASK_STATE_T& stTaskState)
{
    return VideoClipScheduler::GetInstance()->GetTaskInfo(nTaskId, stTaskState);
}

void main()
{
    VIDEOCLIP_Init();
    int n1 = 0;
    int nLen = 0;
    //std::string strfile = "douyin2.mp4";
    //VIDEOCLIP_GetVedioDuration(&n1, strfile.c_str());
    UINT64 nTaskId = 1;
    std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList;
    std::string strOutputPath = "c:\\douyin\\out.mp4";
    VIDEO_CLIP_TASK_INFO_T st1;
    st1.strFilePath = "c:\\douyin\\douyin1.mp4";
    st1.stTimeQuantum.nBeginTime = 0;
    VIDEOCLIP_GetVedioDuration(&nLen, st1.strFilePath.c_str());
    st1.stTimeQuantum.nEndTime = nLen;
    //st1.stTimeQuantum.nEndTime = 3;
    lstFileList.push_back(st1);

    VIDEO_CLIP_TASK_INFO_T st2;
    st2.strFilePath = "c:\\douyin\\douyin2.mp4";
    st2.stTimeQuantum.nBeginTime = 0;
    VIDEOCLIP_GetVedioDuration(&nLen, st2.strFilePath.c_str());
    st2.stTimeQuantum.nEndTime = nLen;
    lstFileList.push_back(st2);

    VIDEO_CLIP_TASK_INFO_T st3;
    st3.strFilePath = "c:\\douyin\\douyin3.mp4";
    st3.enOsdType = OSD_TYPE_DOUYIN;
    st3.stTimeQuantum.nBeginTime = 0;
    VIDEOCLIP_GetVedioDuration(&nLen, st3.strFilePath.c_str());
    st2.stTimeQuantum.nEndTime = nLen;
    lstFileList.push_back(st3);

    //VIDEO_CLIP_TASK_INFO_T st3;
    //st3.strFilePath = "douyin3.mp4";
    //st3.stTimeQuantum.nBeginTime = 0;
    //st3.stTimeQuantum.nEndTime = 9;
    //lstFileList.push_back(st3);

    //VIDEO_CLIP_TASK_INFO_T st4;
    //st4.strFilePath = "douyin4.mp4";
    //st4.stTimeQuantum.nBeginTime = 0;
    //st4.stTimeQuantum.nEndTime = 9;
    //lstFileList.push_back(st4);

    //VIDEO_CLIP_TASK_INFO_T st5;
    //st5.strFilePath = "douyin5.mp4";
    //st5.stTimeQuantum.nBeginTime = 0;
    //st5.stTimeQuantum.nEndTime = 9;
    //lstFileList.push_back(st5);

    //VIDEO_CLIP_TASK_INFO_T st6;
    //st6.strFilePath = "douyin6.mp4";
    //st6.stTimeQuantum.nBeginTime = 0;
    //st6.stTimeQuantum.nEndTime = 9;
    //lstFileList.push_back(st6);
    VIDEOCLIP_AddOneTask(nTaskId, lstFileList, strOutputPath);
    VIDEO_CLIP_TASK_STATE_T st12;
    while (true)
    {
        Sleep(500);
        //获取处理进度
        VIDEOCLIP_GetTaskInfo(nTaskId, st12);
        if (st12.nTaskState == VIDEOCLIP_TASK_IN_PROCESS)
        {
            printf("FILE no: %d, sec:%d\n", st12.nCurProcessFileNo, st12.nCurProcessFileSec);
            //CString strTmp;
            //strTmp.Format(_T("正在处理第%d个文件，已处理%d秒视频\n"), st12.nCurProcessFileNo, st12.nCurProcessFileSec);
            //SetWindowText(strTmp);
        }
    }
}
