/**   @file    CommDef.h 
*    @note    All Right Reserved.
*    @brief   公共定义文件。
*    @author   chenxiaoyan
*    @date     2018/05/30
*
*    @note    下面的note和warning为可选项目
*    @note
*    @note    历史记录：
*    @note    V0.0.1  创建
*
*    @warning 这里填写本文件相关的警告信息
*/

#pragma once
#include <list>
#include <string>
#include <map>

#ifdef WIN32 //windows
#ifdef VIDEOCLIPSDK_EXPORTS
#define SDK_DLL_DEFINE extern "C" __declspec (dllexport)
#else
#define SDK_DLL_DEFINE extern "C" __declspec (dllimport)
#endif

typedef  unsigned __int64   UINT64;
typedef  signed   __int64   INT64;
#else //linux
#define SDK_DLL_DEFINE
#define __stdcall

typedef  unsigned long long UINT64;
typedef  signed long long   INT64;
#endif //#ifdef WIN32

//****************************错误码定义***************************************
static const int VIDEOCLIP_ERRCODE_SUCCESS                 = 0; //成功
static const int VIDEOCLIP_ERRCODE_INIT_FAILED             = -1; //初始华失败
static const int VIDEOCLIP_ERRCODE_MALLOC_FAILED           = -2; //分配内存失败。
static const int VIDEOCLIP_ERRCODE_BAD_PARA                = -3; //参数有问题
static const int VIDEOCLIP_ERRCODE_NONSUPPORT              = -4; //当前版本不支持
static const int VIDEOCLIP_ERRCODE_UNINITIALIZED           = -5; //未初始化
static const int VIDEOCLIP_ERRCODE_NOT_CONFIGURES          = -6; //未配置参数
static const int VIDEOCLIP_ERRCODE_NOT_SET_CALLBACK        = -7; //没有设置回调函数
static const int VIDEOCLIP_ERRCODE_TASK_NONEXISTENT        = -8; //任务不存在
static const int VIDEOCLIP_ERRCODE_OPEN_INPUT_FAILED       = -9; //打开视频文件失败
static const int VIDEOCLIP_ERRCODE_FIND_STREAM_INFO_FAILED = -10; //解析视频文件时查找视频信息失败
static const int VIDEOCLIP_ERRCODE_TASK_ID_EXIST           = -11; //添加任务时 任务id已存在
static const int VIDEOCLIP_ERRCODE_TOO_MANY_TASK           = -12; //任务数量太多 请等下再添加 任务数量上限 VIDEOCLIP_MAX_TASK_NUM
static const int VIDEOCLIP_ERRCODE_NO_VIDEO                = -13; //文件没有视频帧
static const int VIDEOCLIP_ERRCODE_FIND_CODEC_FAILED       = -14; //不支持的编码格式 
static const int VIDEOCLIP_ERRCODE_OPEN_CODEC_FAILED       = -15; //当前文件视频编码格式解码器不存在
static const int VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED        = -16; //打开输出文件错误
static const int VIDEOCLIP_ERRCODE_AVFORMAT_NEW_STREAM_FAILED = -17; //创建输出流失败
static const int VIDEOCLIP_ERRCODE_OPEN_WATERMARK_FAILED   = -18; //打开水印图失败
static const int VIDEOCLIP_ERRCODE_ANALYSE_WATERMARK_FAILED = -19; //解析水印失败
static const int VIDEOCLIP_ERRCODE_CPOTPUT_OPEN_VIDEO_AVCODEC_FAILED = -20; //创建输出文件打开视频解码器失败 一般是h264
static const int VIDEOCLIP_ERRCODE_CPOTPUT_OPEN_AUDIO_AVCODEC_FAILED = -21; //创建输出文件打开音频解码器失败 一般是aac

//****************************消息定义***************************************


//****************************常量定义***************************************
static const int VIDEOCLIP_MAX_TASK_NUM = 5; //任务数量最高值

//任务状态
static const int VIDEOCLIP_TASK_IN_PROCESS = 1; //处理中
static const int VIDEOCLIP_TASK_SUSPEND = 2; //任务已暂停
static const int VIDEOCLIP_TASK_IS_COMPLETE = 3; //任务已完成
static const int VIDEOCLIP_TASK_IS_DELETE = 4; //任务被删除


//****************************结构定义***************************************
//回调消息类型
typedef enum {
    VIDEOCLIP_TASK_COMPLETE = 0x0, // 任务完成
    VIDEOCLIP_TASK_ERROR = 0x1, // 任务出错 具体错误查看错误码定义
} E_VIDEOCLIPSDK_MSG_TYPE;

//视频文件封装类型
typedef enum {
    VIDEO_MP4 = 0x0, // MP4文件
} E_VIDEOCLIPSDK_VIDEO_FORMAT;

//水印类型
typedef enum {
    OSD_TYPE_NON = 0x0, // 无水印
    OSD_TYPE_DOUYIN = 0x1, // 抖音水印
} E_OSD_TYPE;


//任务状态信息
typedef struct _video_clip_task_state_ {
    int nTaskState;          //任务状态 具体请查看commdef.h  任务状态部分
    int nCurProcessFileNo;   //当前正在处理的文件编号 总共n个文件当前正在处理第m个
    int nCurProcessFileSec; //当前正在处理文件处理到第几秒

    _video_clip_task_state_()
        : nTaskState(VIDEOCLIP_TASK_IN_PROCESS)
        , nCurProcessFileNo(1)
        , nCurProcessFileSec(0)
    {}
}VIDEO_CLIP_TASK_STATE_T, *LPVIDEO_CLIP_TASK_STATE_T;

//时间段信息
typedef struct _video_clip_time_quantum_ {
    int nBeginTime; //开始时间 单位 秒
    int nEndTime;   //结束时间 单位 秒

    _video_clip_time_quantum_()
        : nBeginTime(0)
        , nEndTime(0)
    {}
}VIDEO_CLIP_TIME_QUANTUM_T, *LPVIDEO_CLIP_TIME_QUANTUM_T;

//拼接任务 单个文件的设置参数
typedef struct _video_clip_task_info_ {
    std::string 						 strFilePath;   //文件路径
    VIDEO_CLIP_TIME_QUANTUM_T            stTimeQuantum; //截取时间段信息 如果不需要截取 全部填零
    E_VIDEOCLIPSDK_VIDEO_FORMAT          enVideoFormat; //视频封装格式
    E_OSD_TYPE						     enOsdType; //要去除的水印类型

    _video_clip_task_info_()
        : strFilePath("")
        , enVideoFormat(VIDEO_MP4)
        , enOsdType(OSD_TYPE_DOUYIN)
    {}
}VIDEO_CLIP_TASK_INFO_T, *LPVIDEO_CLIP_TASK_INFO_T;

//****************************回调定义***************************************
//回调函数不能阻塞 不然影响正常添加任务
typedef void(*VideoClipMsgCallback)(E_VIDEOCLIPSDK_MSG_TYPE enMsgType, int nErrorCode, int nTaskId, const char* pcMsgContent, void *pUserData);
