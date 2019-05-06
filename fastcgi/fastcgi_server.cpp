#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sstream>

#include "fastcgi_server.h"
#include "fcgi_stdio.h"

#define FCGI_LISTENSOCK_FILENO 0

LogCallBack CFastCgiServer::m_pLogCallBack = NULL;
EFastCgiLogLevel CFastCgiServer::m_eLogLevel = FASTCGI_OFF;

int CFastCgiServer::CreateTcpSocket(unsigned short shPort, const char *pszIP, bool bReuse)
{
    FAST_DEBUG("start create tcp socket");
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    FAST_DEBUG("create tcp socket fd:" << fd);
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
                FAST_WARN("set SO_REUSEPORT failure");
            }   

            struct sockaddr_in addr;
            SetAddr(pszIP, shPort, addr);
            int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
            if(ret != 0)
            {
                close(fd);
                FAST_FATAL("bind failure");
                return -1;
            }
        }
    }
    return fd;
}

void CFastCgiServer::SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
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


bool CFastCgiServer::Init(bool bReuse)
{
    if(m_strIp.empty())
    {
        FAST_FATAL("Ip is empty");
        return false;
    }
    if(0 == m_usPort)
    {
        FAST_FATAL("Port is 0");
        return false;
    }
    if(0 == m_usProcessCount)
    {
        FAST_FATAL("Process Count is 0");
        return false;
    }
    if(NULL == m_pCgiCallBack)
    {
        FAST_FATAL("CgiCallBack is NULL");
        return false;
    }
    FAST_DEBUG("Ip:" << m_strIp << " Port:" << m_usPort);
    m_iListenFd = CreateTcpSocket(m_usPort, m_strIp.c_str(), bReuse);
    if(m_iListenFd < 0)
    {
        FAST_FATAL("create tcp socket failure");
        return false;
    }
    FAST_DEBUG("create listen tcp socket success listenFd:" << m_iListenFd);
    if(listen(m_iListenFd, 5) < 0)
    {
        FAST_FATAL("listen socket failure");
        return false;
    }
    FAST_DEBUG("listen tcp socket success");

    return true;
}

bool CFastCgiServer::Run(bool bDaemonize)
{
    for(int i=0; i<m_usProcessCount; ++i)
    {
        pid_t pid = fork();
        if(pid > 0)
        {
            FAST_DEBUG("fork child process Pid:" << pid);
            continue;
        }
        else if(pid < 0)
        {
            FAST_FATAL("Fork child failure");
            continue;
        }
        else
        {
            close(FCGI_LISTENSOCK_FILENO);
            dup2(m_iListenFd, FCGI_LISTENSOCK_FILENO);   
            close(m_iListenFd);
            int iMaxFd = open("/dev/null", O_RDWR);
            FAST_INFO("MaxFd:" << iMaxFd);
            if (-1 != iMaxFd)
            {
                if (iMaxFd != STDOUT_FILENO) dup2(iMaxFd, STDOUT_FILENO);
                if (iMaxFd != STDERR_FILENO) dup2(iMaxFd, STDERR_FILENO);
                if (iMaxFd != STDOUT_FILENO && iMaxFd != STDERR_FILENO) close(iMaxFd);
            }
            else
            {
                FAST_INFO("couldn't open and redirect stdout/stderr to '/dev/null': " << strerror(errno));
            } 
            while(FCGI_Accept() >= 0)   //阻塞等待新的连接
            {
                clear();
                if(NULL != m_pCgiCallBack)
                {
                    ParseParam();       //参数解析
                    m_pCgiCallBack(m_umParam, m_pCgiParameter, m_strOutPacket); //调用业务逻辑处理
                    FCGI_printf("%s", m_strOutPacket.c_str());  //返回请求报文
                }
                clear();
            }
        }
    }
    if(!bDaemonize)
    {
        FAST_INFO("Non-daemon process waiting");
        wait(NULL);
    }
    return true;
}

void CFastCgiServer::ParseParam()
{
    ParseParamItem(fastcgi::STR_CONTENT_LENGTH     );
    ParseParamItem(fastcgi::STR_CONTENT_TYPE       );
    ParseParamItem(fastcgi::STR_DOCUMENT_ROOT      );
    ParseParamItem(fastcgi::STR_DOCUMENT_URI       );
    ParseParamItem(fastcgi::STR_GATEWAY_INTERFACE  );
    ParseParamItem(fastcgi::STR_HTTP_ACCEPT        );
    ParseParamItem(fastcgi::STR_HTTP_CONTENT_LENGTH);
    ParseParamItem(fastcgi::STR_HTTP_CONTENT_TYPE  );
    ParseParamItem(fastcgi::STR_HTTP_COOKIE        );
    ParseParamItem(fastcgi::STR_HTTP_HOST          );
    ParseParamItem(fastcgi::STR_HTTP_USER_AGENT    );
    ParseParamItem(fastcgi::STR_QUERY_STRING       );
    ParseParamItem(fastcgi::STR_REDIRECT_STATUS    );
    ParseParamItem(fastcgi::STR_REMOTE_ADDR        );
    ParseParamItem(fastcgi::STR_REMOTE_PORT        );
    ParseParamItem(fastcgi::STR_REQUEST_METHOD     );
    ParseParamItem(fastcgi::STR_REQUEST_SCHEME     );
    ParseParamItem(fastcgi::STR_REQUEST_URI        );
    ParseParamItem(fastcgi::STR_SCRIPT_NAME        );
    ParseParamItem(fastcgi::STR_SERVER_ADDR        );
    ParseParamItem(fastcgi::STR_SERVER_NAME        );
    ParseParamItem(fastcgi::STR_SERVER_PORT        );
    ParseParamItem(fastcgi::STR_SERVER_PROTOCOL    );
    ParseParamItem(fastcgi::STR_SERVER_SOFTWARE    );
}                      
