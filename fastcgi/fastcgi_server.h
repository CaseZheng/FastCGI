#ifndef _FASTCGI_SERVER_H_
#define _FASTCGI_SERVER_H_

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string, std::string> ParamMap;
typedef void(*FastCgiCallback)(ParamMap &, void*, std::string &);

enum EFastCgiLogLevel{
    FASTCGI_ALL,
    FASTCGI_DEBUG,
    FASTCGI_INFO,
    FASTCGI_WARN,
    FASTCGI_ERROR,
    FASTCGI_FATAL,
    FASTCGI_OFF
};
typedef void(*LogCallBack)(EFastCgiLogLevel eLogLevel, const char *pFile, int pLine, const std::string& strLog);

#define FASTCGI_PRINT_LOG(FASTCGI_LEVEL, msg) \
    if(NULL!=CFastCgiServer::GetLogCallBack() && CFastCgiServer::GetLogLevel()<=FASTCGI_LEVEL) \
    { \
        std::stringstream ssLog; \
        ssLog << msg; \
        CFastCgiServer::GetLogCallBack()(FASTCGI_LEVEL, __FILE__, __LINE__, ssLog.str()); \
    }

#define FAST_DEBUG(msg) FASTCGI_PRINT_LOG(FASTCGI_DEBUG, msg)
#define FAST_INFO(msg)  FASTCGI_PRINT_LOG(FASTCGI_INFO,  msg)
#define FAST_WARN(msg)  FASTCGI_PRINT_LOG(FASTCGI_WARN,  msg)
#define FAST_ERROR(msg) FASTCGI_PRINT_LOG(FASTCGI_ERROR, msg)
#define FAST_FATAL(msg) FASTCGI_PRINT_LOG(FASTCGI_FATAL, msg)

namespace fastcgi {
const static std::string STR_CONTENT_LENGTH      = "CONTENT_LENGTH";
const static std::string STR_CONTENT_TYPE        = "CONTENT_TYPE";
const static std::string STR_DOCUMENT_ROOT       = "DOCUMENT_ROOT";
const static std::string STR_DOCUMENT_URI        = "DOCUMENT_URI";
const static std::string STR_GATEWAY_INTERFACE   = "GATEWAY_INTERFACE";
const static std::string STR_HTTP_ACCEPT         = "HTTP_ACCEPT";
const static std::string STR_HTTP_CONTENT_LENGTH = "HTTP_CONTENT_LENGTH";
const static std::string STR_HTTP_CONTENT_TYPE   = "HTTP_CONTENT_TYPE";
const static std::string STR_HTTP_COOKIE         = "HTTP_COOKIE";
const static std::string STR_HTTP_HOST           = "HTTP_HOST";
const static std::string STR_HTTP_USER_AGENT     = "HTTP_USER_AGENT";
const static std::string STR_QUERY_STRING        = "QUERY_STRING";
const static std::string STR_REDIRECT_STATUS     = "REDIRECT_STATUS";
const static std::string STR_REMOTE_ADDR         = "REMOTE_ADDR";
const static std::string STR_REMOTE_PORT         = "REMOTE_PORT";
const static std::string STR_REQUEST_METHOD      = "REQUEST_METHOD";
const static std::string STR_REQUEST_SCHEME      = "REQUEST_SCHEME";
const static std::string STR_REQUEST_URI         = "REQUEST_URI";
const static std::string STR_SCRIPT_NAME         = "SCRIPT_NAME";
const static std::string STR_SERVER_ADDR         = "SERVER_ADDR";
const static std::string STR_SERVER_NAME         = "SERVER_NAME";
const static std::string STR_SERVER_PORT         = "SERVER_PORT";
const static std::string STR_SERVER_PROTOCOL     = "SERVER_PROTOCOL";
const static std::string STR_SERVER_SOFTWARE     = "SERVER_SOFTWARE";
}


class CFastCgiServer
{
public:
    CFastCgiServer(const std::string &strIp, unsigned short usPort, unsigned short usPCount,
        FastCgiCallback pCgiCallBack, void *pCgiParameter)
        : m_usProcessCount(usPCount), 
        m_usPort(usPort), 
        m_pCgiCallBack(pCgiCallBack),
        m_pCgiParameter(pCgiParameter),
        m_strIp(strIp)
    {
    }

    virtual ~CFastCgiServer()
    {
    }

    bool Init(bool bReuse=false);
    bool Run(bool bDaemonize=true);

public:
    static LogCallBack GetLogCallBack() { return m_pLogCallBack; }
    static EFastCgiLogLevel GetLogLevel() { return m_eLogLevel; }
    static void SetLogCallBack(LogCallBack pLogCallBack) { m_pLogCallBack = pLogCallBack; }
    static void SetLogLevel(EFastCgiLogLevel eLogLevel) { m_eLogLevel = eLogLevel; }

private:
	void ParseParam();
    int SetNonBlock(int iSock);
    int CreateTcpSocket(unsigned short shPort, const char *pszIP, bool bReuse);
    void SetAddr(const char *pszIP, unsigned short shPort, struct sockaddr_in &addr);

    inline void clear()
    {
        m_strOutPacket.clear();
        m_umParam.clear();
    }

	inline void ParseParamItem(const std::string &strKey)
	{
        char *pValue = getenv(strKey.c_str());
        m_umParam[strKey] = (NULL == pValue) ? "" : pValue;
	}

private:
    static LogCallBack m_pLogCallBack;
    static EFastCgiLogLevel m_eLogLevel;

private:
    unsigned short m_usProcessCount;
    unsigned short m_usPort;
    int m_iListenFd;
    FastCgiCallback m_pCgiCallBack;
    void * m_pCgiParameter;
    std::string m_strIp;
    std::string m_strOutPacket;
    std::unordered_map<std::string, std::string> m_umParam;
};

#endif
