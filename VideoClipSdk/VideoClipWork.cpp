#include "VideoClipWork.h"
#include "log.h"

/**************************************************************************
* name          : r2d
* description   : ʱ���ת��
* input         : r ʱ�����Ϣ
* output        : NA
* return        : ת����ʱ���
* remark        : NA
**************************************************************************/
double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

bool isIdrFrame2(uint8_t* buf, int len){

    switch (buf[0] & 0x1f){
    case 7: // SPS
        return true;
    case 8: // PPS
        return true;
    case 5:
        return true;
    case 1:
        return false;

    default:
        return false;
        break;
    }

    return false;
}
bool isIdrFrame1(uint8_t* buf, int size){
    //��Ҫ�ǽ���idrǰ���sps pps
    //    static bool found = false;
    //    if(found){ return true;}

    int last = 0;
    for (int i = 2; i <= size; ++i){
        if (i == size) {
            if (last) {
                bool ret = isIdrFrame2(buf + last, i - last);
                if (ret) {
                    //found = true;
                    return true;
                }
            }
        }
        else if (buf[i - 2] == 0x00 && buf[i - 1] == 0x00 && buf[i] == 0x01) {
            if (last) {
                int size = i - last - 3;
                if (buf[i - 3]) ++size;
                bool ret = isIdrFrame2(buf + last, size);
                if (ret) {
                    //found = true;
                    return true;
                }
            }
            last = i + 1;
        }
    }
    return false;
}

VideoClipWork::VideoClipWork()
: m_nTaskStat(VIDEOCLIP_TASK_IN_PROCESS)
, m_nCurProcessFileNo(1)
, m_nCurProcessFileSec(0)
, m_pTaskThread(NULL)
, m_bExitTaskUpdate(false)
, m_bStopTaskUpdate(false)
, m_nTaskId(0)
, m_pOc(NULL)
, m_nWaitkey(1)
, m_nVideoPtsInc(0)
, m_nAudioPtsInc(0)
, m_nOutPutVideoIndex(0)
, m_nOutPutAudioIndex(0)
, m_nCurFrameRate(25)
, m_bOpenFile(false)
{
}

VideoClipWork::~VideoClipWork()
{
    DelTask();
}

