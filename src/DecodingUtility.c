#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "DecodingUtility.h"
#include "custom/log.h"

void DecodePacketToFrame(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    //infoLog("Starting ...");

    // Stream Index
    // 0. Video
    // 1. Audio

    if(pkt->stream_index != 0){
        infoLog("Actual Stream Index %d. Has been skipped!", pkt->stream_index);
        return;
    }

    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0  || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        errno = 0;
        errorLog("Error while sending a packet for decoding. AVError Code: %d. AVError Description: %s", AVERROR(ret), av_err2str(ret));
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            errno = 0;
            errorLog("Error during decoding. AVError Code: %d. AVError Description: %s", AVERROR(ret), av_err2str(ret));
            return;
        }
        infoLog("Using frame %3d", dec_ctx->frame_number);
        fflush(stdout);
    }
    //infoLog("Completed");
}