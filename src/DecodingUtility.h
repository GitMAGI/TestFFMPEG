#ifndef DECODING_UTILITTY_H
#define DECODING_UTILITTY_H

#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

void DecodePacketToFrame(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

#endif