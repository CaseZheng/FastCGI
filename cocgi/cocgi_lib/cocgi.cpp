#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sstream>

#include "cocgi.h"
#include "fastcgi.h"
#include "backend.h"
#include "co_routine_inner.h"

#define MAX_RECYCLE_COUNT 60

int co_accept(int fd, struct sockaddr *addr, socklen_t *len);

LogCallBack CCocgiServer::m_pLogCallBack = NULL;
ECocgiLogLevel CCocgiServer::m_eLogLevel = COCGI_OFF;

int CCocgiServer::SetNonBlock(int iSock)
{
    int iFlags;
    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

int CCocgiServer::CreateTcpSocket(unsigned short shPort, const char *pszIP, bool bReuse)
{
    CO_DEBUG("start create tcp socket");
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CO_DEBUG("create tcp socket fd:" << fd);
    if(fd >= 0)
    {
        if(shPort != 0)
        {
            if(bReuse)
            {
                int nReuseAddr = 1;
                setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&nReuseAddr,sizeof(nReuseAddr));
            }

            int val =1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val))<0)
            {
                CO_WARN("set SO_REUSEPORT failure");
            }   

            struct sockaddr_in addr;
            SetAddr(pszIP, shPort, addr);
            int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
            if(ret != 0)
            {
                close(fd);
                CO_FATAL("bind failure");
                return -1;
            }
        }
    }
    return fd;
}

void CCocgiServer::SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
{
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(shPort);
    int nIP = 0;
    if( !pszIP || '\0' == *pszIP   
        || 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
        || 0 == strcmp(pszIP,"*") 
      )
    {
        nIP = htonl(INADDR_ANY);
    }
    else
    {
        nIP = inet_addr(pszIP);
    }
    addr.sin_addr.s_addr = nIP;
}

void *CCocgiServer::ReadwriteRoutine(std::shared_ptr<CocgiTask> &pCocgiTask)
{
    if(NULL == pCocgiTask)
    {
        CO_FATAL("CocgiTask is NULL");
        return NULL;
    }
    for(;;)
    {
        if(-1 == pCocgiTask->iFd)
        {
            pCocgiTask->pFastCgiCodec->clear();
            m_lIdleTask.push_front(pCocgiTask);
            m_umRunTask.erase(pCocgiTask);
            co_yield_ct();
            continue;
        }

        int iFd = pCocgiTask->iFd;
        pCocgiTask->iFd = -1;
        int recycle = 0;
        pCocgiTask->pFastCgiCodec->clear();
        for(;;)
        {
            struct pollfd pf = { 0 };
            pf.fd = iFd;
            pf.events = (POLLIN|POLLERR|POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);
            int ret = pCocgiTask->pFastCgiCodec->readData(iFd);
            if (ret == ERR_OK)
            {
                recycle = 0;
                continue;
            }
            else if (ret == ERR_SOCKET_EAGAIN)
            {
                // Receive Timeout 60 second.
                if (recycle++ >= MAX_RECYCLE_COUNT)
                {
                    close(iFd);
                    break;
                }
            }
            else
            {
                // accept_routine->SetNonBlock(fd) cause EAGAIN, we should continue
                close(iFd);
                break;
            }
        }
    }
    return NULL;
}

void *CCocgiServer::ReadwriteRoutine(void *pArg)
{
    CO_DEBUG("start read write routine");
    co_enable_hook_sys();
    std::shared_ptr<CocgiTask> *ppCocgiTask = (std::shared_ptr<CocgiTask>*)(pArg);
    std::shared_ptr<CocgiTask> pCocgiTask = *ppCocgiTask;
    if(NULL == pCocgiTask)
    {
        CO_DEBUG("CocgiTask is NULL");
        return NULL;
    }
    if(pCocgiTask->pWeakCocgiServer.expired())
    {
        CO_FATAL("Cocgiserver is expired");
        return NULL;
    }
    auto pCocgiServer = pCocgiTask->pWeakCocgiServer.lock();
    return pCocgiServer->ReadwriteRoutine(pCocgiTask);
}
void *CCocgiServer::AcceptRoutine()
{
    for(;;)
    {
        if(m_lIdleTask.empty()
                && (m_lIdleTask.size()+m_umRunTask.size() < m_usMaxCoroutineCount))
        {
            CO_DEBUG("No idle coroutine, create a coroutine");
            if(!CreateCocgiTask())
            {
                CO_ERROR("create cocgitask failure");
                continue;
            }
        }
        if(m_lIdleTask.empty())
        {
            CO_INFO("idel task empty");
            continue;
        }
        struct sockaddr_in addr; //maybe sockaddr_un;
        memset( &addr,0,sizeof(addr) );
        socklen_t len = sizeof(addr);

        int iFd = co_accept(m_iListenFd, (struct sockaddr *)&addr, &len);
        if(iFd < 0)
        {
            struct pollfd pf = { 0 };
            pf.fd = m_iListenFd;
            pf.events = (POLLIN|POLLERR|POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);
            continue;
        }
        SetNonBlock(iFd);
        std::shared_ptr<CocgiTask> pCocgiTask = m_lIdleTask.front();
        pCocgiTask->iFd = iFd;
        m_lIdleTask.pop_front();
        m_umRunTask.insert(pCocgiTask);
        co_resume(pCocgiTask->pCoRoutine.get());
    }

    return NULL;
}
void *CCocgiServer::AcceptRoutine(void *pArg)
{
    CO_DEBUG("Start AcceptRoutine");
    co_enable_hook_sys();
    CCocgiServer *pCocgiServer = static_cast<CCocgiServer*>(pArg);
    if(NULL == pCocgiServer)
    {
        CO_FATAL("CocgiServer is NULL");
        return NULL;
    }
    return pCocgiServer->AcceptRoutine();
}

