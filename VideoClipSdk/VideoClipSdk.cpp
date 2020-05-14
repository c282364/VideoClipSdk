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

//void main()
//{
//    VIDEOCLIP_Init();
//    int n1 = 0;
//    //std::string strfile = "douyin2.mp4";
//    //VIDEOCLIP_GetVedioDuration(&n1, strfile.c_str());
//    UINT64 nTaskId = 1;
//    std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList;
//    std::string strOutputPath = "out.mp4";
//    VIDEO_CLIP_TASK_INFO_T st1;
//    st1.strFilePath = "douyin1.mp4";
//    st1.stTimeQuantum.nBeginTime = 0;
//    st1.stTimeQuantum.nEndTime = 3;
//    lstFileList.push_back(st1);
//
//    VIDEO_CLIP_TASK_INFO_T st2;
//    st2.strFilePath = "douyin2.mp4";
//    st2.stTimeQuantum.nBeginTime = 0;
//    st2.stTimeQuantum.nEndTime = 3;
//    lstFileList.push_back(st2);
//
//    //VIDEO_CLIP_TASK_INFO_T st3;
//    //st3.strFilePath = "douyin3.mp4";
//    //st3.stTimeQuantum.nBeginTime = 0;
//    //st3.stTimeQuantum.nEndTime = 9;
//    //lstFileList.push_back(st3);
//
//    //VIDEO_CLIP_TASK_INFO_T st4;
//    //st4.strFilePath = "douyin4.mp4";
//    //st4.stTimeQuantum.nBeginTime = 0;
//    //st4.stTimeQuantum.nEndTime = 9;
//    //lstFileList.push_back(st4);
//
//    //VIDEO_CLIP_TASK_INFO_T st5;
//    //st5.strFilePath = "douyin5.mp4";
//    //st5.stTimeQuantum.nBeginTime = 0;
//    //st5.stTimeQuantum.nEndTime = 9;
//    //lstFileList.push_back(st5);
//
//    //VIDEO_CLIP_TASK_INFO_T st6;
//    //st6.strFilePath = "douyin6.mp4";
//    //st6.stTimeQuantum.nBeginTime = 0;
//    //st6.stTimeQuantum.nEndTime = 9;
//    //lstFileList.push_back(st6);
//    VIDEOCLIP_AddOneTask(nTaskId, lstFileList, strOutputPath);
//    while (true)
//    {
//        Sleep(500);
//    }
//}



#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;
//
void onMouse(int event, int x, int y, int flags, void* userdata);

Rect rect;

Mat src, roiImg, result;

void showImg();

void showImg() {

    src.copyTo(roiImg);

    rectangle(roiImg, rect, Scalar(0, 0, 255), 2);

    imshow("input", roiImg);

}

//鼠标选择矩形框

void onMouse(int event, int x, int y, int flags, void* userdata){

    switch (event)

    {

    case CV_EVENT_LBUTTONDOWN://鼠标左键按下事件

        rect.x = x;

        rect.y = y;

        rect.width = 1;

        rect.height = 1;

        break;

    case CV_EVENT_MOUSEMOVE://鼠标移动事件

        if (flags && CV_EVENT_FLAG_LBUTTON) {

            rect = Rect(Point(rect.x, rect.y), Point(x, y));

            showImg();

        }

        break;

    case EVENT_LBUTTONUP://鼠标弹起事件

        if (rect.width > 1 && rect.height > 1) {

            showImg();

        }

        break;

    default:

        break;

    }

}
static int Sal_org[800][600];
static int gray[800][600];
void SalientRegionDetectionBasedonAC(Mat &src, int MinR2, int MaxR2, int Scale){

    Mat Lab;

    cvtColor(src, Lab, CV_BGR2Lab);



    int row = src.rows, col = src.cols;



    memset(Sal_org, 0, sizeof(Sal_org));



    Mat Sal = Mat::zeros(src.size(), CV_8UC1);



    Point3_<uchar>* p;

    Point3_<uchar>* p1;

    int val;

    Mat filter;



    int max_v = 0;

    int min_v = 1 << 28;

    for (int k = 0; k < Scale; k++){

        int len = (MaxR2 - MinR2) * k / (Scale - 1) + MinR2;

        blur(Lab, filter, Size(len, len));

        for (int i = 0; i < row; i++){

            for (int j = 0; j < col; j++){

                p = Lab.ptr<Point3_<uchar> >(i, j);

                p1 = filter.ptr<Point3_<uchar> >(i, j);

                //cout<<(p->x - p1->x)*(p->x - p1->x)+ (p->y - p1->y)*(p->y-p1->y) + (p->z - p1->z)*(p->z - p1->z) <<" ";



                val = sqrt((p->x - p1->x)*(p->x - p1->x) + (p->y - p1->y)*(p->y - p1->y) + (p->z - p1->z)*(p->z - p1->z));

                Sal_org[i][j] += val;

                if (k == Scale - 1){

                    max_v = max(max_v, Sal_org[i][j]);

                    min_v = min(min_v, Sal_org[i][j]);

                }

            }

        }

    }



    cout << max_v << " " << min_v << endl;

    int X, Y;

    for (Y = 0; Y < row; Y++)

    {

        for (X = 0; X < col; X++)

        {

            Sal.at<uchar>(Y, X) = (Sal_org[Y][X] - min_v) * 255 / (max_v - min_v);        //    计算全图每个像素的显著性

            //Sal.at<uchar>(Y,X) = (Dist[gray[Y][X]])*255/(max_gray);        //    计算全图每个像素的显著性

        }

    }

    imshow("sal", Sal);

    waitKey(0);

}

