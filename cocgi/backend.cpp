#include "backend.h"
#include <stdio.h> 
#include "log.h"

void BackendProc::PrintRequest(ParamMap &qmap, ParamMap &header, void *pArg, string &strResp)
{
    for (ParamMap::iterator it = qmap.begin(); it != qmap.end(); it++)
    {
        DEBUG("Param " << it->first << "=" << it->second);
    }

    for (ParamMap::iterator it = header.begin(); it != header.end(); it++)
    {
        DEBUG("Header " << it->first << "=" << it->second);
    }

    std::string content = "OK!!";
    strResp = "Content-Type: text/html; charset=UTF-8\r\n";
    strResp += "Content-Length: "; 
    strResp += content.length();
    strResp += "\r\n\r\n";
    strResp += content; 
}

