#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <regex>
#include <process.h>


using namespace std;
using namespace boost::asio;

io_service global_io_service;

void childHandler(int signo){
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
    // NON-BLOCKING WAIT
    // Return immediately if no child has exited.
}

class HTTPSession : public enable_shared_from_this<HTTPSession> {
private:
    enum { max_length = 1024 };
    ip::tcp::socket _socket;
    array<char, max_length> _data;

public:
    HTTPSession(ip::tcp::socket socket) : _socket(move(socket)) {}

    void start() { do_read(); }

private:
    void do_read() {
        auto self(shared_from_this());
        _socket.async_read_some(
            buffer(_data, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
              if (!ec){
                  string url;
                  for (int i = 0; i < length; ++i) {
                      url += _data[i];
                  }
                  do_parse(url);
              }
        });
    }

    void do_parse(const string &url){
        regex reg(R"((http[s]?:\/\/)?([^\/\s]+\/)*(.*\.cgi)(\?)(.*))");
        smatch m;
        string parse_parameter, service;
        regex_search(url, m, reg);
        parse_parameter = move(m[5].str());
        service = move(m[3].str());

        global_io_service.notify_fork(io_service::fork_prepare);
        pid_t pid = fork();
        if (pid == 0){
            global_io_service.notify_fork(boost::asio::io_service::fork_child);

            settingENV(m);

            close(0);
            close(1);
            close(2);

            dup(_socket.native_handle());
            dup(_socket.native_handle());
            dup(_socket.native_handle());

            char *args[2];
            args[0] = strdup(service.c_str());
            args[1] = nullptr;

            if (execvp(args[0], args)<0)

                cout << "Unknown service" << endl;

            free(args[0]);
            exit(0);
        } else{
            global_io_service.notify_fork(boost::asio::io_service::fork_parent);
            _socket.close();
        }
    }

    void settingENV(const smatch &m){
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("REQUEST_URI", (m[3].str()+m[4].str()+m[5].str()).c_str(), 1);
        setenv("QUERY_STRING", m[5].str().c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("HTTP_HOST", "localhost", 1);
        setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string(), 1);
        setenv("SERVER_PORT", _socket.local_endpoint().port().to_string(), 1);
        setenv("REMOTE_ADDR", _socket.remote_enpoint().address().to_string(), 1);
        setenv("REMOTE_PORT", _socket.remote_enpoint().port().to_string(), 1);
    }
};

class HTTPServer {
private:
    ip::tcp::acceptor _acceptor;
    ip::tcp::socket _socket;

public:
    HTTPServer(short port)
        : _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
        _socket(global_io_service) {
        do_accept();
    }

private:
    void do_accept() {
        _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
            if (!ec) make_shared<HTTPSession>(move(_socket))->start();
            do_accept();
        });
    }
};

int main(int argc, char* const argv[]) {
    if (argc != 2) {
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }

    signal(SIGCHLD, childHandler);
    clearenv();
    setenv("PATH", "bin:.", 1);

    try {
        unsigned short port = atoi(argv[1]);
        HTTPServer server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}