#include <boost/asio.hpp>  
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/log/attributes/current_process_name.hpp>
#include <signal.h>
#include <execinfo.h>
#include <fcntl.h>

#include "tools.h"
#include "log.h"

string tools::GetHostNameIp() 
{  
    using boost::asio::ip::tcp;  
    boost::asio::io_service io_service;  
    tcp::resolver resolver(io_service);  
    tcp::resolver::query query(boost::asio::ip::host_name(), "");  
    tcp::resolver::iterator iter = resolver.resolve(query);  
    tcp::resolver::iterator end;
    if(iter != end)
    {
        tcp::endpoint ep = *iter;  
        return ep.address().to_string();
    }
    return string("");
}  

string tools::GetFileNameFromFilePath(const string &strFilePath)
{
    using boost::filesystem::path;
    boost::filesystem::path boostPath(strFilePath);
    return boostPath.filename().generic_string();
}

string tools::GetRandomString()
{
    return to_string(boost::uuids::random_generator()());
}

string tools::GetProcessName()
{
    return boost::log::aux::get_process_name();
}

string tools::PrintHex(const vector<char>& buff)
{
    stringstream ss;
    for(unsigned i=0; i<buff.size(); ++i)
    {
        if(i%16 == 0)
        {
            ss << endl;
        }
        if(i%4 == 0)
        {
           ss << "0x";
        }
        ss << setfill('0') << setw(2) << setiosflags(ios::uppercase) << hex << (int)(*((unsigned char*)(&buff[i])));
        if(i%4 == 3)
        {
            ss << "  ";
        }
    }
    return ss.str();
}

int tools::GetTimeStamps()
{
    return time(NULL);
}

void tools::SaveBackTrace(int sig)
{
    FATAL("CoreTime:" << GetTimeStamps() << " SignalId:" << sig);
    void* DumpArray[1000];
    int nSize = backtrace(DumpArray, 1000);
    ERROR("backtrace rank:" << nSize << " addr2line -f -C -e 应用名 地址");
    if(nSize > 0)
    {
        char** symbols = backtrace_symbols(DumpArray, nSize);
        if (symbols != NULL)
        {
            for (int i=0; i<nSize; ++i)
            {
                ERROR(to_string(nSize-i) << ": " << symbols[i]);
            }
            free(symbols);
        }
    }
    exit(1);
}

void tools::SetSignal()
{
    signal(SIGFPE   , SaveBackTrace); // 浮点异常
    signal(SIGILL   , SaveBackTrace); // 非法指令
    signal(SIGBUS   , SaveBackTrace); // 总线错误
    signal(SIGABRT  , SaveBackTrace); // 来自abort函数的终止信号
    signal(SIGSEGV  , SaveBackTrace); // 无效的存储器引用(段错误)
    signal(SIGIOT   , SaveBackTrace); // 执行I/O自陷
}
