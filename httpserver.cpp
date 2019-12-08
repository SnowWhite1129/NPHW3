#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <regex>


using namespace std;
using namespace boost::asio;

io_service global_io_service;

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
                  do_parse();
              }
        });
    }

    void do_parse(const string &url){
        regex reg(R"((http[s]?:\/\/)?([^\/\s]+\/)*(.*\.cgi)(\?)(.*))");
        smatch m;
        string parse_parameter;
        regex_search(url, m, reg);
        parse_parameter = move(m[5].str());

        global_io_service.notify_fork(io_service::fork_prepare);
        pid_t pid = fork();
        if (pid == 0){
            global_io_service.notify_fork(boost::asio::io_service::fork_child);

            close(0);
            close(1);
            close(2);

            dup();
            execvp();
        } else{

        }
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

    try {
        unsigned short port = atoi(argv[1]);
        HTTPServer server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}