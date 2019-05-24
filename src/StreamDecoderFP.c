#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "StreamDecoderFP.h"

#define BUFSIZE 0x10000

int CStreamDecoderFP(void *data, int(*read_raw_data)(void *, uint8_t *, int)){
    char * _current_ = "CStreamDecoderFP";
    fprintf(stdout, "%s starting ...\n", _current_);

    // 1. AV Format Context Allocation
    AVFormatContext *format_ctx = avformat_alloc_context();
    if (!format_ctx) {
        fprintf(stderr, "%s\n", "Could not allocate format context");
        goto end;
    }
    fprintf(stdout, "%s created\n", "AVFormatContext");

    // 2. Buffer Allocation
    unsigned char *buffer = av_malloc(BUFSIZE);
    if (!buffer) {
        fprintf(stderr, "%s\n", "Could not allocate buffer");        
        goto finally_free_format_ctx;
    }
    fprintf(stdout, "Buffer Allocated\n");

    // 3. AVIO Context Allocation
    int (*read_packet)(void *, uint8_t *, int) = ReadRawPacket;
    //Convert Data to the Struct stream
    struct fstream *stream = data;
    AVIOContext *avio_ctx = avio_alloc_context(buffer, BUFSIZE, 0, stream, read_packet, NULL, NULL);      
    if (!avio_ctx) {
        fprintf(stderr, "%s\n", "Could not allocate avio context");
        av_free(buffer);
        goto finally_free_format_ctx;
    }
    fprintf(stdout, "%s created\n", "AVIOContext");
    format_ctx->pb = avio_ctx;
    fprintf(stdout, "%s assigned to %s\n", "AVIOContext", "AVFormatContext");

    // 4. 
    if (avformat_open_input(&format_ctx, NULL, NULL, NULL) < 0) {
        fprintf(stderr, "%s\n", "Could not open video stream");
        goto finally_free_avio_ctx;
    }
    fprintf(stdout, "%s input opened\n", "AVFormatContext");

    // 5.
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "%s\n", "H.264 decoder not found");
        goto end;
    }
    fprintf(stdout, "%s decoder H264 found\n", "AVCodec");

    // 6.
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx) {
        fprintf(stderr, "%s\n", "Could not allocate decoder context");
        goto finally_close_input;
    }
    fprintf(stdout, "%s allocated\n", "AVCodecContext");

    // 7. 
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "%s\n", "Could not open codec");
        avcodec_free_context(&codec_ctx);
        goto finally_close_input;
    }
    fprintf(stdout, "%s opened\n", "AVCodecContext");

    // 8.
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    while (!av_read_frame(format_ctx, &packet)) {       
        uint8_t *packet_data = NULL;
        memcpy(&packet_data, &packet.data, sizeof(uint8_t) * packet.size);    

        struct timeval tv;
        gettimeofday(&tv, NULL);

        stream->packet_data = packet_data;
        stream->packet_data_size = packet.size;
        stream->timestamp = tv;

        fprintf(stdout, "Going!\n");
        AVFrame *frame = av_frame_alloc();        
        decode(codec_ctx, frame, &packet);

        av_packet_unref(&packet);
        if (avio_ctx->eof_reached) {
            break;
        }
    }

    finally_close_input:
        avformat_close_input(&format_ctx);
    finally_free_avio_ctx:
        av_free(avio_ctx);
    finally_free_format_ctx:
        avformat_free_context(format_ctx);
    end:
        fprintf(stdout, "%s completed\n", _current_);
        return 0;
}

static int ReadRawPacket(void *data, uint8_t *buf, int buf_size)
{
    char *_current_ = "ReadRawPacket";
    fprintf(stdout, "%s starting ... \n", _current_);

    /*
    struct fstream *fdata = data;

    fprintf(stdout, "%s Retrieving struct fstream\n", _current_);

    if(feof(fdata->file_pointer))
        return AVERROR_EOF;

    fprintf(stdout, "%s EOF not reached up\n", _current_);

    uint8_t inbuf[buf_size + AV_INPUT_BUFFER_PADDING_SIZE];
    */
    size_t data_size = 0;
    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    /*
    memset(inbuf + buf_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    fprintf(stdout, "%s Input Buffer Initialized\n", _current_);

    data_size = fread(inbuf, 1, buf_size, fdata->file_pointer);

    fprintf(stdout, "%s File Chunk %d Bytes Read. Returned %d\n", _current_, buf_size, data_size);

    if (data_size == -1)
        return AVERROR(errno);

    fprintf(stdout, "%s Read data, ready to be copied\n", _current_);
    memcpy(buf, &inbuf, sizeof(uint8_t) * (buf_size + AV_INPUT_BUFFER_PADDING_SIZE));  
    */
    fprintf(stdout, "%s completed\n", _current_);

    return data_size;
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

void TestCStreamDecoderFP(const char *filename)
{
    struct fstream *fdata = (struct fstream *)malloc(sizeof(struct fstream));

    FILE *f;
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        return;
    }
    fdata->file_pointer = f;

    int(*fread_raw_data)(void *, uint8_t *, int) = ReadRawPacket;
    int r = CStreamDecoderFP(fdata, fread_raw_data);
    fprintf(stdout, "CStreamDecoderFP returned %d\n", r);

    fclose(f);
}