bool CCocgiServer::Init(bool bReuse, unsigned short usListenNum)
{
    if(m_strIp.empty())
    {
        CO_FATAL("Ip is empty");
        return false;
    }
    if(0 == m_usPort)
    {
        CO_FATAL("Port is 0");
        return false;
    }
    if(0 == m_usProcessCount)
    {
        CO_FATAL("Process Count is 0");
        return false;
    }
    if(0 == m_usCoroutineCount)
    {
        CO_FATAL("Corountine Count is 0");
        return false;
    }
    if(NULL == m_pCgiCodecCallBack)
    {
        CO_FATAL("CgiCodesCallBack is NULL");
        return false;
    }
    CO_DEBUG("Ip:" << m_strIp << " Port:" << m_usPort);
    m_iListenFd = CreateTcpSocket(m_usPort, m_strIp.c_str(), bReuse);
    if(m_iListenFd < 0)
    {
        CO_FATAL("create tcp socket failure");
        return false;
    }
    CO_DEBUG("create listen tcp socket success listenFd:" << m_iListenFd);
    if(listen(m_iListenFd, usListenNum) < 0)
    {
        CO_FATAL("listen socket failure");
        return false;
    }
    CO_DEBUG("listen tcp socket success");

    SetNonBlock(m_iListenFd);
    CO_DEBUG("set listen socket non-blocking");

    for(int i=0; i<m_usCoroutineCount; ++i)
    {
        if(!CreateCocgiTask())
        {
            CO_FATAL("create cocgi task failure");
            return false;
        }
    }
    CO_DEBUG("create coroutine success");
    CO_DEBUG("make_shared Accept Coroutine success");
    stCoRoutine_t *pAcceptCoRount = NULL;
    co_create(&pAcceptCoRount, NULL, AcceptRoutine, (void*)this);
    m_pAcceptCoRoutine = std::shared_ptr<stCoRoutine_t>(pAcceptCoRount, CstCoRoutineDelete());
    if(NULL == m_pAcceptCoRoutine)
    {
        CO_FATAL("make_shared listen stCoRoutine_t failure");
        return false;
    }

    co_resume(m_pAcceptCoRoutine.get());

    return true;
}

bool CCocgiServer::CreateCocgiTask()
{
    std::shared_ptr<CocgiTask> pCocgiTask = std::make_shared<CocgiTask>();
    if(NULL == pCocgiTask)
    {
        CO_FATAL("make_shared CocgiTask failure");
        return false;
    }
    pCocgiTask->pFastCgiCodec = std::make_shared<FastCgiCodec>(m_pCgiCodecCallBack, m_pCgiCodecParameter);
    if(NULL == pCocgiTask->pFastCgiCodec)
    {
        CO_FATAL("make_shared FastCgiCodec failure");
        return false;
    }
    pCocgiTask->pWeakCocgiServer = std::weak_ptr<CCocgiServer>(shared_from_this());
    if(pCocgiTask->pWeakCocgiServer.expired())
    {
        CO_FATAL("weak_ptr CocgiServer expired");
        return false;
    }
    pCocgiTask->iFd = -1;
    stCoRoutine_t *pReadWriteCoRoutine = NULL;
    co_create(&pReadWriteCoRoutine, NULL, ReadwriteRoutine, (void*)&pCocgiTask);
    pCocgiTask->pCoRoutine = std::shared_ptr<stCoRoutine_t>(pReadWriteCoRoutine, CstCoRoutineDelete());
    if(NULL == pCocgiTask->pCoRoutine)
    {
        CO_FATAL("make_shared stCoRoutine_t failure");
        return false;
    }
    m_lIdleTask.push_front(pCocgiTask);

    return true;
}

bool CCocgiServer::Run(bool bDaemonize)
{
    for(int i=0; i<m_usProcessCount; ++i)
    {
        pid_t pid = fork();
        if(pid > 0)
        {
            CO_DEBUG("Fork child process Pid:" << pid);
            continue;
        }
        else if(pid < 0)
        {
            CO_FATAL("Fork child failure");
            break;
        }
        else
        {
            co_eventloop(co_get_epoll_ct(), 0, 0);
            exit(0);
        }
    }

    if(!bDaemonize)
    {
        CO_INFO("Non-daemon process waiting");
        wait(NULL);
    }
    return true;
}
