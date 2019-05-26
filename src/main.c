#include <stdio.h>

#include "custom/log.h"

#include "StreamDecoderFP.h"
#include "TestTimeStamp.h"
#include "StreamDecoder.h"
#include "GenericTests.h"

int main(int argc, char* argv[]){
    char * _current_ = "TestFFMPEG";

    #if defined _WIN64 || defined _WIN32
        // disable buffering, we want logs immediately
        // even line buffering (setvbuf() with mode _IOLBF) is not sufficient
        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
    #endif

    infoLog("Starting ...");

    const char* filename = ".\\asset\\SampleVideo_360x240_10mb.mp4";
    //const char* filename = ".\\asset\\TheSimpsonsMovie-Trailer.mp4";
    
    infoLog("%s starting ...", "Function Pointer");
    TestCStreamDecoderFP(filename);
    infoLog("%s Completed", "Function Pointer");
    
    //TestTimeStamp(100);
    
    infoLog("%s starting ...", "Easy");
    TestCStreamDecoder(filename);
    infoLog("%s Completed", "Easy");
    
    //GeneralTest();

    infoLog("Completed!");
    return 0;
}