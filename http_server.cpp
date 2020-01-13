#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <regex>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>

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
        istringstream iss(url);
        string str;
        string parse_parameter, service, host, request;
        while (iss >> str){
            if (str.find(".cgi") != string::npos){
                parse_parameter = str;
            } else if (str.find("Host") != string::npos){
                iss >> host;
                break;
            }
        }
		

        regex reg(R"(([^\/\s]+\/)*(\/)(.*\.cgi)(\?)*(.*))");
        smatch m;

        if (regex_search(parse_parameter, m, reg)){
            parse_parameter = m[5].str();
	
	    service = m[3].str();		
	    request = m[3].str();
	    request += m[4].str();
	    request += m[5].str();	
            service = "/" + service;

            global_io_service.notify_fork(io_service::fork_prepare);
            pid_t pid = fork();
            if (pid == 0){
                global_io_service.notify_fork(boost::asio::io_service::fork_child);

                settingENV(parse_parameter, request, service, host);
		
		        service = "." + service;

                close(0);
                close(1);
                close(2);

                dup(_socket.native_handle());
                dup(_socket.native_handle());
                dup(_socket.native_handle());
                _socket.close();

                char *args[2];
                args[0] = strdup(service.c_str());
                args[1] = NULL;
                cout << "HTTP/1.1 200 OK" << endl;
                if (execvp(args[0], args) < 0){
                    cout << "Content-type: text/html" << endl << endl;
                    cout << "Unknown service" << endl;
                    cout << args[0] << endl;
                }

                free(args[0]);
                exit(0);
            } else{
                global_io_service.notify_fork(boost::asio::io_service::fork_parent);
                _socket.close();
            }
        }
    }

    void settingENV(string parse_parameter, string request, string service, string host){
        if (service =="console.cgi"){
            setenv("REQUEST_URI", request.c_str(), 1);
        } else{
            setenv("REQUEST_URI", service.c_str(), 1);
        }
	setenv("QUERY_STRING", parse_parameter.c_str(), 1);
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("HTTP_HOST", host.c_str(), 1);
        setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string().c_str(), 1);
        setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()).c_str(), 1);
        setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string().c_str(), 1);
        setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()).c_str(), 1);
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
    setenv("PATH", "/usr/bin:.", 1);

    try {
        unsigned short port = atoi(argv[1]);
        HTTPServer server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
