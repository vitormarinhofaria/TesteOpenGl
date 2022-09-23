//
// Created by vitor on 11/09/2022.
//
#include <cstdlib>
#include <dlfcn.h>
#include <vector>

#include "video_rs.h"
#include "libavcodec/avcodec.h"
#include "LogUtils.h"
//#define MINIMP4_IMPLEMENTATION
//#include "minimp4.h"

static avcodec ctx;

void loadLib() {
    ctx.dllHandle = dlopen("libavcodec.so", RTLD_NOW);
//    ctx.otherLibs.push_back(dlopen("libavdevice.so", RTLD_NOW));
//    ctx.otherLibs.push_back(dlopen("libavfilter.so", RTLD_NOW));
//    ctx.otherLibs.push_back(dlopen("libavformat.so", RTLD_NOW));
//    ctx.otherLibs.push_back(dlopen("libavutil.so", RTLD_NOW));
//    ctx.otherLibs.push_back(dlopen("libswresample.so", RTLD_NOW));
//    ctx.otherLibs.push_back(dlopen("libswscale.so", RTLD_NOW));
    if (ctx.dllHandle) {
        ctx.avcodec_find_encoder_by_name = ctx.load_func<p_avcodec_find_encoder_by_name>("avcodec_find_encoder_by_name");
        ctx.avcodec_alloc_context3 = ctx.load_func<p_avcodec_alloc_context3>("avcodec_alloc_context3");
        ctx.av_packet_alloc = ctx.load_func<p_av_packet_alloc>("av_packet_alloc");
        ctx.av_opt_set = ctx.load_func<p_av_opt_set>("av_opt_set");
        ctx.avcodec_open2 = ctx.load_func<p_avcodec_open2>("avcodec_open2");
        ctx.av_frame_alloc = ctx.load_func<p_av_frame_alloc>("av_frame_alloc");
        ctx.av_frame_get_buffer = ctx.load_func<p_av_frame_get_buffer>("av_frame_get_buffer");
        ctx.av_frame_make_writable = ctx.load_func<p_av_frame_make_writable>("av_frame_make_writable");
        ctx.avcodec_send_frame = ctx.load_func<p_avcodec_send_frame>("avcodec_send_frame");
        ctx.avcodec_receive_packet = ctx.load_func<p_avcodec_receive_packet>("avcodec_receive_packet");
        ctx.av_packet_unref = ctx.load_func<p_av_packet_unref>("av_packet_unref");
        ctx.avcodec_free_context = ctx.load_func<p_avcodec_free_context>("avcodec_free_context");
        ctx.av_frame_free = ctx.load_func<p_av_frame_free>("av_frame_free");
        ctx.av_packet_free = ctx.load_func<p_av_packet_free>("av_packet_free");
    }
}

const char* filename = "out_v.mkv";
const char* codec_name = "libvpx-vp9";
const AVCodec* codec;
AVCodecContext* c = nullptr;
FILE* file;
AVFrame* frame;
AVPacket* pkt;

void rgbToYcBcR(double& Y, double & Cb, double & Cr, uint8_t R, uint8_t G, uint8_t B){
    Y = 16 + (65.481 * R + 128.553 * G + 24.966 * B);
    Cb = 128 + (-37.797 * R - 74.203 * G + 112.0 * B);
    Cr = 128 + (112.0 * R - 93.786 * G - 18.214 * B);
}
void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B)
{
    Y =  0.257 * R + 0.504 * G + 0.098 * B +  16;
    U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
    V =  0.439 * R - 0.368 * G - 0.071 * B + 128;
}

void initEncoding(){
    AMediaCodec* mcodec = AMediaCodec_createEncoderByType("video/avc");
    AMediaFormat* format = AMediaCodec_getOutputFormat(mcodec);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);
    int colorFormat = 0;
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &colorFormat);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, 30);
    int frameRate = 0;
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, &frameRate);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, 400000);
    int bitRate = 0;
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, &bitRate);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 720);
    int h = 0;
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &h);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1280);
    int w = 0;
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &w);

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 10);

    ANativeWindow* window = nullptr;
    AMediaCrypto* crypto = nullptr;

    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    media_status_t status = AMediaCodec_configure(mcodec, format, window, crypto, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    if(status == AMEDIA_OK){
        LOG("Created encoder contex");
        AMediaCodec_start(mcodec);
    }else{
        LOG("Failed to create encoder context");
    }
};

