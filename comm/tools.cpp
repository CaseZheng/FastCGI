#include <boost/asio.hpp>  
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/log/attributes/current_process_name.hpp>

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
