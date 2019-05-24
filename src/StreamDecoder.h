#ifndef STREAM_DECODER_H
#define STREAM_DECODER_H

#include <libavformat/avformat.h>

int CStreamDecoder(const char *filename);
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

void TestCStreamDecoder(const char *filename);

#endif