#include <utility>
#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <regex>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <regex>
#include <sys/types.h>
#include <sstream>

#define MAXTESTCASE 10;
using namespace std;
using namespace boost::asio;
using namespace boost::system;
using namespace ip;

io_service global_io_service;

class ShellSession :public enable_shared_from_this<ShellSession>{
private:
    enum { max_length = 1024 };
    tcp::socket _socket;
    tcp::resolver _resolver;
    tcp::resolver::query _query;
    tcp::socket _html;
    string html;
    string cmd;

    string _hostname;
    string _port;
    string _filename;
    array<char, max_length> _data;
    ifstream _in;
    int _session;
public:
    ShellSession(string hostname, string port, string filename, int session, tcp::socket socket):
            _socket(global_io_service),
            _resolver(global_io_service),
            _query(tcp::v4(), hostname, port),
            _hostname(hostname),
            _port(port),
            _filename(filename),
            _in("test_case/" + _filename),
            _session(session),
            _html(socket){}
    void start(){
        do_resolve();
    }
private:
    void do_resolve(){
        auto self(shared_from_this());
        _resolver.async_resolve(_query,
                                [this, self](boost::system::error_code ec,
                                             tcp::resolver::iterator endpoint_iterator){
                                    if (!ec){
                                        // Attempt a connection to the first endpoint in the list. Each endpoint
                                        // will be tried until we successfully establish a connection.
                                        do_connect(endpoint_iterator);
                                    } else{
                                        output_shell(0, "resolve error");
                                        _socket.close();
                                    }
                                });
    }
    void do_connect(tcp::resolver::iterator endpoint_iterator){
        auto self(shared_from_this());
        async_connect(_socket,
                      endpoint_iterator,
                      [this, self](boost::system::error_code ec, tcp::resolver::iterator){
                          if (!ec){
                              do_read();
                          } else{
                              output_shell(0, "connection error");
                              _socket.close();
                          }
                      });
    }
    void do_read() {
        auto self(shared_from_this());
        _socket.async_read_some(
                buffer(_data, max_length),
                [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec){
                        string cmd;
                        for (int i = 0; i < length; ++i) {
                            cmd += _data[i];
                        }
                        output_shell(_session, cmd);
                        if (cmd.find("%")!=string::npos)
                            do_send_cmd();
                        do_read();
                    } else{
                        _socket.close();
                    }
                });
    }
    void do_send_cmd(){
        auto self(shared_from_this());
        string line;
        getline(_in, line);
        line += '\n';
        output_command(_session, line);
        _socket.async_send(
                buffer(line),
                [this, self](boost::system::error_code ec, size_t){
                    if (ec){
                        _socket.close();
                    }
                });
    }
    void do_send_html(){
        auto self(shared_from_this());
        _html.async_send(
                buffer(html),
                [this, self](boost::system::error_code ec, size_t){
                    if (ec){
                        _socket.close();
                    }
                });
    }
    void output_shell(int ID, string content){
        boost::replace_all(content, "\r\n", "&NewLine;");
        boost::replace_all(content, "\n", "&NewLine;");
        boost::replace_all(content, "\\", "\\\\");
        boost::replace_all(content, "\'", "\\\'");
        boost::replace_all(content, "<", "&lt;");
        boost::replace_all(content, ">", "&gt;");
        string session = "s" + to_string(ID);
        html = "<script>document.getElementById('" << session << "').innerHTML += '" << content << "';</script>" << endl;
        do_send_html();
    }

    void output_command(int ID, string content){
        boost::replace_all(content,"\r\n","&NewLine;");
        boost::replace_all(content,"\n","&NewLine;");
        boost::replace_all(content, "\\", "\\\\");
        boost::replace_all(content,"\'","\\\'");
        boost::replace_all(content,"<","&lt;");
        boost::replace_all(content,">","&gt;");
        string session = "s" + to_string(ID);
        html = "<script>document.getElementById('" << session << "').innerHTML += '<b>" << content << "</b>';</script>" << endl;
        do_send_html();
    }
};
class Client{
private:
    string hostname;
    string port;
    string filename;
    int session;
    tcp::socket socket;
public:
    Client(string hostname_, string port_, string filename_, int session_, tcp::socket socket_){
        hostname = hostname_;
        port = port_;
        filename = filename_;
        session = session_;
        socket = socket_;
    }
    void start(){
        make_shared<ShellSession>(hostname, port, filename, session, socket)->start();
    }
    string output_server(){
        string CSS = R"(            <th scope="col">)";
        CSS += hostname;
        CSS += R"(:)";
        CSS += port;
        CSS += R"(</th>)";
        return CSS;
    }
};

using namespace std;
using namespace boost::asio;

io_service global_io_service;

