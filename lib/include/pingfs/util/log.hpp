#ifndef _LOG_
#define _LOG_

#include <boost/thread/recursive_mutex.hpp>

#include <cstdarg>
#include <fstream>
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
    static void init_cout(LogLevel level) {
        get_instance().reset(new Log(&std::cout, level));
    }
    static void init_file(const std::string& logging_filename,
        LogLevel level) {
        get_instance().reset(
            // Note that nothing ever frees this ofstream.
            // That's not great. But it's lifetime should be
            // the lifetime of the entire program, so we
            // can live with it.
            new Log(new std::ofstream(logging_filename), level));
    }

    template<class ...Args>
    static void log_with_level(LogLevel level, Args... args) {
        get_instance()->log_with_level_internal(level, args...);
    }

    ~Log() {
    }

 private:
    static std::shared_ptr<Log>& get_instance() {
        static std::shared_ptr<Log> instance;
        // static std::once_flag flag;
        // std::call_once(flag, [] { init_cout(LogLevel::DEBUG); });
        return instance;
    }

    Log(std::ostream* ostream, LogLevel level)
     : write_mutex_(),
       ostream_(ostream),
       level_(level) {
    }

    template<class T>
    void log_internal(T first) {
        (*ostream_) << first;
    }

    template<class T, class ...Args>
    void log_internal(T first, Args... args) {
        (*ostream_) << first;
        log_internal(args...);
    }

    template<class ...Args>
    void log_with_level_internal(LogLevel level, Args... args) {
        boost::recursive_mutex::scoped_lock locker(write_mutex_);

        if (level < level_) {
            // message to log is below the configured level;
            // ignore it.
            return;
        }

        switch (level) {
          case LogLevel::DEBUG:
            (*ostream_) << "DEBUG: ";
            break;
          case LogLevel::INFO:
            (*ostream_) << "INFO: ";
            break;
          case LogLevel::ERROR:
            (*ostream_) << "ERROR: ";
            break;
          default:
            throw "Unexpected level";
        }

        log_internal(args...);
        (*ostream_) << "\n";
        ostream_->flush();
    }

 private:
    boost::recursive_mutex write_mutex_;
    std::ostream* ostream_;
    LogLevel level_;
};


#define LOGGING_ON

#ifdef LOGGING_ON
#define LOG(log_level, ...)  \
    Log::log_with_level(log_level, __VA_ARGS__);
#else
#define LOG(log_level, ...)
#endif


}  // end namespace pingfs

#endif
