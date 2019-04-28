#include "log.h"

void CLog::SetFilter(severity_level filterLevel)
{
    logging::core::get()->set_filter(expr::attr<severity_level>("Severity")>=filterLevel);
}

bool CLog::InitLog(const string &strLogPath, const string &strLogName)
{
    SetFilter(debug);

    string strHostNameIp = tools::GetHostNameIp();
    typedef sinks::synchronous_sink<sinks::text_file_backend> TextSink;
    boost::shared_ptr<sinks::text_file_backend> backend_file = boost::make_shared<sinks::text_file_backend>(  
        keywords::file_name = strLogPath + "/" + strLogName + "-%Y%m%d.%N.log",  
        keywords::rotation_size = 10 * 1024 * 1024 * 100,  
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),  //每天00:00:00创建新日志文件
        keywords::min_free_space = 30 * 1024 * 1024,
        keywords::open_mode = std::ios_base::app            //追加写入
        );

    backend_file->auto_flush(true);  
    boost::shared_ptr<TextSink> sink_file(new TextSink(backend_file));  
    //时间|日志等级|进程id|线程id|本地ip|服务名|文件名|函数名|所在行|日志正文
    string strFormat = "%1%|%2%|%3%|%4%|"+strHostNameIp+"|%5%|%6%";
    sink_file->set_formatter (  
        expr::format("%1%|%2%|%3%|%4%|%5%|"+strHostNameIp+"|%6%")  
        % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y%m%d-%H%M%S.%f")  
        % expr::attr<severity_level>("Severity")  
        % expr::attr<attrs::current_process_id::value_type>("ProcessID")  
        % expr::attr<attrs::current_thread_id::value_type >("ThreadID")  
        % expr::attr<attrs::current_process_name::value_type >("ProcessName")  
        % expr::smessage  
        );  

    logging::core::get()->add_sink(sink_file);
    sink_file->set_filter(expr::attr<severity_level>("Severity") >= debug);

    boost::shared_ptr<sinks::text_file_backend> backend_report = boost::make_shared<sinks::text_file_backend>(  
        keywords::file_name = strLogPath + "/" + strLogName + "-%Y%m%d.report.log",  
        keywords::rotation_size = 100 * 1024 * 1024,  
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(boost::gregorian::greg_day(1), 0, 0, 0),  
        keywords::min_free_space = 30 * 1024 * 1024,
        keywords::open_mode = std::ios_base::app
        );

    backend_report->auto_flush(true);  
    boost::shared_ptr<TextSink> sink_report(new TextSink(backend_report));  
    sink_report->set_formatter (  
        expr::format("%1%,%2%")  
        % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")  
        % expr::smessage 
        );  

    sink_report->set_filter(expr::attr<severity_level>("Severity") == report);  
    logging::core::get()->add_sink(sink_report);  

    logging::add_common_attributes();  
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
    logging::core::get()->add_global_attribute("ProcessID", attrs::current_process_id());
    logging::core::get()->add_global_attribute("ProcessName", attrs::current_process_name());

    return true;
}
