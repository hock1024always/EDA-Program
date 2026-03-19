#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace eda {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        min_level_ = level;
    }

    void debug(const std::string& msg) {
        log(LogLevel::DEBUG, msg);
    }

    void info(const std::string& msg) {
        log(LogLevel::INFO, msg);
    }

    void warning(const std::string& msg) {
        log(LogLevel::WARNING, msg);
    }

    void error(const std::string& msg) {
        log(LogLevel::ERROR, msg);
    }

private:
    LogLevel min_level_ = LogLevel::INFO;

    Logger() = default;

    void log(LogLevel level, const std::string& msg) {
        if (level < min_level_) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] ";

        switch (level) {
            case LogLevel::DEBUG:   ss << "[DEBUG] "; break;
            case LogLevel::INFO:    ss << "[INFO] "; break;
            case LogLevel::WARNING: ss << "[WARN] "; break;
            case LogLevel::ERROR:   ss << "[ERROR] "; break;
        }

        ss << msg;

        if (level == LogLevel::ERROR) {
            std::cerr << ss.str() << "\n";
        } else {
            std::cout << ss.str() << "\n";
        }
    }
};

// Convenience functions
inline void logDebug(const std::string& msg) {
    Logger::getInstance().debug(msg);
}

inline void logInfo(const std::string& msg) {
    Logger::getInstance().info(msg);
}

inline void logWarning(const std::string& msg) {
    Logger::getInstance().warning(msg);
}

inline void logError(const std::string& msg) {
    Logger::getInstance().error(msg);
}

} // namespace eda
