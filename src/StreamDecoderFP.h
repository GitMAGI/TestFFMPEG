#ifndef STREAM_DECODER_FP_H
#define STREAM_DECODER_FP_H

#include <inttypes.h>
#include <libavformat/avformat.h>
#include <sys/time.h>

#if defined _WIN64 || defined _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
#else
    #include <unistd.h>
    typedef int socket_t;
#endif

struct fstream {
    FILE *file_pointer;
    uint8_t *video_buffer;    
    uint8_t *packet_data;
    uint32_t packet_data_size;
    uint8_t *frame_data;
    uint32_t farme_data_size;
    struct timeval timestamp;
};

static int ReadRawPacket(void *, uint8_t *, int);
int CStreamDecoderFP(void *data, int(*read_raw_data)(void *, uint8_t *, int));
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

void TestCStreamDecoderFP(const char *filename);

#endif