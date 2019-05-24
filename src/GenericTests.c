#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "GenericTests.h"
 
void GeneralTest(){
    // 5.
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "%s\n", "H.264 decoder not found");
        return;
    }
    fprintf(stdout, "%s decoder H264 found\n", "AVCodec");

    // 6.
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate decoder context. Error Code %d\n", AVERROR(errno));
        return;
    }
    fprintf(stdout, "%s allocated\n", "AVCodecContext");
}