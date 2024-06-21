#include <stddef.h>
#include <iostream>
#include <functional>
#include <sstream>
#include <tuple>

#include <gflags/gflags.h>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include "Socket.hpp"

using fmt::format;

DEFINE_string(host, "127.0.0.1", "IP address the client sends to");
DEFINE_uint32(port, 1060, "TCP port number");
DEFINE_string(role, "", "which role to play");

static bool ValidateRole(const char* name, const std::string& value) {
    if (value == "client" || value == "server") {
        return true;
    }
    std::cout << "Invalid value for --" << name << ": " << value << "\n";
    std::cout << "  valid value: client or server\n";
    return false;
}

// 使用全局 static 变量来注册函数，static 变量会在 main 函数开始时就调用
static const bool role_dummy = gflags::RegisterFlagValidator(&FLAGS_role, &ValidateRole);

std::ostream& operator<< (std::ostream& out, const std::tuple<std::string, uint16_t>& address)
{
    out << "(" << std::get<0>(address) << ", " << std::get<1>(address) << ")";
    return out;
}

std::string usage(const char* prog) {
    std::ostringstream os;
    os << "\nusage: " << prog << " [--help] [--role client|server] [--host HOST] [--port PORT]\n\n"
        << "Send and receive over TCP\n";
    return os.str();
}

std::string recvall(Socket& sock, size_t length) {
    std::string data;
    while (data.length() < length) {
        auto more = sock.Recv(length - data.length());
        if (more.empty()) {
            throw std::runtime_error(
                    format("was expecting {} bytes but only received"
                       " {} bytes before the socket closed", length, data.length()));
        }
        data += more;
    }
    return data;
}

void server(const char* interface, uint16_t port) {
    Socket sock(AF_INET, SOCK_STREAM);
    sock.Setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
    sock.Bind(interface, port);
    sock.Listen(1);
    std::cout << "Listening at " << sock.Getsockname() << "\n";
    while (true) {
        std::cout << "Waiting to accept a new connection\n";
        std::tuple<std::string, uint16_t> sockname;
        Socket sc = sock.Accept(&sockname);
        std::cout << "We have accepted a connection from " << sockname << "\n";
        std::cout << "  Socket name: " << sc.Getsockname() << "\n";
        std::cout << "  Socket peer: " << sc.Getpeername() << "\n";
        auto message = recvall(sc, 16);
        std::cout << "  Incoming sixteen-octet message: " << message << "\n";
        sc.Sendall("Farewell, client");
        sc.Close();
        std::cout << "  Reply sent, socket closed\n";
    }
}

void client(const char* host, uint16_t port) {
    Socket sock(AF_INET, SOCK_STREAM);
    sock.Connect(host, port);
    std::cout << "Client has been assigned socket name " << sock.Getsockname() << "\n";
    sock.Sendall("Hi there, server");
    auto reply = recvall(sock, 16);
    std::cout << "The server said " << reply << "\n";
    sock.Close();
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage(usage(argv[0]));
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::function<void(const char*, uint16_t)> function;
    if (FLAGS_role == "client") {
        function = &client;
    } else {
        function = &server;
    }
    function(FLAGS_host.c_str(), FLAGS_port);

    return 0;
}
