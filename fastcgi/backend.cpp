#include "backend.h"
#include <stdio.h> 
#include "log.h"

void BackendProc::PrintRequest(ParamMap &umParam, void *pArg, std::string &strResp)
{
    for(ParamMap::iterator it=umParam.begin(); it!=umParam.end(); ++it)
    {
        INFO("Param " << it->first << "=" << it->second);
    }

    std::string content = "OK!!";
    strResp = "Content-Type: text/html; charset=UTF-8\r\n";
    strResp += "Content-Length: "; 
    strResp += content.length();
    strResp += "\r\n\r\n";
    strResp += content; 

    return;
}

