#ifndef _LOG_H_
#define _LOG_H_

#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/exceptions.hpp>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <boost/shared_ptr.hpp>  
#include <boost/make_shared.hpp>  
#include <boost/filesystem.hpp>  

#include "tools.h"

class tools;

namespace logging  = boost::log;
namespace sinks    = boost::log::sinks;
namespace attrs    = boost::log::attributes;
namespace src      = boost::log::sources;
namespace expr     = boost::log::expressions;
namespace keywords = boost::log::keywords;

using namespace std;
using boost::shared_ptr;

enum severity_level  
{  
    debug,
    info,
    warning,  
    error,  
    fatal, 
    report
};  

template< typename CharT, typename TraitsT >  
inline std::basic_ostream< CharT, TraitsT >& operator<< (  
            std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)  
{  
        static const char* const str[] =  
        {  
            "DEBUG",  
            "INFO",  
            "WARNING",  
            "ERROR",  
            "FATAL",  
            "REPORT"
        };  
        if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))  
            strm << str[lvl];
        else  
            strm << static_cast< int >(lvl);  
        return strm;  
}  

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt<severity_level>);

class CLog
{
public:
    static void SetFilter(severity_level filterLevel);
    static bool InitLog(const string &strLogPath, const string &strLogName);
};

#define LOG_CO_PRINT(level, file, line, msg) \
    do { \
        std::stringstream __ss_msg; \
        __ss_msg << msg; \
        BOOST_LOG_SEV(my_logger::get(), level) << tools::GetFileNameFromFilePath(file) << "|" << __FUNCTION__ << "|" << to_string(line) << "|" << __ss_msg.str(); \
    } while (0)

#define LOG_PRINT(level, msg) \
    do { \
        std::stringstream __ss_msg; \
        __ss_msg << msg; \
        BOOST_LOG_SEV(my_logger::get(), level) << tools::GetFileNameFromFilePath(__FILE__) << "|" << __FUNCTION__ << "|" << to_string(__LINE__) << "|" << __ss_msg.str(); \
    } while (0)

#define INFO(msg)       LOG_PRINT(info, msg)
#define DEBUG(msg)      LOG_PRINT(debug, msg)
#define WARNING(msg)    LOG_PRINT(warning, msg)
#define ERROR(msg)      LOG_PRINT(error, msg)
#define FATAL(msg)      LOG_PRINT(fatal, msg)
#define REPORT(msg) \
    do { \
        std::stringstream __ss_msg; \
        __ss_msg << msg; \
        BOOST_LOG_SEV(my_logger::get(), level) << __ss_msg.str(); \
    } while (0)


#endif
