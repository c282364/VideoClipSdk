#pragma once
#include "CommDef.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>
class Watermark
{
public:
    Watermark();
    virtual ~Watermark();

    int AnalyseDouyin(std::string strFile);
    int ProcessDouyin(cv::Mat &mat);
private:
    cv::Mat m_matAccount; //�˺�ˮӡͼ
    cv::Mat m_matLogo; //ͼ��ˮӡͼ
    cv::Mat m_matLogoResize; //ͼ��ˮӡͼ
    cv::Mat m_matWords; //��������ˮӡͼ
    cv::Mat m_matMaskWord; //
    cv::Mat m_matMaskLogo; //
    cv::Mat m_matMaskAccount; //
    cv::Mat m_matTemplate1Erode;
    double m_dResizeW;
    double m_dResizeH;
};
;
