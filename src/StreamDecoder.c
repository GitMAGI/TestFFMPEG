#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "custom/log.h"
#include "DecodingUtility.h"
#include "StreamDecoder.h"

#define BUFSIZE 0x10000

int CStreamDecoder(const char *filename){
    infoLog("Starting ...");
    
    // 1.
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    //AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        errorLog("H.264 decoder not found");
        goto end;
    }
    infoLog("%s decoder H264 found", "AVCodec");

    // 2. 
    AVCodecParserContext *parser = av_parser_init(codec->id);
    if (!parser) {
        errorLog("Parser not found");
        goto end;
    }
    infoLog("%s initiated", "AVCodecParserContext");

    // 3.
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        errorLog("Could not allocate video codec context");
        goto finally_close_parser;
    }
    infoLog("%s allocated", "AVCodecContext");

    // 4.
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    /* open it */
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        errorLog("Could not open codec");
        goto finally_free_avcodec_ctx;
    }
    infoLog("%s opened", "AVCodec");

    FILE *f;
    f = fopen(filename, "rb");
    if (!f) {
        errorLog("Could not open %s", filename);
        goto finally_free_avcodec_ctx;
    }
    infoLog("File %s opened", filename);
    uint8_t inbuf[BUFSIZE + AV_INPUT_BUFFER_PADDING_SIZE];    

    AVPacket *packet = av_packet_alloc();
    if (!packet){
        errorLog("Could not allocate video packet");
        goto finally_close_file;
    }
    infoLog("%s allocated", "AVPacket");
    
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        errorLog("Could not allocate video frame");
        av_packet_free(&packet);
        goto finally_close_file;
    }
    infoLog("%s allocated", "AVFrame");
    
    while (!feof(f)) {
        /* read raw data from the input file */
        size_t data_size = fread(inbuf, 1, BUFSIZE, f);
        //infoLog("Read %d bytes of dta from file", data_size);
        if (!data_size)
            break;    
        uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * (BUFSIZE + AV_INPUT_BUFFER_PADDING_SIZE));
        //infoLog("Copying data from buffer");
        memcpy(data, inbuf, data_size);
        //infoLog("Data copied successfully");
        while (data_size > 0) {
            //infoLog("Parsing data to Packet");
            int ret = av_parser_parse2(parser, codec_ctx, &packet->data, &packet->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                errno = 0;
                errorLog("Error while parsing. AVError Code: %d. AVError Description: %s", AVERROR(ret), av_err2str(ret));
                goto finally_free_packet;
            }
            data += ret;
            data_size -= ret;
            if (packet->size)
                DecodePacketToFrame(codec_ctx, frame, packet);
        }
    }    
    /* flush the decoder */
    //DecodePacketToFrame(codec_ctx, frame, NULL);

    finally_free_packet:
        av_packet_free(&packet);
        infoLog("%s deaallocated", "AVPacket");
    finally_free_frame:
        av_frame_free(&frame);
        infoLog("%s deallocated", "AVFrame");
    finally_close_file:
        fclose(f);
        infoLog("File %s closed", filename);
    finally_free_avcodec_ctx:
        avcodec_free_context(&codec_ctx);
        infoLog("%s deallocated", "AVCodecContext");
    finally_close_parser:
        av_parser_close(parser);
        infoLog("%s closed", "AVCodecParserContext");
    end:
        infoLog("Completed");
        return 0;
}

void TestCStreamDecoder(const char *filename)
{    
    int r = CStreamDecoder(filename);
    infoLog("CStreamDecoder returned %d", r);
}