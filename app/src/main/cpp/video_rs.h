//
// Created by vitor on 11/09/2022.
//

#ifndef TESTEOPENGL_VIDEO_RS_H
#define TESTEOPENGL_VIDEO_RS_H
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <dlfcn.h>
//
typedef AVCodec *(*p_avcodec_find_encoder_by_name)(const char * name);
typedef AVCodecContext *(*p_avcodec_alloc_context3)(AVCodec * codec);
typedef AVPacket*(*p_av_packet_alloc)();
typedef int(*p_av_opt_set)(void* obj, const char* name, const char* val, int search_flags);
typedef int(*p_avcodec_open2)(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options);
typedef AVFrame*(*p_av_frame_alloc)();
typedef int(*p_av_frame_get_buffer)(AVFrame* frame, int align);
typedef int(*p_av_frame_make_writable)(AVFrame* frame);
typedef int(*p_avcodec_send_frame)(AVCodecContext* avctx, const AVFrame* frame);
typedef int(*p_avcodec_receive_packet)(AVCodecContext* avctx, AVPacket* avpkt);
typedef void(*p_av_packet_unref)(AVPacket* pkt);
typedef void(*p_avcodec_free_context)(AVCodecContext **avctx);
typedef void(*p_av_frame_free)(AVFrame** frame);
typedef void(*p_av_packet_free)(AVPacket** pkt);

struct avcodec {
    void *dllHandle;
    p_avcodec_find_encoder_by_name avcodec_find_encoder_by_name;
    p_avcodec_alloc_context3 avcodec_alloc_context3;
    p_av_packet_alloc av_packet_alloc;
    p_av_opt_set av_opt_set;
    p_avcodec_open2 avcodec_open2;
    p_av_frame_alloc av_frame_alloc;
    p_av_frame_get_buffer av_frame_get_buffer;
    p_av_frame_make_writable av_frame_make_writable;
    p_avcodec_send_frame avcodec_send_frame;
    p_avcodec_receive_packet avcodec_receive_packet;
    p_av_packet_unref av_packet_unref;
    p_avcodec_free_context avcodec_free_context;
    p_av_frame_free av_frame_free;
    p_av_packet_free av_packet_free;

    std::vector<void*> otherLibs;

    template<typename T>
    T load_func(const char *func_name) {
        return (T) dlsym(dllHandle, func_name);
    }
};



struct avformat {
    void* handle;

};

void loadLib();

void initEncoding();
void addFrame(const uint8_t* frame, int frameCount);
void endEncoding();

void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B);

#endif //TESTEOPENGL_VIDEO_RS_H
