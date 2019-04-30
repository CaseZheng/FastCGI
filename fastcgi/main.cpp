#include "fastcgi_server.h"
#include "log.h"
#include "backend.h"

void PrintLog(EFastCgiLogLevel eLogLevel, const char *pFile, int iLine, const std::string& strLog)
{
    switch(eLogLevel)
    {
        case FASTCGI_DEBUG:
            LOG_CO_PRINT(debug, pFile, iLine, strLog);
            break;
        case FASTCGI_INFO:
            LOG_CO_PRINT(info, pFile, iLine, strLog);
            break;
        case FASTCGI_WARN:
            LOG_CO_PRINT(warning, pFile, iLine, strLog);
            break;
        case FASTCGI_ERROR:
            LOG_CO_PRINT(error, pFile, iLine, strLog);
            break;
        case FASTCGI_FATAL:
            LOG_CO_PRINT(fatal, pFile, iLine, strLog);
            break;
        default:
            LOG_CO_PRINT(debug, pFile, iLine, strLog);
    };
}


int main(int argc, char *argv[])
{
    if(argc<5)
    {
        std::cerr << "Usage:" << std::endl
            << "fastcgi [IP] [PORT] [PROCESS_COUNT] " << std::endl
            << "fastcgi [IP] [PORT] [PROCESS_COUNT] -d #daemonize" << std::endl;
        return -1;
    }
    CLog::InitLog("./log", tools::GetProcessName());
    CLog::SetFilter(debug);
    const char *ip = argv[1];
    int port = atoi( argv[2] );
    int proccnt = atoi( argv[3] );
    bool deamonize = argc >= 5 && strcmp(argv[4], "-d") == 0;
    
    CFastCgiServer oCgiServer(ip, port, proccnt, BackendProc::PrintRequest, (void*)NULL);
    oCgiServer.SetLogCallBack(PrintLog);
    oCgiServer.SetLogLevel(FASTCGI_ALL);
    if(!oCgiServer.Init())
    {
        FAST_FATAL("Init failure");
        return -1;
    }

    if(!oCgiServer.Run(deamonize))
    {
        return -1;
    }

    return 0;
}
