/* 
 * fastcgi.h --
 *
 *	Defines for the FastCGI protocol.
 *
 *
 * Copyright (c) 1995-1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: fastcgi.h,v 1.1.1.1 1997/09/16 15:36:32 stanleyg Exp $
 */

#ifndef _FASTCGI_H
#define _FASTCGI_H

/*
 * Listening socket file number
 */
#define FCGI_LISTENSOCK_FILENO 0

//FastCGI的header 消息头
typedef struct {
    unsigned char version;         // 版本  表示FCGI的版本信息
    unsigned char type;            // 操作类型
    unsigned char requestIdB1;     // 请求Id requestIdB1和requestIdB0两个字节组合来表示requestId(每个请求Web服务器会分配一个requestId), requestIdB1是requestId的高8位,requestIdB0是低8位,所以requestId=(requestIdB1<<8)+requestIdB0
    unsigned char requestIdB0;     //        requestId用两个字节表示其取值最大为65535
    unsigned char contentLengthB1; // 内容长度 contentLengthB1和contentLengthB0两个字节组合表示消息头的长度,contentLength=(contentLengthB1<<8)+contentLengthB0
    unsigned char contentLengthB0; //          contentLength用两个字节表示,最大长度为65535,对于超过65535的消息体,可以切割成多个消息体来传输
    unsigned char paddingLength;   // 填充字节长度  为使消息8字节对齐,可以在消息体中额外添加一些字节数达到消息对齐的目的,paddingLength为添加的字节数,额外添加字节是无用字节,读出来可直接丢弃
    unsigned char reserved;        // 保留字节,暂时无用
} FCGI_Header;

#define FCGI_MAX_LENGTH 0xffff

/*
 * Number of bytes in a FCGI_Header.  Future versions of the protocol
 * will not reduce this number.
 */
#define FCGI_HEADER_LEN  8          //FCGI的长度8个字节

/*
 * Value for version component of FCGI_Header
 */
#define FCGI_VERSION_1           1  //FastCGI消息头版本

/*
 * Values for type component of FCGI_Header FCGI操作类型
 */
#define FCGI_BEGIN_REQUEST       1      //Web->FastCGI 表示一个请求的开始
#define FCGI_ABORT_REQUEST       2      //Web->FastCGI 表示服务器希望终止一个请求
#define FCGI_END_REQUEST         3      //FastCGI->Web 表示该请求处理完毕
#define FCGI_PARAMS              4      //Web->FastCGI 对应于CGI程序的环境变量
#define FCGI_STDIN               5      //Web->FastCGI 对应CGI程序的标准输入 FastCGI程序从此消息获取HTTP请求的POST数据
#define FCGI_STDOUT              6      //FastCGI->Web 对应CGI程序的标准输出
#define FCGI_STDERR              7      //FastCGI->Web 对应CGI程序的标准错误输出
#define FCGI_DATA                8      //Web->FastCGI
#define FCGI_GET_VALUES          9      //Web->FastCGI
#define FCGI_GET_VALUES_RESULT  10      //FastCGI->Web
#define FCGI_UNKNOWN_TYPE       11      //FastCGI->Web FastCGI程序无法解析该消息类型
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

/*
 * Value for requestId component of FCGI_Header
 */
#define FCGI_NULL_REQUEST_ID     0


//请求开始请求的消息体
typedef struct {
    unsigned char roleB1;       //roleB1和roleB0两个字节组合代指Web服务器希望FastCGI程序充当的角色  常见FastCGI程序基本为FCGI_RESPONDER(响应器角色)
    unsigned char roleB0;
    unsigned char flags;        //8位掩码(目前只有一个FCGI_KEEP_CONN的标志,表示FastCGI请求结束连接是否关闭)
    unsigned char reserved[5];  //保留字节 8位对齐 暂时无用
} FCGI_BeginRequestBody;        //对应FCGI_BEGIN_REQUEST类型请求

typedef struct {
    FCGI_Header header;
    FCGI_BeginRequestBody body;
} FCGI_BeginRequestRecord;

/*
 * Mask for flags component of FCGI_BeginRequestBody
 */
#define FCGI_KEEP_CONN  1   //FastCGI程序请求结束关闭连接标志的掩码

/*
 * Values for role component of FCGI_BeginRequestBody
 */
#define FCGI_RESPONDER  1   //响应器角色
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3


//请求结束发送的消息体
typedef struct {
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;   //状态码
    unsigned char reserved[3];      //保留字节 8位对齐 暂时无用
} FCGI_EndRequestBody;              //对应FCGI_END_REQUEST类型请求

typedef struct {
    FCGI_Header header;
    FCGI_EndRequestBody body;
} FCGI_EndRequestRecord;

/*
 * Values for protocolStatus component of FCGI_EndRequestBody
 */
#define FCGI_REQUEST_COMPLETE 0     //请求正常结束
#define FCGI_CANT_MPX_CONN    1     //拒绝新的请求.
#define FCGI_OVERLOADED       2     //拒绝新的请求.应用程序耗尽了某些资源
#define FCGI_UNKNOWN_ROLE     3     //拒绝新的请求.Web服务器指定应用程序未知角色


/*
 * Variable names for FCGI_GET_VALUES / FCGI_GET_VALUES_RESULT records
 */
#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"


typedef struct {
    unsigned char type;    
    unsigned char reserved[7];
} FCGI_UnknownTypeBody;

typedef struct {
    FCGI_Header header;
    FCGI_UnknownTypeBody body;
} FCGI_UnknownTypeRecord;

#endif	/* _FASTCGI_H */

