#ifndef _BACKEND_PROC_H__
#define _BACKEND_PROC_H__

#include <string>
#include "fastcgi_server.h"

class BackendProc
{
  public:
    static void PrintRequest(ParamMap &umParam, void *pArg, std::string &strResp);
};

#endif /*_BACKEND_PROC_H__*/

