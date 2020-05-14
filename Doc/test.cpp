// test.cpp : 定义控制台应用程序的入口点。
//
#include "windows.h"
#include "time.h"
#include "stdafx.h"
#include <string>
#include  <io.h>
#include <fstream>
#include <iostream>
#include  <stdio.h>
#include  <stdlib.h>
#include <map>
//#include "jsoncpp\include\json\json.h"
//#include "curl\include\curl.h"
//#include "boost/thread/thread.hpp"
//#include "boost/thread/mutex.hpp"
//#include "HttpClient.h"
//#include "inifile.h"
//#include "dhconfigsdk.h"
//#include "dhnetsdk.h"
#include "inifile.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <list>
#include <cstdlib>
#include "HCNetSDK.h"
#include "VideoClipSdk.h"
#include "CommDef.h"
#include "TransCodeCommnDef.h"
#include "TransCode.h"
#include "md5.h"
extern "C"
{
#include <libavformat/avformat.h> 
#include <libavcodec/avcodec.h>
}


//#include "opencv2/opencv.hpp"
//#include "opencv2/core/core.hpp"
//#include "opencv2/include/highgui/highgui.hpp"
using namespace std;
//using namespace inifile;
using namespace inifile;
using std::cout;
using std::endl;

int main(void)
{
        VIDEOCLIP_Init();
        int n1 = 0;
        //获取视频长度
        //std::string strfile = "douyin2.mp4";
        //VIDEOCLIP_GetVedioDuration(&n1, strfile.c_str());

        //拼接视频去水印
        UINT64 nTaskId = 1;
        std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList;
        std::string strOutputPath = "out.mp4";
        VIDEO_CLIP_TASK_INFO_T st1;
        st1.strFilePath = "douyin1.mp4";
        st1.stTimeQuantum.nBeginTime = 0;
        st1.stTimeQuantum.nEndTime = 3;
        lstFileList.push_back(st1);
    
        VIDEO_CLIP_TASK_INFO_T st2;
        st2.strFilePath = "douyin2.mp4";
        st2.stTimeQuantum.nBeginTime = 0;
        st2.stTimeQuantum.nEndTime = 3;
        lstFileList.push_back(st2);
    
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
            }
        }
    return 0;
}