/**************************************************************************
* name          : StartTask
* description   : ����һ����Ƶƴ������
* input         : nTaskId ����id �����ϱ�����״̬����ѯ����״̬
*                 lstFileList ��ƴ�Ӻ�ȥˮӡ����Ƶ�ļ��б�
*                 strOutputPath ����������Ƶ�ļ�·���������ļ�����
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipWork::AddTask(UINT64 nTaskId, std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath)
{
    m_nTaskStat = VIDEOCLIP_TASK_IN_PROCESS;
    m_nTaskId = nTaskId;
    //���̴߳���
    m_bExitTaskUpdate = false;
    m_bStopTaskUpdate = false;
    m_pTaskThread = new(std::nothrow) boost::thread(boost::bind(&VideoClipWork::VideoClip, this, lstFileList, strOutputPath));
    boost::this_thread::sleep(boost::posix_time::seconds(1)); 
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : StopTask
* description   : ��ͣһ������
* input         : NA
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipWork::StopTask()
{
    m_bStopTaskUpdate = true;
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : StartTask
* description   : �ָ�һ������
* input         : NA
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipWork::StartTask()
{
    m_bStopTaskUpdate = false;
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/**************************************************************************
* name          : DelTask
* description   : ɾ��һ������
* input         : NA
* output        : NA
* return        : 0��ʾ�ɹ� С����ʧ�� ����������붨��
* remark        : NA
**************************************************************************/
int VideoClipWork::DelTask()
{
    if (NULL != m_pTaskThread)
    {
        if (m_pTaskThread->joinable())
        {
            m_bExitTaskUpdate = true;
            m_pTaskThread->join();
        }
        delete m_pTaskThread;
        m_pTaskThread = NULL;
    }
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

/* Add an output stream */
AVStream* VideoClipWork::AddStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, AVStream *in_stream)
{
    AVCodecContext *c;
    AVStream *st;
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!*codec)
    {
        IF_LOG_E("AddStream, could not find encoder for " << avcodec_get_name(codec_id));
        return NULL;
        //exit(1);
    }
    st = avformat_new_stream(oc, *codec);
    if (!st)
    {
        IF_LOG_E("AddStream, could not allocate stream");
        return NULL;
        //exit(1);
    }

    st->id = oc->nb_streams - 1;
    c = st->codec;
    switch ((*codec)->type)
    {
    case AVMEDIA_TYPE_AUDIO:
        m_nOutPutAudioIndex = st->index;
        c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate = in_stream->codec->bit_rate; //64000;
        c->sample_rate = in_stream->codec->sample_rate;// 44100;
        c->channels = in_stream->codec->channels;//2
        break;
    case AVMEDIA_TYPE_VIDEO:
        m_nOutPutVideoIndex = st->index;
        st->codec->codec_tag = 0;
        c->codec_id = in_stream->codec->codec_id;
        c->bit_rate = 0;
        c->width = in_stream->codec->width;
        c->height = in_stream->codec->height;
        c->time_base.den = in_stream->codec->time_base.den;
        c->time_base.num = in_stream->codec->time_base.num;
        c->gop_size = in_stream->codec->gop_size;
        c->pix_fmt = AV_PIX_FMT_YUV420P;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
        {
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        {
            c->mb_decision = 2;
        }
        break;
    default:
        break;
    }
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    {
        //c->flags |= CODEC_FLAG_GLOBAL_HEADER;
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    return st;
}
int VideoClipWork::OpenVideo(AVFormatContext *oc, AVCodec *codec, AVStream *st)
{
    int ret;
    AVCodecContext *c = st->codec;
    /* open the codec */
    ret = avcodec_open2(c, codec, NULL);
    return ret;
}
int VideoClipWork::CreateMp4(const char* filename, AVStream *video_stream, AVStream *audio_stream)
{
    int ret; // �ɹ�����0��ʧ�ܷ���1
    const char* pszFileName = filename;
    AVOutputFormat *fmt;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    AVStream *m_pVideoSt;
    AVStream *m_pAudioSt;
    //av_register_all();
    avformat_alloc_output_context2(&m_pOc, NULL, NULL, pszFileName);
    if (!m_pOc)
    {
        IF_LOG_E("CreateMp4, avformat_alloc_output_context2 failed, file: " << pszFileName);
        return VIDEOCLIP_ERRCODE_MALLOC_FAILED;
    }
    fmt = m_pOc->oformat;
    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        m_pVideoSt = AddStream(m_pOc, &video_codec, fmt->video_codec, video_stream);
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE)
    {
        m_pAudioSt = AddStream(m_pOc, &audio_codec, fmt->audio_codec, audio_stream);
    }
    if (m_pVideoSt)
    {
        ret = OpenVideo(m_pOc, video_codec, m_pVideoSt);
        if (ret < 0)
        {
            IF_LOG_E("��������ļ�����Ƶ������ʧ��, ����ļ�: " << pszFileName << ", ��Ƶ����������: " << avcodec_get_name(fmt->video_codec) << ", ������:" << ret);
            return VIDEOCLIP_ERRCODE_CPOTPUT_OPEN_VIDEO_AVCODEC_FAILED;
        }
    }
    else
    {
        IF_LOG_E("��������ļ������Ƶ��ʧ��, ����ļ�: " << pszFileName);
        return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    }

    if (m_pAudioSt)
    {
        ret = OpenVideo(m_pOc, audio_codec, m_pAudioSt);;
        if (ret < 0)
        {
            IF_LOG_E("��������ļ�����Ƶ������ʧ��, ����ļ�: " << pszFileName << ", ��Ƶ����������: " << avcodec_get_name(fmt->audio_codec) << ", ������:" << ret);
            return VIDEOCLIP_ERRCODE_CPOTPUT_OPEN_AUDIO_AVCODEC_FAILED;
        }
    }
    else
    {
        IF_LOG_E("��������ļ������Ƶ��ʧ��, ����ļ�: " << pszFileName);
        return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    }

    av_dump_format(m_pOc, 0, pszFileName, 1);
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&m_pOc->pb, pszFileName, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            IF_LOG_E("CreateMp4, avio_open out file failed, file: " << pszFileName);
            return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
        }
    }
    m_bOpenFile = true;
    /* Write the stream header, if any */
    ret = avformat_write_header(m_pOc, NULL);
    if (ret < 0)
    {
        IF_LOG_E("CreateMp4, дmp4ͷ��Ϣ��mp4�ļ�ʧ��, �ļ�: " << pszFileName);
        return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    }
    return VIDEOCLIP_ERRCODE_SUCCESS;
}

