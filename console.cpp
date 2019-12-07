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

using namespace std;
using namespace boost::asio;
using namespace boost::system;

class ShellSession: enable_share_from_this<ShellSession>{
private:
    enum { max_length = 1024 };
    ip::tcp::socket _socket;
    ip::tcp::resolver _resolver;
    tcp::resolver::iterator endpoint_iterator;

    string hostname;
    unsigned short port;
    string filename;

public:
    ShellSession(
            string hostname,
            unsigned short port,
            string filename
            ): _socket(global)
    void start(){
        do_resolve();
    }
private:
    void do_resolve(){
        auto self(shared_from_this());
        _resolver.async_resolve(query,
                [this, self](boost::system::error_code ec,
                        tcp::resolver::iterator endpoint_iterator){
                if (!err)
                {
                    // Attempt a connection to the first endpoint in the list. Each endpoint
                    // will be tried until we successfully establish a connection.
                    do_connect(endpoint_iterator);
                } else{
                    _socket.close();
                }
        });
    }
    void do_connect(tcp::resolver::iterator endpoint_iterator){
        auto self(shared_from_this());
        _socket.async_connect(endpoint,
                      [this, self](boost::system::error_code ec,
                                   tcp::resolver::iterator endpoint_iterator){
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
                    if (buffer.find("%")!=string::npos)
                        do_send_cmd();
                    do_send_cmd();
                } else{
                    _socket.close();
                }
        });
    }
    void do_send_cmd(string id){

    }
};
class clinet{
private:
    string hostname;
    unsigned short port;
    string filename;
public:
    void run(){
        make_shared<ShellSession>();
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
    string parse_paremeter = getenv("QUERY_STRING");
    regex reg("((|&)\\w+=)([^&]+)");
    smatch m;
    while (regex_search(parse_paremeter, m, reg)){
        
    }
}