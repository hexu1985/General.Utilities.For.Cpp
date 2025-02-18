#include "SocketException.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <sstream>

namespace {

std::string Strerror(std::string_view msg, int error_code) {
    std::ostringstream os;
    os << msg;
    if (error_code) {
        os << ": " << strerror(error_code);
    }
    return os.str();
}

std::string Hstrerror(std::string_view msg, int error_code) {
    std::ostringstream os;
    os << msg;
    if (error_code) {
        os << ": " << hstrerror(error_code);
    }
    return os.str();
}

std::string Gai_strerror(std::string_view msg, int error_code) {
    std::ostringstream os;
    os << msg;
    if (error_code) {
        os << ": " << gai_strerror(error_code);
    }
    return os.str();
}

}   // namespace

OSError::OSError(int error_code, std::string_view msg): 
    std::runtime_error(Strerror(msg, error_code)), 
    error_code_(error_code) {
}

OSError::OSError(std::string_view msg): OSError(0, msg) {}

HError::HError(int error_code, std::string_view msg): 
    std::runtime_error(Hstrerror(msg, error_code)),
    error_code_(error_code) {
}

HError::HError(std::string_view msg): HError(0, msg) {}

GAIError::GAIError(int error_code, std::string_view msg): 
    std::runtime_error(Gai_strerror(msg, error_code)),
    error_code_(error_code) {
}

GAIError::GAIError(std::string_view msg): GAIError(0, msg) {}

