#include <stdio.h>
#include <inttypes.h>
#include <libavformat/avformat.h>

#include "custom/log.h"
#include "DecodingUtility.h"
#include "StreamDecoderFP.h"

#define BUFSIZE 0x10000

int CStreamDecoderFP(void *data, int(*read_raw_data)(void *, uint8_t *, int)){
    infoLog("Starting ...");

    // 1. AV Format Context Allocation
    AVFormatContext *format_ctx = avformat_alloc_context();
    if (!format_ctx) {
        errorLog("Could not allocate format context");
        goto end;
    }
    infoLog("%s created", "AVFormatContext");

    // 2. Buffer Allocation
    unsigned char *buffer = av_malloc(BUFSIZE);
    if (!buffer) {
        errorLog("Could not allocate buffer");        
        goto finally_free_format_ctx;
    }
    infoLog("Buffer Allocated");

    // 3. AVIO Context Allocation
    int (*read_packet)(void *, uint8_t *, int) = ReadRawPacket;
    //Convert Data to the Struct stream
    struct fstream *stream = data;
    AVIOContext *avio_ctx = avio_alloc_context(buffer, BUFSIZE, 0, stream, read_packet, NULL, NULL);      
    if (!avio_ctx) {
        errorLog("Could not allocate avio context");
        av_free(buffer);
        goto finally_free_format_ctx;
    }
    infoLog("%s created", "AVIOContext");
    format_ctx->pb = avio_ctx;
    infoLog("%s assigned to %s", "AVIOContext", "AVFormatContext");

    // 4. 
    if (avformat_open_input(&format_ctx, NULL, NULL, NULL) < 0) {
        errorLog("Could not open video stream");
        goto finally_free_avio_ctx;
    }
    infoLog("%s input opened", "AVFormatContext");

    // 5.
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        errorLog("H.264 decoder not found");
        goto end;
    }
    infoLog("%s decoder H264 found", "AVCodec");

    // 6.
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        errorLog("Could not allocate decoder context. Error Code %d", AVERROR(errno));
        goto finally_close_input;
    }
    infoLog("%s allocated", "AVCodecContext");

    // 7. 
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        errorLog("Could not open codec");
        avcodec_free_context(&codec_ctx);
        goto finally_close_input;
    }
    infoLog("%s opened", "AVCodecContext");

    // 8.
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    infoLog("Entering in Loop!"); 
    while (!av_read_frame(format_ctx, &packet)) {     
        //infoLog("Going in Loop!");  
        if(packet.size == 0){
            infoLog("Got an empty Packet! Keep going");  
            continue;
        }
        uint8_t *packet_data = (uint8_t*)malloc(sizeof(uint8_t)*packet.size);
        //infoLog("Copying packet of size %d!", packet.size);  
        memcpy(packet_data, packet.data, packet.size);    
        //infoLog("Packet duplicated!");  

        struct timeval tv;
        gettimeofday(&tv, NULL);

        stream->packet_data = packet_data;
        stream->packet_data_size = packet.size;
        stream->timestamp = tv;

        //infoLog("Stream Object Updated!");  
        
        //infoLog("Allocating AVFrame!");  
        AVFrame *frame = av_frame_alloc();
        //infoLog("AVFrame allocated!");  
        //infoLog("Decoding Packet to Frame!");  
        DecodePacketToFrame(codec_ctx, frame, &packet);
        infoLog("Packet decoded to Frame! Frame Counter: %d", codec_ctx->frame_number);  

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
        infoLog("Completed");
        return 0;
}

static int ReadRawPacket(void *data, uint8_t *buf, int buf_size)
{
    //infoLog("Starting ... ");
    
    struct fstream *fdata = data;
    //infoLog("Retrieving struct fstream");
    if(feof(fdata->file_pointer)){
        errorLog("EOF reached up");
        return AVERROR_EOF;
    }
    //infoLog("EOF not reached up");

    uint8_t inbuf[buf_size + AV_INPUT_BUFFER_PADDING_SIZE];
    size_t data_size = 0;
    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */

    memset(inbuf + buf_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    //infoLog("%s Input Buffer Initialized");
    data_size = fread(inbuf, 1, buf_size, fdata->file_pointer);
    //infoLog("File Chunk %d Bytes Read. Returned %d", buf_size, data_size);
    if(data_size == 0){
        errorLog("Got an empty file chunk");
        return 0;
    }
    if (data_size == -1){
        errorLog("Got Error on File Reading. Error: %d", errno);
        return AVERROR(errno);
    }
    //infoLog("Read data, ready to be copied");    
    //memcpy(buf, &inbuf, data_size);   
    memcpy(buf, &inbuf, buf_size);
    //infoLog("Completed");

    return data_size;
}

void TestCStreamDecoderFP(const char *filename)
{
    struct fstream *fdata = (struct fstream *)malloc(sizeof(struct fstream));

    FILE *f;
    f = fopen(filename, "rb");
    if (!f) {
        errorLog("Could not open %s", filename);
        return;
    }
    infoLog("File %s opened", filename);
    fdata->file_pointer = f;

    int(*fread_raw_data)(void *, uint8_t *, int) = ReadRawPacket;
    int r = CStreamDecoderFP(fdata, fread_raw_data);
    infoLog("CStreamDecoderFP returned %d", r);

    fclose(f);
    infoLog("File %s closed", filename);
}
