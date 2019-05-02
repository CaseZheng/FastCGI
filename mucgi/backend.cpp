#include "backend.h"
#include <stdio.h> 
#include "log.h"

std::string BackendProc::printRequest(SSMap &qmap, SSMap &header)
{
    for (SSMap::iterator it = qmap.begin(); it != qmap.end(); it++)
    {
        DEBUG("Param: " << it->first << "=" << it->second.c_str());
    }

    for (SSMap::iterator it = header.begin(); it != header.end(); it++)
    {
        DEBUG("Header: " << it->first << "=" << it->second.c_str());
    }

    std::string content = "OK!!";
    std::string resp = "Content-Type: text/html; charset=UTF-8\r\n";
    resp += "Content-Length: "; 
    resp += content.length();
    resp += "\r\n\r\n";
    resp += content; 

    return resp;
}

