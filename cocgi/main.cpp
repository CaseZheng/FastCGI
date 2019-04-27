#include <iostream>
#include <string>
#include "cocgi.h"
#include "backend.h"

using namespace std;

int main(int argc,char *argv[])
{
    if(argc<5)
    {
        std::cerr << "Usage:" << std::endl
            << "cocgi [IP] [PORT] [PROCESS_COUNT] [TASK_COUNT] " << std::endl
            << "cocgi [IP] [PORT] [PROCESS_COUNT] [TASK_COUNT] -d #daemonize" << std::endl;
        return -1;
    }
    const char *ip = argv[1];
    int port = atoi( argv[2] );
    int cnt = atoi( argv[3] );
    int proccnt = atoi( argv[4] );
    bool deamonize = argc >= 6 && strcmp(argv[5], "-d") == 0;

    CCocgiServer oCocgiServer(std::string(ip), (unsigned short)port, (unsigned short)proccnt, (unsigned short)cnt, BackendProc::PrintRequest, (void *)NULL);
    if(!oCocgiServer.Init())
    {
        std::cerr << "CocgiServer Init failure" << std::endl;
        return -1;
    }
    if(!oCocgiServer.Run(deamonize))
    {
        std::cerr << "CocgiServer Run failure" << std::endl;
        return -1;
    }

    return 0;
}
