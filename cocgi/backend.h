#ifndef _BACKEND_PROC_H__
#define _BACKEND_PROC_H__
#include "type.h"

class BackendProc
{
  public:
    static std::string PrintRequest(ParamMap &qmap, ParamMap &header, void *pArg);
};

#endif /*_BACKEND_PROC_H__*/

