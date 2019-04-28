#include "fastcgi_server.h"
#include "log.h"

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
    int cnt = atoi( argv[3] );
    int proccnt = atoi( argv[4] );
    bool deamonize = argc >= 6 && strcmp(argv[5], "-d") == 0;


    return 0;
}
