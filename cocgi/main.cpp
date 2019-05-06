#include <iostream>
#include <string>
#include <memory>
#include <sys/types.h>
#include <unistd.h>
#include "sstream"
#include "cocgi.h"
#include "backend.h"
#include "log.h"

using namespace std;

void PrintLog(ECocgiLogLevel eLogLevel, const char *pFile, const char *pFunc, int iLine, const std::string& strLog)
{
    switch(eLogLevel)
    {
        case COCGI_DEBUG:
            LOG_CO_PRINT(debug, pFile, pFunc, iLine, strLog);
            break;
        case COCGI_INFO:
            LOG_CO_PRINT(info, pFile, pFunc, iLine, strLog);
            break;
        case COCGI_WARN:
            LOG_CO_PRINT(warning, pFile, pFunc, iLine, strLog);
            break;
        case COCGI_ERROR:
            LOG_CO_PRINT(error, pFile, pFunc, iLine, strLog);
            break;
        case COCGI_FATAL:
            LOG_CO_PRINT(fatal, pFile, pFunc, iLine, strLog);
            break;
        default:
            LOG_CO_PRINT(debug, pFile, pFunc, iLine, strLog);
    };
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
    CLog::InitLog("./log", tools::GetProcessName());
    CLog::SetFilter(debug);
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
        ERROR("make_shared CocgiServer failure");
        return -1;
    }
    pCocgiServer->SetLogCallBack(PrintLog);
    pCocgiServer->SetLogLevel(COCGI_ALL);
    if(!pCocgiServer->Init())
    {
        ERROR("CocgiServer Init failure");
        return -1;
    }
    if(!pCocgiServer->Run(deamonize))
    {
        ERROR("CocgiServer Run failure");
        return -1;
    }

    return 0;
}
