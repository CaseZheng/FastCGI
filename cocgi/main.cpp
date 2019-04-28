#include <iostream>
#include <string>
#include <memory>
#include <sys/types.h>
#include <unistd.h>
#include "sstream"
#include "cocgi.h"
#include "backend.h"

using namespace std;

void PrintLog(ECocgiLogLevel eLogLevel, const char *pFile, int pLine, const std::string& strLog)
{
    std::stringstream ssLog;
    ssLog << getpid() << "|" << eLogLevel << "|" << pFile << "|" << pLine << "|" << strLog;
    std::cout << ssLog.str() << std::endl;
    fflush(stdout);
}

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

    std::shared_ptr<CCocgiServer> pCocgiServer = std::make_shared<CCocgiServer>(std::string(ip), 
                                (unsigned short)port, (unsigned short)proccnt, (unsigned short)cnt, 
                                            BackendProc::PrintRequest, (void *)NULL);
    if(NULL == pCocgiServer)
    {
        std::cerr << "make_shared CocgiServer failure" << std::endl;
        return -1;
    }
    pCocgiServer->SetLogCallBack(PrintLog);
    pCocgiServer->SetLogLevel(COCGI_ALL);
    if(!pCocgiServer->Init())
    {
        std::cerr << "CocgiServer Init failure" << std::endl;
        return -1;
    }
    if(!pCocgiServer->Run(deamonize))
    {
        std::cerr << "CocgiServer Run failure" << std::endl;
        return -1;
    }

    return 0;
}
