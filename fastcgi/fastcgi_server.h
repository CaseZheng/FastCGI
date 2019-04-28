#ifndef _FASTCGI_SERVER_H_
#define _FASTCGI_SERVER_H_

typedef std::map<std::string, std::string> ParamMap;
typedef std::string(*FastCgiCodecCallback)(ParamMap &, ParamMap &, void*);

typedef void(*LogCallBack)(ECocgiLogLevel eLogLevel, const char *pFile, int pLine, const std::string& strLog);
enum EFastcgiLogLevel{
    FASTCGI_ALL,
    FASTCGI_DEBUG,
    FASTCGI_INFO,
    FASTCGI_WARN,
    FASTCGI_ERROR,
    FASTCGI_FATAL,
    FASTCGI_OFF
};

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

class CFastCgiServer
{
public:
    CFastCgiServer(const std::string &strIp, unsigned short usPort, unsigned short usPCount,
        FastCgiCallback pCgiCallBack, void *pCgiParameter)
        : m_usProcessCount(usPCount), 
        m_usPort(usPort), 
        m_pCgiCodecCallBack(pCgiCodecCallBack),
        m_pCgiCodecParameter(pCgicodecParameter),
        m_strIp(strIp)

    virtual ~CFastCgiServer()
    {
    }

    bool Init();
    bool Run(bool bDaemonize=true);

public:
    static LogCallBack GetLogCallBack() { return m_pLogCallBack; }
    static EFastcgiLogLevel GetLogLevel() { return m_eLogLevel; }
    static void SetLogCallBack(LogCallBack pLogCallBack) { m_pLogCallBack = pLogCallBack; }
    static void SetLogLevel(EFastcgiLogLevel eLogLevel) { m_eLogLevel = eLogLevel; }

private:
    static LogCallBack m_pLogCallBack;
    static EFastcgiLogLevel m_eLogLevel;

private:
    unsigned short m_usProcessCount;
    unsigned short m_usPort;
    FastCgiCallback m_pCgiCodecCallBack;
    void * m_pCgiCodecParameter;
    std::string m_strIp;
};

#endif
