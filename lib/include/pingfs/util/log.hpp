#ifndef _LOG_
#define _LOG_

#include <boost/thread/recursive_mutex.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace pingfs {

enum class LogLevel {
    DEBUG,
    INFO,
    ERROR,
};


class Log {
 public:
    static void init_cout(LogLevel level);
    static void init_file(const std::string& logging_filename,
        LogLevel level);
    static void log(LogLevel level, const std::string& msg);

    ~Log();

 private:
    static std::shared_ptr<Log>& get_instance();
    Log(std::ostream* ostream, LogLevel level);
    void log_internal(LogLevel level, const std::string& msg);

 private:
    boost::recursive_mutex write_mutex_;
    std::ostream* ostream_;
    LogLevel level_;
};


#define LOGGING_ON

#ifdef LOGGING_ON
#define LOG(log_level, msg)         \
    Log::init_cout(LogLevel::DEBUG) \
    Log::log(level, msg);
#else
#define LOG(log_level, msg)
#endif


}  // end namespace pingfs

#endif
