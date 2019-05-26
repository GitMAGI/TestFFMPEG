#include <sys/time.h>

#include "StreamDecoderFP.h"
#include "TestTimeStamp.h"
#include "custom/log.h"

void fakeProcessing(struct fstream* data)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    data->timestamp = tv;
}

void TestTimeStamp(uint8_t counter)
{
    struct fstream *fdata = (struct fstream *)malloc(sizeof(struct fstream));

    for(int i=0; i<counter; i++){
        infoLog("Iterazione %d", i);
        fakeProcessing(fdata);
        long sec = fdata->timestamp.tv_sec;
        long usec = fdata->timestamp.tv_usec;
        infoLog("sec: %d", sec);
        infoLog("usec: %d", usec);
    }
}