void SalientRegionDetectionBasedonLC(Mat &src){

    int HistGram[256] = { 0 };

    int row = src.rows, col = src.cols;



    //int Sal_org[row][col];

    int val;

    Mat Sal = Mat::zeros(src.size(), CV_8UC1);

    Point3_<uchar>* p;

    for (int i = 0; i < row; i++){

        for (int j = 0; j < col; j++){

            p = src.ptr<Point3_<uchar> >(i, j);

            val = (p->x + (p->y) * 2 + p->z) / 4;

            HistGram[val]++;

            gray[i][j] = val;

        }

    }



    int Dist[256];

    int Y, X;

    int max_gray = 0;

    int min_gray = 1 << 28;

    for (Y = 0; Y < 256; Y++)

    {

        val = 0;

        for (X = 0; X < 256; X++)

            val += abs(Y - X) * HistGram[X];                //    论文公式（9），灰度的距离只有绝对值，这里其实可以优化速度，但计算量不大，没必要了

        Dist[Y] = val;

        max_gray = max(max_gray, val);

        min_gray = min(min_gray, val);

    }

    for (Y = 0; Y < row; Y++)
    {
        for (X = 0; X < col; X++)
        {
            Sal.at<uchar>(Y, X) = (Dist[gray[Y][X]] - min_gray) * 255 / (max_gray - min_gray);        //    计算全图每个像素的显著性
            //Sal.at<uchar>(Y,X) = (Dist[gray[Y][X]])*255/(max_gray);        //    计算全图每个像素的显著性
        }
    }
    imshow("sal", Sal);
    waitKey(0);
}

void SalientRegionDetectionBasedonFT(Mat &src){

    Mat Lab;

    cvtColor(src, Lab, CV_BGR2Lab);



    int row = src.rows, col = src.cols;





    memset(Sal_org, 0, sizeof(Sal_org));



    Point3_<uchar>* p;



    int MeanL = 0, Meana = 0, Meanb = 0;

    for (int i = 0; i < row; i++){

        for (int j = 0; j < col; j++){

            p = Lab.ptr<Point3_<uchar> >(i, j);

            MeanL += p->x;

            Meana += p->y;

            Meanb += p->z;

        }

    }

    MeanL /= (row*col);

    Meana /= (row*col);

    Meanb /= (row*col);



    GaussianBlur(Lab, Lab, Size(3, 3), 0, 0);



    Mat Sal = Mat::zeros(src.size(), CV_8UC1);



    int val;



    int max_v = 0;

    int min_v = 1 << 28;



    for (int i = 0; i < row; i++){

        for (int j = 0; j < col; j++){

            p = Lab.ptr<Point3_<uchar> >(i, j);

            val = sqrt((MeanL - p->x)*(MeanL - p->x) + (p->y - Meana)*(p->y - Meana) + (p->z - Meanb)*(p->z - Meanb));

            Sal_org[i][j] = val;

            max_v = max(max_v, val);

            min_v = min(min_v, val);

        }

    }



    cout << max_v << " " << min_v << endl;

    int X, Y;

    for (Y = 0; Y < row; Y++)

    {

        for (X = 0; X < col; X++)

        {

            Sal.at<uchar>(Y, X) = (Sal_org[Y][X] - min_v) * 255 / (max_v - min_v);        //    计算全图每个像素的显著性

            //Sal.at<uchar>(Y,X) = (Dist[gray[Y][X]])*255/(max_gray);        //    计算全图每个像素的显著性

        }

    }

    //imshow("sal", Sal);

    namedWindow("input", CV_WINDOW_AUTOSIZE);

    imshow("input", Sal);

    setMouseCallback("input", onMouse);

    //定义输出结果，结果为：GC_BGD =0（背景）,GC_FGD =1（前景）,GC_PR_BGD = 2（可能的背景）, GC_PR_FGD = 3（可能的前景）		

    Mat result = Mat::zeros(Sal.size(), CV_8UC1);

    // GrabCut 抠图

    //两个临时矩阵变量，作为算法的中间变量使用

    Mat bgModel, fgModel;

    char c = waitKey(0);

    if (c == 'g') {

        grabCut(Sal, result, rect, bgModel, fgModel, 1, GC_INIT_WITH_RECT);

        //比较result的值为可能的前景像素才输出到result中

        compare(result, GC_PR_FGD, result, CMP_EQ);

        // 产生输出图像

        Mat foreground(Sal.size(), CV_8UC3, Scalar(255, 255, 255));

        //将原图像src中的result区域拷贝到foreground中

        Sal.copyTo(foreground, result);

        imshow("output", foreground);

    }


    waitKey(0);
    return ;

}

void main()
{
    Mat src = imread("d:\\4.jpg", 1);
    //SalientRegionDetectionBasedonLC(src);
    //SalientRegionDetectionBasedonAC(src, src.rows / 8, src.rows / 2, 3);
    SalientRegionDetectionBasedonFT(src);
}