class HTTPSession : public enable_shared_from_this<HTTPSession> {
private:
    enum { max_length = 1024 };
    ip::tcp::socket _socket;
    array<char, max_length> _data;
    string CSS;

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

    void do_parse(string url){
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

            cout << "HTTP/1.1 200 OK" << endl;
            if (service == "console.cgi"){


                regex reg("((|&)\\w+=)([^&]+)");
                smatch m;
                int session = 0;
                while (regex_search(parse_parameter, m, reg)){
                    string hostname = m[3].str();

                    parse_parameter = m.suffix().str();
                    regex_search(parse_parameter, m, reg);
                    string port = m[3].str();

                    parse_parameter = m.suffix().str();
                    regex_search(parse_parameter, m, reg);
                    string filename = m[3].str();
                    parse_parameter = m.suffix().str();

                    make_shared<Client>(hostname, port, filename, session, _socket)->start();
                    ++session;
                }

                CSS = "HTTP/1.1 200 OK\n";
                CSS += "Content-type:text/html\n\n";

                CSS += R"(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>NP Project 3 Console</title>
    <link
      rel="stylesheet"
      href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css"
      integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
      rel="stylesheet"
    />
    <link
      rel="icon"
      type="image/png"
      href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
    />
    <style>
      * {
        font-family: 'Source Code Pro', monospace;
        font-size: 1rem !important;
      }
      body {
        background-color: #212529;
      }
      pre {
        color: #cccccc;
      }
      b {
        color: #ffffff;
      }
    </style>
  </head>
  <body>
    <table class="table table-dark table-bordered">
      <thead>
        <tr>)";
                for (int i = 0; i < session; ++i) {
                    CSS += clients[i].output_server();
                }
                CSS+=R"(        </tr>
      </thead>
      <tbody>
        <tr>)";
                for (int i = 0; i < session; ++i) {
                    CSS+=R"(            <td><pre id="s)";
                    CSS+= to_string(i);
                    CSS+=R"(" class="mb-0"></pre></td>)";
                }
                CSS+=R"(        </tr>
      </tbody>
    </table>
  </body>
</html>)";
                do_send();

                //---------------------------------------------
                //---------------------------------------------
                //---------------------------------------------
            } else if (service == "panel.cgi"){
                string host_menu;
                string test_case_menu;
                for (int i = 0; i < MAXTESTCASE; ++i) {
                    test_case_menu += R"(<option value=")" + "t" + to_string(i) + ".txt" + R"(>)" + "t" + to_string(i) + R"(</option>)";
                }
                CSS = "Content-type: text/html\r\n\r\n";
                CSS += R"(
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>NP Project 3 Panel</title>
    <link
      rel="stylesheet"
      href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css"
      integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
      rel="stylesheet"
    />
    <link
      rel="icon"
      type="image/png"
      href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
    />
    <style>
      * {
        font-family: 'Source Code Pro', monospace;
      }
    </style>
  </head>
  <body class="bg-secondary pt-5">
    <form action="console.cgi" method="GET">
      <table class="table mx-auto bg-light" style="width: inherit">
        <thead class="thead-dark">
          <tr>
            <th scope="col">#</th>
            <th scope="col">Host</th>
            <th scope="col">Port</th>
            <th scope="col">Input File</th>
          </tr>
        </thead>
        <tbody>)";
                for (int i = 0; i < 5; ++i) {
                    CSS += R"(
          <tr>
            <th scope="row" class="align-middle">Session )";
                    CSS += to_string(i+1) + "</th>";
                    CSS += R"(
            <td>
              <div class="input-group">
                <select name="h)";
                    CSS += to_string(i);
                    CSS += R"(" class="custom-select">
                  <option></option>)";
                    CSS += host_menu;
                    CSS += R"(
                </select>
                <div class="input-group-append">
                  <span class="input-group-text">.cs.nctu.edu.tw</span>
                </div>
              </div>
            </td>
            <td>
              <input name="p)";
                    CSS += to_string(i);
                    CSS += R"( type="text" class="form-control" size="5" />
            </td>
            <td>
              <select name="f)";
                    CSS += to_string(i);
                    CSS += R"(" class="custom-select">
                <option></option>
                )";
                    CSS += test_case_menu;
                    CSS += R"(
              </select>
            </td>
          </tr>)";
                }
                CSS += R"(
          <tr>
            <td colspan="3"></td>
            <td>
              <button type="submit" class="btn btn-info btn-block">Run</button>
            </td>
          </tr>
        </tbody>
      </table>
    </form>
  </body>
</html>)";
                do_send();
            } else{
                cout << "What a hell ?" << endl;
            }
        }
    }

    void do_send(){
        auto self(shared_from_this());
        _socket.async_send(
                buffer(CSS),
                [this, self](boost::system::error_code ec, size_t){
                    if (ec){
                        _socket.close();
                    }
                });
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
