#include "Watermark.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>
#include "log.h"
using namespace std;
using namespace cv;

Watermark::Watermark()
{
    m_dResizeW = 1.0;
    m_dResizeH = 1.0;
}


Watermark::~Watermark()
{
}

int Watermark::AnalyseDouyin(std::string strFile)
{
    //检测用户信息水印 并保存 按比列拉伸缩放 水印src 544*960 -> dst
    //水印位置变化为准 douyinzhanghao.PNG位置变化 查看变化范围以配置中坐标为准查找两张图相同部分 水印变化以后到右下角。离右下角距离固定
    VideoCapture capture;
    Mat frame;
    frame = capture.open(strFile.c_str());
    if (!capture.isOpened())
    {
        IF_LOG_E("AnalyseDouyin, open input failed file: " << strFile);
        return VIDEOCLIP_ERRCODE_OPEN_INPUT_FAILED;
    }
    
    Mat matAccountTemp = imread("douyinzhanghao.PNG");
    if (NULL == matAccountTemp.data)
    {
        IF_LOG_E("open douyinzhanghao.png failed");
        return VIDEOCLIP_ERRCODE_OPEN_WATERMARK_FAILED;
    }
    m_matLogo = imread("douyintubiao.PNG");
    if (NULL == m_matLogo.data)
    {
        IF_LOG_E("open douyintubiao.png failed");
        return VIDEOCLIP_ERRCODE_OPEN_WATERMARK_FAILED;
    }
    m_matWords = imread("douyinziti.PNG");
    if (NULL == m_matWords.data)
    {
        IF_LOG_E("open douyinziti.png failed");
        return VIDEOCLIP_ERRCODE_OPEN_WATERMARK_FAILED;
    }
    //m_matMaskWord = cv::imread("douyinziti.PNG", 0);
    //m_matMaskLogo = cv::imread("douyintubiao.PNG", 0);
    threshold(m_matWords, m_matWords, 150, 255, CV_THRESH_BINARY);
    threshold(m_matLogo, m_matLogo, 150, 255, CV_THRESH_BINARY);
    cvtColor(m_matWords, m_matWords, COLOR_BGR2GRAY);
    cvtColor(m_matLogo, m_matLogo, COLOR_BGR2GRAY);
    cvtColor(matAccountTemp, matAccountTemp, COLOR_BGR2GRAY);
    threshold(matAccountTemp, matAccountTemp, 150, 255, CV_THRESH_BINARY);
    double minVal1, maxVal1;
    cv::Point minLoc1, maxLoc1;
    double minVal2, maxVal2;
    cv::Point minLoc2, maxLoc2;
    double minVal3, maxVal3;
    cv::Point minLoc3, maxLoc3;
    cv::Mat image_matched2 = Mat::zeros(Size(frame.cols, frame.rows), CV_8UC1);//CV_32FC3
    maxLoc1.x = 0;
    maxLoc1.y = 0;
    cv::Point maxLocCur;
    maxLocCur.x = 0;
    maxLocCur.y = 0;
    int nFrameCount = 0;
    bool bFindAcc = false;
    while (capture.read(frame))
    {
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        Mat Frame1;
        Mat Frame2;
        Mat Frame3;
        //应对背景白色
        threshold(frame, Frame1, 235, 255, CV_THRESH_BINARY);
        ////应对背景不是很白
        threshold(frame, Frame2, 215, 255, CV_THRESH_BINARY);
        //作为参照 如果这个更准确则用这个 应对比较模糊的水印
        threshold(frame, Frame3, 200, 255, CV_THRESH_BINARY);
        //imshow("Frame1", Frame1);
        //imshow("Frame2", Frame2);
        //imshow("Frame3", Frame3);
        int nthreshold = 235;
        cv::matchTemplate(Frame1, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched2, &minVal1, &maxVal1, &minLoc1, &maxLoc1);

        cv::matchTemplate(Frame2, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched2, &minVal2, &maxVal2, &minLoc2, &maxLoc2);

        cv::matchTemplate(Frame3, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched2, &minVal3, &maxVal3, &minLoc3, &maxLoc3);

        double nMaxTemplate = maxVal1;
        cv::Point maxLocTemplate = maxLoc1;
        if (maxVal2 > maxVal1)
        {
            nthreshold = 215;
            nMaxTemplate = maxVal2;
            maxLocTemplate = maxLoc2;
        }
        if (maxVal3 > nMaxTemplate)
        {
            nthreshold = 200;
            nMaxTemplate = maxVal3;
            maxLocTemplate = maxLoc3;
        }

        if (0 == nFrameCount)
        {
            //Mat lastFrame;
            if (frame.cols != 544 || frame.rows != 960)
            {
                m_dResizeW = (double)((double)frame.cols / (double)544);
                m_dResizeH = (double)((double)frame.rows / (double)960);
                int nWidth = (int)((double)matAccountTemp.cols * m_dResizeW);
                int nHeight = (int)((double)matAccountTemp.rows * m_dResizeH);
                cv::resize(matAccountTemp, matAccountTemp, cv::Size(nWidth, nHeight), (0, 0), (0, 0), cv::INTER_LINEAR);
                nWidth = (int)((double)m_matLogo.cols * m_dResizeW);
                nHeight = (int)((double)m_matLogo.rows * m_dResizeH);
                cv::resize(m_matLogo, m_matLogo, cv::Size(nWidth, nHeight), (0, 0), (0, 0), cv::INTER_LINEAR);
                nWidth = (int)((double)m_matWords.cols * m_dResizeW);
                nHeight = (int)((double)m_matWords.rows * m_dResizeH);
                cv::resize(m_matWords, m_matWords, cv::Size(nWidth, nHeight), (0, 0), (0, 0), cv::INTER_LINEAR);
            }
            cv::matchTemplate(Frame1, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
            cv::minMaxLoc(image_matched2, &minVal1, &maxVal1, &minLoc1, &maxLoc1);

            cv::matchTemplate(Frame2, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
            cv::minMaxLoc(image_matched2, &minVal2, &maxVal2, &minLoc2, &maxLoc2);

            cv::matchTemplate(Frame3, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
            cv::minMaxLoc(image_matched2, &minVal3, &maxVal3, &minLoc3, &maxLoc3);

            nMaxTemplate = maxVal1;
            maxLocTemplate = maxLoc1;
            if (maxVal2 > maxVal1)
            {
                nthreshold = 215;
                nMaxTemplate = maxVal2;
                maxLocTemplate = maxLoc2;
            }
            if (maxVal3 > nMaxTemplate)
            {
                nthreshold = 200;
                nMaxTemplate = maxVal3;
                maxLocTemplate = maxLoc3;
            }
            maxLocCur = maxLocTemplate;
            Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
            erode(m_matLogo, m_matTemplate1Erode, kernel);
        }

        nFrameCount++;
        if (maxLocCur.x != maxLocTemplate.x && maxLocCur.y != maxLocTemplate.y)
        {
            double d1 = m_dResizeW * (double)12;
            bFindAcc = true;
            Rect rect(maxLocTemplate.x, maxLocTemplate.y, frame.cols - maxLocTemplate.x - (int)d1, matAccountTemp.rows);
            Mat image_roi_cur = frame(rect);
            //白色 如果不够白的话用200就可以
            //double minVal3, maxVal3;
            //cv::Point minLoc3, maxLoc3;
            //if (235 == nthreshold)
            //{
            //    cv::matchTemplate(Frame2, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
            //    cv::minMaxLoc(image_matched2, &minVal3, &maxVal3, &minLoc3, &maxLoc3);
            //    if (maxVal3 > maxVal1)
            //    {
            //        nthreshold = 200;
            //    }
            //}
            //else if (200 == nthreshold)
            //{
            //    cv::matchTemplate(Frame3, matAccountTemp, image_matched2, cv::TM_CCOEFF_NORMED);
            //    cv::minMaxLoc(image_matched2, &minVal3, &maxVal3, &minLoc3, &maxLoc3);
            //    if (maxVal3 > maxVal1)
            //    {
            //        nthreshold = 150;
            //    }
            //}
            threshold(image_roi_cur, image_roi_cur, nthreshold, 255, CV_THRESH_BINARY);
            image_roi_cur.copyTo(m_matAccount);
            //Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
            //dilate(m_matAccount, m_matAccount, element);
            m_matAccount.copyTo(m_matMaskAccount);

            Rect rectTemp(0, 0, matAccountTemp.cols, matAccountTemp.rows);
            Mat image_roi_temp = m_matAccount(rectTemp);
            matAccountTemp.copyTo(image_roi_temp);
            //test
            //imshow("matAccountTemp", matAccountTemp);
            //imshow("m_matAccount", m_matAccount);
            //printf("nthreshold is %d\n", nthreshold);
            //waitKey();
            break;
        }
    }
    capture.release();
    if (!bFindAcc)
    {
        return VIDEOCLIP_ERRCODE_ANALYSE_WATERMARK_FAILED;
    }
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

int Watermark::ProcessDouyin(cv::Mat &matImageSource)
{
    cv::cvtColor(matImageSource, matImageSource, CV_YUV2BGR_I420);
    cv::Mat image_matched = Mat::zeros(Size(matImageSource.cols, matImageSource.rows), CV_8UC1);//CV_32FC3
    cv::Mat image_matched1 = Mat::zeros(Size(matImageSource.cols, matImageSource.rows), CV_8UC1);
    cv::Mat image_matched2 = Mat::zeros(Size(matImageSource.cols, matImageSource.rows), CV_8UC1);
    //模板匹配
    Mat image_source2;
    matImageSource.copyTo(image_source2);
    Mat image_source3;
    matImageSource.copyTo(image_source3);
    cvtColor(image_source2, image_source2, COLOR_BGR2GRAY);
    cvtColor(image_source3, image_source3, COLOR_BGR2GRAY);
    //应对背景不是很白
    threshold(image_source2, image_source2, 200, 255, CV_THRESH_BINARY);
    //应对背景白色
    threshold(image_source3, image_source3, 230, 255, CV_THRESH_BINARY);

    cv::matchTemplate(image_source2, m_matWords, image_matched, cv::TM_CCOEFF_NORMED);
    cv::matchTemplate(image_source2, m_matTemplate1Erode, image_matched1, cv::TM_CCOEFF_NORMED);
    cv::matchTemplate(image_source2, m_matAccount, image_matched2, cv::TM_CCOEFF_NORMED);

    double minVal, maxVal;
    double minVal1, maxVal1;
    double minVal2, maxVal2;
    cv::Point minLoc, maxLoc;
    cv::Point minLoc1, maxLoc1;
    cv::Point minLoc2, maxLoc2;

    //寻找最佳匹配位置
    cv::minMaxLoc(image_matched, &minVal, &maxVal, &minLoc, &maxLoc);
    cv::minMaxLoc(image_matched1, &minVal1, &maxVal1, &minLoc1, &maxLoc1);
    cv::minMaxLoc(image_matched2, &minVal2, &maxVal2, &minLoc2, &maxLoc2);
    if (maxVal < 0.5)
    {
        cv::matchTemplate(image_source3, m_matWords, image_matched, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched, &minVal, &maxVal, &minLoc, &maxLoc);
    }

    if (maxVal1 < 0.6)
    {
        cv::matchTemplate(image_source3, m_matTemplate1Erode, image_matched1, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched1, &minVal1, &maxVal1, &minLoc1, &maxLoc1);
    }

    if (maxVal2 < 0.5)
    {
        cv::matchTemplate(image_source3, m_matAccount, image_matched2, cv::TM_CCOEFF_NORMED);
        cv::minMaxLoc(image_matched2, &minVal2, &maxVal2, &minLoc2, &maxLoc2);
    }

    cv::Mat image_dst = cv::Mat::zeros(matImageSource.rows, matImageSource.cols, CV_8UC1);
    Mat imageROI = image_dst(cv::Rect(maxLoc.x, maxLoc.y, m_matWords.cols, m_matWords.rows));
    m_matWords.copyTo(imageROI, m_matMaskWord);

    cv::Mat matLogoCur;
    Mat element1 = getStructuringElement(MORPH_RECT, Size(6, 6));
    dilate(m_matLogo, matLogoCur, element1);
    Mat imageROI1 = image_dst(cv::Rect(maxLoc1.x, maxLoc1.y, matLogoCur.cols, matLogoCur.rows));
    matLogoCur.copyTo(imageROI1, m_matMaskLogo);

    if (maxLoc1.y < matImageSource.rows / 2)
    {
        int nDiffX = maxLoc2.x - 13 * m_dResizeW;
        int nDiffY = maxLoc2.y - 66 * m_dResizeW;
        if (nDiffX > 50 * m_dResizeW || nDiffY > 50 * m_dResizeW)
        {
            maxLoc2.x = 13 * m_dResizeW;
            maxLoc2.y = 66 * m_dResizeW;
        }
    }
    else
    {
        double d1 = m_dResizeW * (double)12;
        int nDiffX = image_dst.cols - (int)d1 - m_matAccount.cols - maxLoc2.x;
        int nDiffY = 926 * m_dResizeW - maxLoc2.y;
        if (nDiffX > 50 * m_dResizeW || nDiffY > 50 * m_dResizeW)
        {
            maxLoc2.x = image_dst.cols - (int)d1 - m_matAccount.cols;
            maxLoc2.y = 926 * m_dResizeW;
        }

    }
    Mat imageROI2 = image_dst(cv::Rect(maxLoc2.x - 2, maxLoc2.y - 2, m_matMaskAccount.cols + 2, m_matMaskAccount.rows + 2));
    imageROI2.setTo(cv::Scalar(255, 255, 255));
    //cv::Mat matAccountCur;
    //Mat element2 = getStructuringElement(MORPH_RECT, Size(5, 5));
    //dilate(m_matAccount, matAccountCur, element2);
    //Mat imageROI2 = image_dst(cv::Rect(maxLoc2.x, maxLoc2.y, matAccountCur.cols, matAccountCur.rows));
    //matAccountCur.copyTo(imageROI2, m_matMaskAccount);

    Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));
    dilate(image_dst, image_dst, element);
    
    //cv::imshow("matImageSource", matImageSource);
    //cv::imshow("target", image_dst);
    //waitKey();
    inpaint(matImageSource, image_dst, matImageSource, 3, INPAINT_TELEA);
    cv::cvtColor(matImageSource, matImageSource, CV_BGR2YUV_I420);
    //imshow("INPIANT_TELEA", matImageSource);
    //waitKey();
    return VIDEOCLIP_ERRCODE_SUCCESS;
}