int VideoClipWork::CreateMp4(const char* filename, AVStream *in_stream)
{
    int ret; // �ɹ�����0��ʧ�ܷ���1
    const char* pszFileName = filename;
    AVOutputFormat *fmt;
    AVCodec *video_codec;
    AVStream *m_pVideoSt;
    //av_register_all();
    avformat_alloc_output_context2(&m_pOc, NULL, NULL, pszFileName);
    if (!m_pOc)
    {
        IF_LOG_E("CreateMp4, avformat_alloc_output_context2 failed, file: " << pszFileName);
        return VIDEOCLIP_ERRCODE_MALLOC_FAILED;
    }

    fmt = m_pOc->oformat;
    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        m_pVideoSt = AddStream(m_pOc, &video_codec, fmt->video_codec, in_stream);
    }

    //if (m_pVideoSt)
    //{
    //    OpenVideo(m_pOc, video_codec, m_pVideoSt);
    //}
    //else
    //{
    //    IF_LOG_E("aOpenVideo failed, file: " << pszFileName);
    //    return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    //}

    if (m_pVideoSt)
    {
        ret = OpenVideo(m_pOc, video_codec, m_pVideoSt);
        if (ret < 0)
        {
            IF_LOG_E("��������ļ�����Ƶ������ʧ��, ����ļ�: " << pszFileName << ", ��Ƶ����������: " << avcodec_get_name(fmt->video_codec) << ", ������:" << ret);
            return VIDEOCLIP_ERRCODE_CPOTPUT_OPEN_VIDEO_AVCODEC_FAILED;
        }
    }
    else
    {
        IF_LOG_E("��������ļ������Ƶ��ʧ��, ����ļ�: " << pszFileName);
        return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    }

    av_dump_format(m_pOc, 0, pszFileName, 1);
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&m_pOc->pb, pszFileName, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            IF_LOG_E("avio_open out file failed, file: " << pszFileName);
            return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
        }
    }
    m_bOpenFile = true;
    /* Write the stream header, if any */
    ret = avformat_write_header(m_pOc, NULL);
    if (ret < 0)
    {
        IF_LOG_E("CreateMp4, avformat_write_header failed, file: " << pszFileName);
        return VIDEOCLIP_ERRCODE_AVIO_OPEN_FAILED;
    }
    return VIDEOCLIP_ERRCODE_SUCCESS;
}
/* write h264 data to mp4 file

* ����mp4�ļ�����2��д������֡����0 */
void VideoClipWork::WriteVideo(void* data, int nLen, AVStream *in_stream)
{
    int ret;
    AVStream *pst = m_pOc->streams[m_nOutPutVideoIndex];
    // Init packet
    AVPacket pkt;
    // �ҵ���ӣ�Ϊ�˼���pts
    //AVCodecContext *c = pst->codec;
    av_init_packet(&pkt);
    int isI = isIdrFrame1((uint8_t*)data, nLen);
    pkt.flags |= isI ? AV_PKT_FLAG_KEY : 0;
    pkt.stream_index = pst->index;
    pkt.data = (uint8_t*)data;
    pkt.size = nLen;
    // Wait for key frame
    if (m_nWaitkey){
        if (0 == (pkt.flags & AV_PKT_FLAG_KEY)){
            return;
        }
        else
            m_nWaitkey = 0;
    }

    //Write PTS  
    AVRational time_base1;
    time_base1.num = in_stream->time_base.num;
    time_base1.den = in_stream->time_base.den;
    //Duration between 2 frames (us)  
    int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
    //Parameters  
    pkt.pts = (double)(m_nVideoPtsInc*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
    pkt.dts = pkt.pts;
    pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
    m_nVideoPtsInc++;
    //Convert PTS/DTS  
    pkt.pts = av_rescale_q_rnd(pkt.pts, time_base1, pst->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt.dts = av_rescale_q_rnd(pkt.dts, time_base1, pst->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

    pkt.duration = av_rescale_q(pkt.duration, time_base1, pst->time_base);

    //pkt.pts = (m_nVideoPtsInc++) * (9000 / m_nCurFrameRate);
    //pkt.pts = av_rescale_q((m_nVideoPtsInc++) * 2, pst->codec->time_base, pst->time_base);
    ////pkt.dts = (ptsInc++) * (90000/m_nCurFrameRate);
    ////  pkt.pts=av_rescale_q_rnd(pkt.pts, pst->time_base,pst->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    //pkt.dts = av_rescale_q_rnd(pkt.dts, pst->time_base, pst->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    //pkt.duration = av_rescale_q(pkt.duration, pst->time_base, pst->time_base);
    pkt.pos = -1;
    ret = av_interleaved_write_frame(m_pOc, &pkt);
    //av_destruct_packet(&pkt);
}

void VideoClipWork::WriteAudio(void* data, int nLen, int nDuration)
{
    int ret;
    AVStream *pst = m_pOc->streams[m_nOutPutAudioIndex];
    // Init packet
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.stream_index = pst->index;
    pkt.data = (uint8_t*)data;
    pkt.size = nLen;

    pkt.pts = m_nAudioPtsInc + nDuration;
    pkt.dts = pkt.pts;
    pkt.duration = nDuration;
    pkt.pos = -1;
    m_nAudioPtsInc += nDuration;
    ret = av_interleaved_write_frame(m_pOc, &pkt);

}

void VideoClipWork::CloseMp4()
{
    m_nWaitkey = -1;
    m_nOutPutVideoIndex = 0;
    m_nOutPutAudioIndex = 0;
    if (m_pOc && m_bOpenFile)
        av_write_trailer(m_pOc);
    if (m_pOc &&  m_bOpenFile && !(m_pOc->oformat->flags & AVFMT_NOFILE))
        avio_close(m_pOc->pb);
    if (m_pOc)
    {
        avformat_free_context(m_pOc);
        m_pOc = NULL;
    }
    m_bOpenFile = false;
}
/**************************************************************************
* name          : VideoClip
* description   : ��Ƶƴ�Ӵ�����
* input         : lstFileList ��ƴ�Ӻ�ȥˮӡ����Ƶ�ļ��б�
*                 strOutputPath ����������Ƶ�ļ�·���������ļ�����
* output        : NA
* return        : NA
* remark        : NA
**************************************************************************/
void VideoClipWork::VideoClip(std::list<VIDEO_CLIP_TASK_INFO_T> lstFileList, std::string strOutputPath)
{
    IF_LOG_I("��ʼ��������, ����id:" << m_nTaskId);
    bool bRet = true;
    bool bCreateMp4 = false;
    std::list<VIDEO_CLIP_TASK_INFO_T>::iterator itorFile = lstFileList.begin();
    int nFirstW = -1;
    int nFirstH = -1;
    m_nCurProcessFileNo = 0;
    for (; itorFile != lstFileList.end(); ++itorFile)
    {
        m_nCurProcessFileNo += 1;
        m_nCurProcessFileSec = 0;
        Watermark m_oWatermark;
        if (itorFile->enOsdType == OSD_TYPE_DOUYIN)
        {
            int nRet = m_oWatermark.AnalyseDouyin(itorFile->strFilePath);
            if (nRet < 0)
            {
                m_nTaskStat = nRet;
                return;
            }
        }
        //���������˺���Ϣ
        AVStream *video_stream = NULL;
        AVStream *audio_stream = NULL;
        //��װ��ʽ�����ģ�ͳ��ȫ�ֵĽṹ�壬��������Ƶ�ļ���װ��ʽ�������Ϣ
        AVFormatContext *pFormatCtx = NULL;
        //2.��������Ƶ�ļ�
        if (avformat_open_input(&pFormatCtx, itorFile->strFilePath.c_str(), NULL, NULL) != 0)
        {
            IF_LOG_E("open video file failed! file: " << itorFile->strFilePath);
            m_nTaskStat = VIDEOCLIP_ERRCODE_OPEN_INPUT_FAILED;
            bRet = false;
            break;
        }

        //3.��ȡ��Ƶ�ļ���Ϣ
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        {
            IF_LOG_E("find stream info failed! file: " << itorFile->strFilePath);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_FIND_STREAM_INFO_FAILED;
            bRet = false;
            break;
        }
        int nDuration = pFormatCtx->duration / AV_TIME_BASE;
        //��ȡ��Ƶ��������λ��
        //�����������͵�������Ƶ������Ƶ������Ļ�������ҵ���Ƶ������Ƶ��
        int nVideoIdx = -1;
        int nAudioIdx = -1;
        int i = 0;
        //number of streams
        for (; i < pFormatCtx->nb_streams; i++)
        {
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audio_stream = pFormatCtx->streams[i];
                nAudioIdx = i;
            }
            //��������
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_stream = pFormatCtx->streams[i];
                nVideoIdx = i;
            }
        }

        if (nVideoIdx == -1)
        {
            IF_LOG_E("�Ҳ�����Ƶ��:  �ļ�:" << itorFile->strFilePath);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_NO_VIDEO;
            bRet = false;
            break;
        }
        if (!bCreateMp4)
        {
            if (nAudioIdx == -1)
            {
                int nRet = CreateMp4(strOutputPath.c_str(), video_stream);
                if (0 > nRet)
                {
                    CloseMp4();
                    m_nTaskStat = nRet;
                    bRet = false;
                    break;
                }
            }
            else
            {
                int nRet = CreateMp4(strOutputPath.c_str(), video_stream, audio_stream);
                if (0 > nRet)
                {
                    CloseMp4();
                    m_nTaskStat = nRet;
                    bRet = false;
                    break;
                }
            }
            nFirstW = video_stream->codec->width;
            nFirstH = video_stream->codec->height;
            bCreateMp4 = true;
        }
        //������س�ʼ��
        //��ȡ��Ƶ���еı����������
        AVCodecContext *pCodecCtx = pFormatCtx->streams[nVideoIdx]->codec;
        //4.���ݱ�����������еı���id���Ҷ�Ӧ�Ľ���
        AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            IF_LOG_E("�Ҳ���������:  �ļ�:" << itorFile->strFilePath << ", ��������: " << pCodecCtx->codec_id);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_FIND_CODEC_FAILED;
            bRet = false;
            break;
        }

        //5.�򿪽�����
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            IF_LOG_E("�������޷���:  �ļ�:" << itorFile->strFilePath << ", ������������: " << pCodec->name);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_OPEN_CODEC_FAILED;
            bRet = false;
            break;
        }

        //������س�ʼ��
        AVCodec* pCoEnc = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (pCoEnc == NULL)
        {
            IF_LOG_E("�Ҳ���������:  �ļ�:" << itorFile->strFilePath << ", ��������: AV_CODEC_ID_H264");
            avcodec_close(pCodecCtx);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_FIND_CODEC_FAILED;
            bRet = false;
            break;
        }
        AVCodecContext* pCodecCtxEnc;

        pCodecCtxEnc = avcodec_alloc_context3(pCoEnc);//����AVCodecContext������ʼ����  
        if (!pCodecCtxEnc)
        {
            IF_LOG_E("encode avcodec_alloc_context3 failed:  FILE:" << itorFile->strFilePath << ", ��������: AV_CODEC_ID_H264");
            avcodec_close(pCodecCtx);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_FIND_CODEC_FAILED;
            bRet = false;
            //failed get AVCodecContext  
            break;
        }

        //pCodecCtxEnc = video_st->codec;
        pCodecCtxEnc->codec_id = AV_CODEC_ID_H264;
        pCodecCtxEnc->codec_type = AVMEDIA_TYPE_VIDEO;
        pCodecCtxEnc->pix_fmt = pCodecCtx->pix_fmt;
        pCodecCtxEnc->width = nFirstW;
        pCodecCtxEnc->height = nFirstH;
        pCodecCtxEnc->b_frame_strategy = true;

        pCodecCtxEnc->bit_rate = pCodecCtx->bit_rate;
        pCodecCtxEnc->gop_size = pCodecCtx->gop_size;
        pCodecCtxEnc->time_base.num = pCodecCtx->time_base.num;
        pCodecCtxEnc->time_base.den = pCodecCtx->time_base.den;
        //��С����������
        pCodecCtxEnc->qmin = pCodecCtx->qmin;
        //������������
        pCodecCtxEnc->qmax = pCodecCtx->qmax;
        //���B֡��
        pCodecCtxEnc->max_b_frames = pCodecCtx->max_b_frames;
        pCodecCtxEnc->sample_aspect_ratio = pCodecCtx->sample_aspect_ratio;
        pCodecCtxEnc->qcompress = pCodecCtx->qcompress;
        pCodecCtxEnc->mb_decision = pCodecCtx->mb_decision;

        if (avcodec_open2(pCodecCtxEnc, pCoEnc, NULL) < 0)
        {
            IF_LOG_E("encode avcodec_open2 failed:  FILE:" << itorFile->strFilePath << ", CODE ID: AV_CODEC_ID_H264");
            avcodec_free_context(&pCodecCtxEnc);
            avcodec_close(pCodecCtx);
            avformat_close_input(&pFormatCtx);
            m_nTaskStat = VIDEOCLIP_ERRCODE_OPEN_CODEC_FAILED;
            bRet = false;
            break;
        }

        int picture_size = avpicture_get_size(pCodecCtx->pix_fmt, nFirstW, nFirstH);
        AVPacket pktEnc;
        av_new_packet(&pktEnc, picture_size);

        //����ת����ʼ��
        struct SwsContext *sws_ctx = NULL;
        //ffmpegתopencv
        //����ת�루���ţ��Ĳ�����ת֮ǰ�Ŀ�ߣ�ת֮��Ŀ�ߣ���ʽ��
        sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BICUBIC, NULL, NULL, NULL);

        //׼����ȡ
        //AVPacket���ڴ洢һ֡һ֡��ѹ�����ݣ�H264��
        //�����������ٿռ�
        AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));

        //AVFrame���ڴ洢��������������(YUV)
        //�ڴ����
        AVFrame *pFrame = av_frame_alloc();
        int got_picture, ret;

        bool bEnableTimeQuantum = false;
        int nCurBeginTime = 0;
        int nCurEndTime = 0;
        nCurBeginTime = itorFile->stTimeQuantum.nBeginTime;
        nCurEndTime = itorFile->stTimeQuantum.nEndTime;
        if (nCurBeginTime > 0 && nCurBeginTime < nDuration)
        {
            ret = av_seek_frame(pFormatCtx, -1, nCurBeginTime * AV_TIME_BASE, AVSEEK_FLAG_ANY);
        }
        if (nCurEndTime > 0)
        {
            nCurEndTime = nCurEndTime * 1000;//ת����
            bEnableTimeQuantum = true;
        }

        //6.һ֡һ֡�Ķ�ȡѹ������
        bool bReadAllDate = true;
        int nVideoStreamIdx = 0;

        AVFrame *avframe = av_frame_alloc();
        unsigned char *out_buffer = NULL;
        out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, nFirstW, nFirstH, 1));
        av_image_fill_arrays(avframe->data, avframe->linesize, out_buffer, AV_PIX_FMT_YUV420P, nFirstW, nFirstH, 1);
        bool bFirstYuv = true;
        struct SwsContext *convert_ctx = NULL;
        cv::Mat mat;
        cv::Mat matResize;
        AVFrame dst;
        while (av_read_frame(pFormatCtx, packet) >= 0)
        {
            //�ж�ʱ���
            //����ʱ��� ����seek
            //ֻҪ��Ƶѹ�����ݣ�������������λ���жϣ�
            if (packet->stream_index == nVideoIdx)
            {
                int nPts = (packet->pts *r2d(pFormatCtx->streams[nVideoIdx]->time_base)) * 1000;
                int nDts = (packet->dts *r2d(pFormatCtx->streams[nVideoIdx]->time_base)) * 1000;
                m_nCurProcessFileSec = nPts/1000;
                if (bEnableTimeQuantum && nPts > nCurEndTime && nDts > nCurEndTime)
                {
                    break;
                }
                //7.����һ֡��Ƶѹ�����ݣ��õ���Ƶ��������
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if (ret < 0)
                {
                    IF_LOG_E("Failed to decode one frame!" << itorFile->strFilePath);
                }

                //Ϊ0˵��������ɣ���0���ڽ���
                if (got_picture)
                {
                    //avframe to mat
                    if (bFirstYuv)
                    {
                        //enum PixelFormat src_pixfmt = (enum PixelFormat)pFrame->format;
                        //enum PixelFormat dst_pixfmt = PIX_FMT_YUV420P;
                        enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)pFrame->format;
                        enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_YUV420P;
                        mat = cv::Mat(pFrame->height * 3 / 2, pFrame->width, CV_8UC1);
                        convert_ctx = sws_getContext(pFrame->width, pFrame->height, src_pixfmt, pFrame->width, pFrame->height, dst_pixfmt, SWS_BICUBIC, NULL, NULL, NULL);
                        bFirstYuv = false;
                    }
                    memset(&dst, 0, sizeof(dst));
                    dst.data[0] = (uint8_t *)mat.data;
                    //avpicture_fill((AVPicture *)&dst, dst.data[0], PIX_FMT_YUV420P, pFrame->width, pFrame->height);
                    avpicture_fill((AVPicture *)&dst, dst.data[0], AV_PIX_FMT_YUV420P, pFrame->width, pFrame->height);
                    sws_scale(convert_ctx, pFrame->data, pFrame->linesize, 0, pFrame->height, dst.data, dst.linesize);

                    //ȥˮӡ
                    if (itorFile->enOsdType == OSD_TYPE_DOUYIN)
                    {
                        m_oWatermark.ProcessDouyin(mat);
                    }
                    //����͵�һ����Ƶ�ֱ��ʲ�һ������Ҫת���ֱ��ʳɵ�һ����Ƶ
                    if (pFrame->width != nFirstW || pFrame->height != nFirstH)
                    {
                        cv::cvtColor(mat, matResize, CV_YUV2BGR_I420);
                        cv::resize(matResize, matResize, cv::Size(nFirstW, nFirstH), (0, 0), (0, 0), cv::INTER_LINEAR);
                        cv::cvtColor(matResize, matResize, CV_BGR2YUV_I420);
                        if (avframe && !matResize.empty()) {
                            int frame_size = nFirstW * nFirstH;
                            unsigned char *pdata = matResize.data;
                            // fill yuv420
                            avframe->data[0] = pdata;// fill y
                            avframe->data[1] = pdata + frame_size;// fill u
                            avframe->data[2] = pdata + frame_size * 5 / 4;// fill v
                            avframe->width = nFirstW;
                            avframe->height = nFirstH;
                            avframe->format = pFrame->format;
                        }
                    }
                    else
                    {
                        //mat to avframe
                        if (avframe && !mat.empty()) {
                            int frame_size = nFirstW * nFirstH;
                            unsigned char *pdata = mat.data;
                            // fill yuv420
                            avframe->data[0] = pdata;// fill y
                            avframe->data[1] = pdata + frame_size;// fill u
                            avframe->data[2] = pdata + frame_size * 5 / 4;// fill v
                            avframe->width = nFirstW;
                            avframe->height = nFirstH;
                            avframe->format = pFrame->format;
                        }
                    }
                    //����
                    avframe->pts = nVideoStreamIdx;
                    nVideoStreamIdx++;
                    ret = avcodec_encode_video2(pCodecCtxEnc, &pktEnc, avframe, &got_picture);
                    if (ret < 0)
                    {
                        IF_LOG_E("Failed to encode one frame!" << itorFile->strFilePath);
                    }
                    if (got_picture == 1)
                    {
                        WriteVideo(pktEnc.data, pktEnc.size, video_stream);
                        // ����������Ƶ����д���ļ� 
                        av_free_packet(&pktEnc);
                    }
                }
            }
            else if (packet->stream_index == nAudioIdx)
            {
                //audio_stream->nb_frames;
                //audio_stream->time_base;
                
                int nPts = (packet->pts *r2d(audio_stream->time_base)) * 1000;
                //int nPts = packet->pts / packet->duration;
                if (bEnableTimeQuantum && nPts > nCurEndTime)
                {
                    continue;
                }
                //д�ļ�
                WriteAudio(packet->data, packet->size, packet->duration);
            }
            else
            {
                //do nothing
            }
            //�ͷ���Դ
            av_free_packet(packet);
            if (m_bExitTaskUpdate)
            {
                //�ͷ���Դ
                m_nTaskStat = VIDEOCLIP_TASK_IS_DELETE;
                bReadAllDate = false;
                break;
            }
            while (m_bStopTaskUpdate)
            {
                boost::this_thread::sleep(boost::posix_time::seconds(2));
            }
        }
        if (bReadAllDate)
        {
            if ((m_pOc->streams[m_nOutPutVideoIndex]->codec->codec->capabilities & /*AV_CODEC_CAP_DELAY*/CODEC_CAP_DELAY))
            {
                while (true)
                {
                    ret = avcodec_encode_video2(pCodecCtxEnc, &pktEnc, NULL, &got_picture);
                    if (ret < 0)
                    {
                        break;
                    }
                    if (!got_picture)
                    {
                        ret = 0;
                        break;
                    }
                    WriteVideo(pktEnc.data, pktEnc.size, video_stream);
                    av_free_packet(&pktEnc);
                }
            }
        }

        if (pFrame != NULL)
        {
            av_frame_free(&pFrame);
            pFrame = NULL;
        }
        if (avframe != NULL)
        {
            av_frame_free(&avframe);
            av_free(out_buffer);
            out_buffer = NULL;
            avframe = NULL;
        }
        avcodec_free_context(&pCodecCtxEnc);
        if (convert_ctx != NULL)
        {
            sws_freeContext(convert_ctx);
            convert_ctx = NULL;
        }
        //av_free(picture_buf);
        av_free(packet);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);
        av_free_packet(&pktEnc);
        sws_freeContext(sws_ctx);
    }
    /* close output */
    CloseMp4();
    if (bRet == true)
    {
        m_nTaskStat = VIDEOCLIP_TASK_IS_COMPLETE;
    }
    
    IF_LOG_I("������������, ����id:" << m_nTaskId);
    return;
}
