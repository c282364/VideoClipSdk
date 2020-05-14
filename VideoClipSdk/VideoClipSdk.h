/**   @file    VideoClipSdk.h
 *    @note    All Right Reserved.
 *    @brief   VideoClip SDK 分为带有视频拼接以及去水印两部分功能。 
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

#ifndef CHENXY_VIDEO_CLIP_SDK_H
#define CHENXY_VIDEO_CLIP_SDK_H

#include <list>
#include <string>
#include <map>
#include "CommDef.h"

#undef SDK_DLL_DEFINE

#ifdef WIN32 //windows
#ifdef VIDEOCLIPSDK_EXPORTS
        #define SDK_DLL_DEFINE extern "C" __declspec (dllexport)
#else
        #define SDK_DLL_DEFINE extern "C" __declspec (dllimport)
#endif
#else //linux
    #define SDK_DLL_DEFINE
    #define __stdcall
#endif //#ifdef WIN32

//****************************接口定义***************************************

/**************************************************************************
* name          : VIDEOCLIP_Init
* description   : 初始化 SDK
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Init();

/**************************************************************************
* name          : VIDEOCLIP_Fini
* description   : 反初始化 SDK
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Fini();

/**************************************************************************
* name          : VIDEOCLIP_GetVedioDuration
* description   : 获取视频文件总时长
* input         : pcVedioPath 文件路径
* output        : pnSec 文件时间长度
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_GetVedioDuration(int* pnSec, const char* pcVedioPath);

/**************************************************************************
* name          : VIDEOCLIP_SetMsgCallback
* description   : 设置消息回调函数。 包括任务完成，任务错误等消息。
* input         : pCB 回调函数        
*                 pUser 用户数据
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_SetMsgCallback(VideoClipMsgCallback pCB, void* pUser);


/**************************************************************************
* name          : VIDEOCLIP_StartUp
* description   : 启动。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StartUp();

/**************************************************************************
* name          : VIDEOCLIP_Stop
* description   : 停止。
* input         : NA
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_Stop();

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
SDK_DLL_DEFINE int VIDEOCLIP_AddOneTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath);

/**************************************************************************
* name          : VIDEOCLIP_StopTask
* description   : 暂停一个任务。
* input         : nTaskId 任务id      
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StopTask(UINT64 nTaskId);

/**************************************************************************
* name          : VIDEOCLIP_StartTask
* description   : 恢复一个暂停的任务。
* input         : nTaskId 任务id      
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_StartTask(UINT64 nTaskId);

/**************************************************************************
* name          : VIDEOCLIP_DelTask
* description   : 删除一个任务。
* input         : nTaskId 任务id      
* output        : NA
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_DelTask(UINT64 nTaskId);

/**************************************************************************
* name          : VIDEOCLIP_GetTaskInfo
* description   : 获取任务状态
* input         : nTaskId 任务id      
* output        : stTaskState 任务信息
* return        : 0表示成功 小于零失败 具体见错误码定义
* remark        : NA
**************************************************************************/
SDK_DLL_DEFINE int VIDEOCLIP_GetTaskInfo(UINT64 nTaskId, VIDEO_CLIP_TASK_STATE_T& stTaskState);

#endif 