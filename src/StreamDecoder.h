#ifndef STREAM_DECODER_H
#define STREAM_DECODER_H

#include <libavformat/avformat.h>

int CStreamDecoder(const char *filename);
void TestCStreamDecoder(const char *filename);

#endif