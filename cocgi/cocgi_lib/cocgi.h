#ifndef COCGI_H
#define COCGI_H
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include "co_routine.h"
#include "fastcgi.h"

enum ECocgiLogLevel{
    COCGI_ALL,
    COCGI_DEBUG,
    COCGI_INFO,
    COCGI_WARN,
    COCGI_ERROR,
    COCGI_FATAL,
    COCGI_OFF
};

typedef void(*LogCallBack)(ECocgiLogLevel eLogLevel, const char *pFile, int pLine, const std::string& strLog);

#define COCGI_PRINT_LOG(COCGI_LEVEL, msg) \
    if(NULL!=CCocgiServer::GetLogCallBack() && CCocgiServer::GetLogLevel()<=COCGI_LEVEL) \
    { \
        std::stringstream ssLog; \
        ssLog << msg; \
        CCocgiServer::GetLogCallBack()(COCGI_LEVEL, __FILE__, __LINE__, ssLog.str()); \
    }

#define CO_DEBUG(msg) COCGI_PRINT_LOG(COCGI_DEBUG, msg)
#define CO_INFO(msg)  COCGI_PRINT_LOG(COCGI_INFO,  msg)
#define CO_WARN(msg)  COCGI_PRINT_LOG(COCGI_WARN,  msg)
#define CO_ERROR(msg) COCGI_PRINT_LOG(COCGI_ERROR, msg)
#define CO_FATAL(msg) COCGI_PRINT_LOG(COCGI_FATAL, msg)

class CCocgiServer;
struct CocgiTask
{
    int iFd;
    unsigned uCoId;
    std::shared_ptr<stCoRoutine_t> pCoRoutine;
    std::weak_ptr<CCocgiServer> pWeakCocgiServer;
    std::shared_ptr<FastCgiCodec> pFastCgiCodec;
};

class CCocgiServer : public std::enable_shared_from_this<CCocgiServer>
{
public:
    CCocgiServer(const std::string &strIp, unsigned short usPort, unsigned short usPCount, unsigned short usCCount,
            FastCgiCodecCallback pCgiCodecCallBack, void *pCgicodecParameter)
        : m_usProcessCount(usPCount), 
        m_usCoroutineCount(usCCount), 
        m_usMaxCoroutineCount(50),
        m_usPort(usPort), 
        m_pCgiCodecCallBack(pCgiCodecCallBack),
        m_pCgiCodecParameter(pCgicodecParameter),
        m_strIp(strIp)
    {
    }
    bool Init(bool bReuse=true, unsigned short usListenNum=5);
    bool Run(bool bDaemonize=true);

public:
    static LogCallBack GetLogCallBack() { return m_pLogCallBack; }
    static ECocgiLogLevel GetLogLevel() { return m_eLogLevel; }
    static void SetLogCallBack(LogCallBack pLogCallBack) { m_pLogCallBack = pLogCallBack; }
    static void SetLogLevel(ECocgiLogLevel eLogLevel) { m_eLogLevel = eLogLevel; }

private:
    int SetNonBlock(int iSock);
    int CreateTcpSocket(unsigned short shPort, const char *pszIP, bool bReuse);
    void SetAddr(const char *pszIP, unsigned short shPort, struct sockaddr_in &addr);
    bool CreateCocgiTask();
    void *AcceptRoutine();
    void *RealReadwriteRoutine(CocgiTask *pCocgiTask);

private:
    static void *AcceptRoutine(void *pArg);
    static void *ReadwriteRoutine(void *pArg);

private:
    static LogCallBack m_pLogCallBack;
    static ECocgiLogLevel m_eLogLevel;
    static unsigned m_uCoIndex;

private:
    unsigned short m_usProcessCount;
    unsigned short m_usCoroutineCount;
    unsigned short m_usMaxCoroutineCount;
    unsigned short m_usPort;
    int m_iListenFd;
    FastCgiCodecCallback m_pCgiCodecCallBack;
    void * m_pCgiCodecParameter;
    std::string m_strIp;
    std::list<std::shared_ptr<CocgiTask> > m_lIdleTask;           //空闲的协程
    std::unordered_set<std::shared_ptr<CocgiTask> > m_usRunTask;  //正在运行的协程
    std::unordered_map<unsigned, std::shared_ptr<CocgiTask> > m_umTask;  //所有协程
    std::shared_ptr<stCoRoutine_t> m_pAcceptCoRoutine;
};

class CstCoRoutineDelete
{
public:
    void operator()(stCoRoutine_t *pCoRoutine)
    {
        CO_INFO("release stCoRoutine_t");
        co_release(pCoRoutine);
    }
};

#endif