void initEncoding2() {
    LOG("libavcodec config:\n\t%s", avcodec_configuration());
    //codec = ctx.avcodec_find_encoder_by_name(codec_name);
    codec = avcodec_find_encoder_by_name(codec_name);
    c = avcodec_alloc_context3(codec);
    pkt = av_packet_alloc();

    c->bit_rate = 400000;
    c->width = 1920;
    c->height = 1080;
    c->time_base = {1, 24};
    c->framerate = {24, 1};

    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    auto res = ctx.avcodec_open2(c, codec, nullptr);
    if (res < 0) {
        LOG("Failed to open codec %d", res);
    }

    //file = fopen(filename, "wb");
    //if (!file) {
      //  LOG("Failed to create file");
    //}

    frame = ctx.av_frame_alloc();
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;

    res = ctx.av_frame_get_buffer(frame, 0);
    if (res < 0) {
        LOG("Failed to allocate video frame buffer");
    }

}

static void encode(AVFrame* pframe){
    static int count = 0;

    auto ret = ctx.avcodec_send_frame(c, pframe);

    if(ret < 0){
        LOG("Error sending frame for encoding");
    }
    while(ret >= 0){
        ret = ctx.avcodec_receive_packet(c, pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }else if(ret < 0){
            LOG("Error while encoding");
        }

        //fwrite(pkt->data, 1, pkt->size, file);

        auto ts = (double)pkt->dts * ((double)1'000'000'000 / (double)24);

        ctx.av_packet_unref(pkt);
    }

    count += 1;
}

struct f3 {
    double Y;
    double Cb;
    double Cr;
    f3(double y, double cb, double cr): Y(y), Cb(cb), Cr(cr){}
};

void addFrame(const uint8_t* imgData, int frameNum){

    auto res = ctx.av_frame_make_writable(frame);
    if(res < 0){
        LOG("Could not make frame writable");
    }
    int x, y = 0;
    for (y = 0; y < c->height; y++) {
        for (x = 0; x < c->width; x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + frameNum * 3;
        }
    }

    //Cb and Cr
    for (y = 0; y < c->height/2; y++) {
        for (x = 0; x < c->width/2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + frameNum * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + frameNum * 5;
        }
    }
    frame->pts = frameNum;
    encode(frame);
    return;
    std::vector<double> cbs{};
    std::vector<double> crs{};
    for( auto y = 0; y < c->height; y++){
        for(auto x = 0; x < c->width; x++){
            double Y, Cb, Cr = 0.0;
            uint8_t r = imgData[x * y];
            uint8_t g = imgData[x * y + 8];
            uint8_t b = imgData[x * y + 16];
            YUVfromRGB(Y, Cb, Cr, r, g, b);
            cbs.emplace_back(Cb);
            crs.emplace_back(Cr);
            frame->data[0][y * frame->linesize[0] + x] = (uint8_t)Y;
        }
    }
    for(auto yc = 0; yc < c->height / 2; yc++){
        for(auto x = 0; x < c->width / 2; x++){
            frame->data[1][yc * frame->linesize[1] + x] = cbs[yc * x];
            frame->data[2][yc * frame->linesize[2] + x] = crs[yc * x];
        }
    }
    //memcpy(frame->data[0], imgData, ((1920 * 1080) * 3) * sizeof(uint8_t));
    frame->pts = frameNum;
    encode(frame);

}

void endEncoding(){
    //encode(nullptr);


    //fclose(file);

    ctx.avcodec_free_context(&c);
    ctx.av_frame_free(&frame);
    ctx.av_packet_free(&pkt);

}


