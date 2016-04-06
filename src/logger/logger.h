#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <sstream>

#include "define.h"
#include "logger/zmqext.h"
#include "rfmdriver.h"

#define _ME_ __PRETTY_FUNCTION__

namespace zmq { class socket_t; }
class RFMDriver;
class DMA;

enum class LogType {
    None = 0,
    Log = 1,
    Error = 2,
};
enum class LogValue {
    None = 0,
    BPM,
    CM
};
/**
 * \addtogroup
 */
namespace Logger {

struct log_stream_t {
    LogType header;
    std::ostringstream message;
    std:: string other;
};

class Logger
{
public:
    /**
     * @brief Constructor
     *
     * The ZMQ publisher socket is set here with the default port.
     */
    explicit Logger(zmq::context_t& context);

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Set the RFM Helper.
     */
    void setRFM(RFMDriver* driver) { m_driver = driver; }

    void sendMessage(std::string message, std::string error="");
    void sendZmq(const std::string& header, const std::string& message, const std::string& other);
    void sendZmqValue(const std::string& header, const int loopPos, const arma::vec& valueX, const arma::vec& valueY);
    void sendRFM(std::string message, std::string error);

    /**
     * @brief Set/Unset the debug mode
     */
    void setDebug(const bool debug) { m_debug = debug; }

    /**
     * @brief Check if the mbox is in debug mode.
     *
     * @return True if debug mode, else false.
     */
    bool hasDebug() const { return m_debug; }

    /**
     * @brief Acess to m_logStream
     */
    log_stream_t& logStream() { return m_logStream; };

private:
    RFMDriver* m_driver;

    /**
     * @brief ZMQ Socket used to publish logs, values, errors.
     */
    zmq_ext::socket_t* m_zmqSocket;

    /**
     * @brief Is the mBox in debug mode?
     */
    bool m_debug;
    log_stream_t m_logStream;
};

extern Logger logger;

/**
 * @brief Global wrapper to access the setDebug(bool) method of the class Logger.
 */
void setDebug(bool debug);
/**
 * @brief
 */
std::ostream& flush(std::ostream& output);

/**
 * @brief
 */
std::ostringstream& log(LogType type);

/**
 * @brief
 */
std::ostringstream& log();

/**
 * @brief
 */
void values(LogValue name, const int loopPos, const arma::vec& valueX, const arma::vec& valueY);

/**
 * @brief Global wrapper to log errors.
 *
 * @param fctname Name of the function in which is the error.
 * @return Stream Logger::m_logStream.message;
 *
 * @note Use it as:
 * \code{.cpp}
 *      Logger::error(_ME_) << "This is an error: << Logger::flush;
 * \endcode
 * A prepocessor macro `_ME_` is used for `__PRETTY_FUNCTION__`.
 */
std::ostringstream& error(std::string fctname);

void postError(unsigned int errornr);
std::string errorMessage(unsigned int errornr);

}

#endif // LOGGER_H
