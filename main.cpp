#include "AVCore.h"

//main for test
int main()
{
    //creat AVFormatContext for demux
    Demuxer* dm = new Demuxer();
    dm->open("C:/Users/lyl/Videos/test.mp4");
    
    //read the packet from AVFormatContext
    std::shared_ptr<Packet> pP = std::make_shared<Packet>();
    dm->read(pP);

    //creat AVCodecContext for decodec
    Decoder* dc = new Decoder();


    //something error happen
    dc->init(dm->getStreamCodecPar(dm->getVideoStreamIndex()));
    dc->sendPacket(pP);

    //creat the frame to hold the data
    std::shared_ptr<Frame> pF = std::make_shared<Frame>();
    dc->recvFrame(pF);
    
    pF->videoPrint();

    dc->close();
    dm->close();
    return 0;
}