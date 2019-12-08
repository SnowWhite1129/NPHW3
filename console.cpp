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
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <err.h>

using namespace std;
using namespace boost::asio;
using namespace boost::system;
using namespace ip;

io_service global_io_service;

class ShellSession: enable_shared_from_this<ShellSession>{
private:
    enum { max_length = 1024 };
    tcp::socket _socket;
    tcp::resolver _resolver;
    tcp::resolver::query _query;

    string _hostname;
    string _port;
    string _filename;
    array<char, max_length> _data;

public:
    ShellSession(const string &hostname, const string &port, const string &filename):
    _socket(global_io_service),
    _resolver(global_io_service),
    _query(hostname, port),
    _hostname(hostname),
    _port(port),
    _filename(filename){}
    void start(){
        do_resolve();
    }
private:
    void do_resolve(){
        auto self(shared_from_this());
        _resolver.async_resolve(_query,
                [this, self](boost::system::error_code ec,
                        tcp::resolver::iterator endpoint_iterator){
                if (!ec)
                {
                    // Attempt a connection to the first endpoint in the list. Each endpoint
                    // will be tried until we successfully establish a connection.
                    do_connect(endpoint_iterator);
                } else{
                    _socket.close();
                }
        });
    }
    void do_connect(const tcp::resolver::iterator &endpoint_iterator){
        auto self(shared_from_this());
        async_connect(_socket, endpoint_iterator, [this, self](const boost::system::error_code &ec, tcp::resolver::iterator endpoint_iterator){
            if (!ec){
                do_read();
            } else{
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
//                    if (buffer.find("%")!=string::npos)
//                        do_send_cmd();
//                    do_read();
                } else{
                    _socket.close();
                }
        });
    }
    void do_send_cmd(){

    }
};
class Client{
private:
    string hostname;
    string port;
    string filename;
public:
    Client(const string &hostname_, const string &port_, const string &filename_){
        hostname = hostname_;
        port = port_;
        filename = filename_;
    }
    void run(){
        make_shared<ShellSession>(hostname, port, filename)->start();
    }
};

int main(){
    string CSS = R"(
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
        <tr>
          <th scope="col">nplinux7.cs.nctu.edu.tw:3333</th>
          <th scope="col">nplinux8.cs.nctu.edu.tw:4444</th>
          <th scope="col">nplinux9.cs.nctu.edu.tw:5555</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><pre id="s0" class="mb-0"></pre></td>
          <td><pre id="s1" class="mb-0"></pre></td>
          <td><pre id="s2" class="mb-0"></pre></td>
        </tr>
      </tbody>
    </table>
  </body>
</html>
    )";
    vector <Client> clients;
    string parse_paremeter = getenv("QUERY_STRING");
    regex reg("((|&)\\w+=)([^&]+)");
    smatch m;
    while (regex_search(parse_paremeter, m, reg)){
        string hostname = m[3].str();
        regex_search(parse_paremeter, m, reg);
        string port = m[3].str();
        regex_search(parse_paremeter, m, reg);
        string filename = m[3].str();
        Client client(hostname, port, filename);
        clients.push_back(client);
    }
    try {
        if (!clients.empty()){
            for (auto & client : clients) {
                client.run();
            }
            global_io_service.run();
        }
    } catch (exception& e){
        cout << "Error" << endl;
    }
}