#include <stdio.h>

#include "StreamDecoderFP.h"
#include "TestTimeStamp.h"
#include "StreamDecoder.h"
#include "GenericTests.h"

int main(int argc, char* argv[]){
    char * _current_ = "TestFFMPEG";
    fprintf(stdout, "%s starting ...\n", _current_);

    const char* filename = ".\\asset\\SampleVideo_360x240_10mb.mp4";
    //TestCStreamDecoderFP(filename);
    //TestTimeStamp(100);
    TestCStreamDecoder(filename);
    //GeneralTest();

    fprintf(stdout, "%s completed!\n", _current_);
    return 0;
}