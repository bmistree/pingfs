#include <pingfs/util/log.hpp>

#include <fstream>

namespace pingfs {

std::shared_ptr<Log>& Log::get_instance() {
    static std::shared_ptr<Log> instance;
    return instance;
}

void Log::init_cout(LogLevel level) {
    get_instance().reset(new Log(&std::cout, level));
}

void Log::init_file(const std::string& logging_filename, LogLevel level) {
    get_instance().reset(
        // Note that nothing ever frees this ofstream.
        // That's not great. But it's lifetime should be
        // the lifetime of the entire program, so we
        // can live with it.
        new Log(new std::ofstream(logging_filename), level));
}

Log::Log(std::ostream* ostream, LogLevel level) :
  write_mutex_(),
  ostream_(ostream),
  level_(level) {
}

void Log::log(LogLevel level, const std::string& msg) {
    get_instance()->log_internal(level, msg);
}

Log::~Log() {
}

void Log::log_internal(LogLevel level, const std::string& msg) {
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
    (*ostream_) << msg << "\n";
}

}  // namespace pingfs
