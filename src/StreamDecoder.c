#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "StreamDecoder.h"

#define BUFSIZE 0x10000

int CStreamDecoder(const char *filename){
    char * _current_ = "CStreamDecoder";
    fprintf(stdout, "%s starting ...\n", _current_);
    
    // 1.
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    //AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        fprintf(stderr, "%s\n", "H.264 decoder not found");
        goto end;
    }
    fprintf(stdout, "%s decoder H264 found\n", "AVCodec");

    // 2. 
    AVCodecParserContext *parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "Parser not found\n");
        goto end;
    }
    fprintf(stdout, "%s initiated\n", "AVCodecParserContext");

    // 3.
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        goto finally_close_parser;
    }
    fprintf(stdout, "%s allocated\n", "AVCodecContext");

    // 4.
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    /* open it */
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        goto finally_free_avcodec_ctx;
    }
    fprintf(stdout, "%s opened\n", "AVCodec");

    FILE *f;
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        goto finally_free_avcodec_ctx;
    }
    fprintf(stdout, "File %s opened\n", filename);
    uint8_t inbuf[BUFSIZE + AV_INPUT_BUFFER_PADDING_SIZE];    
    AVPacket *packet = av_packet_alloc();
    if (!packet){
        fprintf(stderr, "Could not allocate video packet\n");
        goto finally_close_file;
    }
    fprintf(stdout, "%s allocated\n", "AVPacket");
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        av_packet_free(&packet);
        goto finally_close_file;
    }
    fprintf(stdout, "%s allocated\n", "AVFrame");
    while (!feof(f)) {
        /* read raw data from the input file */
        size_t data_size = fread(inbuf, 1, BUFSIZE, f);
        if (!data_size)
            break;    
        uint8_t *data = inbuf;        
        while (data_size > 0) {
            int ret = av_parser_parse2(parser, codec_ctx, &packet->data, &packet->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                goto finally_free_packet;
            }
            data += ret;
            data_size -= ret;
            if (packet->size)
                decode(codec_ctx, frame, packet);
        }
    }    
    /* flush the decoder */
    decode(codec_ctx, frame, NULL);

    finally_free_packet:
        av_packet_free(&packet);
        fprintf(stdout, "%s deaallocated\n", "AVPacket");
    finally_free_frame:
        av_frame_free(&frame);
        fprintf(stdout, "%s deallocated\n", "AVFrame");
    finally_close_file:
        fclose(f);
        fprintf(stdout, "File %s closed\n", filename);
    finally_free_avcodec_ctx:
        avcodec_free_context(&codec_ctx);
        fprintf(stdout, "%s deallocated\n", "AVCodecContext");
    finally_close_parser:
        av_parser_close(parser);
        fprintf(stdout, "%s closed\n", "AVCodecParserContext");
    end:
        fprintf(stdout, "%s completed\n", _current_);
        return 0;
}

static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    char buf[1024];
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return;
        }
        fprintf(stdout, "Using frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);
    }
}

void TestCStreamDecoder(const char *filename)
{    
    int r = CStreamDecoder(filename);
    fprintf(stdout, "CStreamDecoder returned %d\n", r);
}