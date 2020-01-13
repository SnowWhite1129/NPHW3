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
#include <err.h>


using namespace std;
using namespace boost::asio;
using namespace boost::system;
using namespace ip;

io_service global_io_service;

void output_shell(int ID, string content){
    boost::replace_all(content, "\r\n", "&NewLine;");
    boost::replace_all(content, "\n", "&NewLine;");
    boost::replace_all(content, "\\", "\\\\");
    boost::replace_all(content, "\'", "\\\'");
    boost::replace_all(content, "<", "&lt;");
    boost::replace_all(content, ">", "&gt;");
    string session = "s" + to_string(ID);
<<<<<<< HEAD
    cout << "<script>document.getElementById('" << session << "').innerHTML += '" << content << "';</script>" << endl; 
=======
    cout << "<script>document.getElementById('{ " << session << "}').innerHTML += '{" << content << "}';</script>";
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    fflush(stdout);
}

void output_command(int ID, string content){
    boost::replace_all(content,"\r\n","&NewLine;");
    boost::replace_all(content,"\n","&NewLine;");
    boost::replace_all(content, "\\", "\\\\");
    boost::replace_all(content,"\'","\\\'");
    boost::replace_all(content,"<","&lt;");
    boost::replace_all(content,">","&gt;");
    string session = "s" + to_string(ID);
<<<<<<< HEAD
    cout << "<script>document.getElementById('" << session << "').innerHTML += '<b>" << content << "</b>';</script>" << endl;
    fflush(stdout);
}


class ShellSession :public enable_shared_from_this<ShellSession>{
=======
    cout << "<script>document.getElementById('{" << session << "}').innerHTML += '<b>{" << content << "}</b>';</script>";
    fflush(stdout);
}

class ShellSession: enable_shared_from_this<ShellSession>{
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
private:
    enum { max_length = 1024 };
    tcp::socket _socket;
    tcp::resolver _resolver;
    tcp::resolver::query _query;

    string _hostname;
    string _port;
    string _filename;
    array<char, max_length> _data;
    ifstream _in;
    int _session;
public:
<<<<<<< HEAD
    ShellSession(string hostname, string port, string filename, int session):
=======
    ShellSession(const string &hostname, const string &port, const string &filename, int session):
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    _socket(global_io_service),
    _resolver(global_io_service),
    _query(tcp::v4(), hostname, port),
    _hostname(hostname),
    _port(port),
    _filename(filename),
<<<<<<< HEAD
    _in("test_case/" + _filename),
=======
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    _session(session){}
    void start(){
        _in.open("test_case/" + _filename);
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
<<<<<<< HEAD
                    output_shell(_session, cmd);
                    if (cmd.find("%")!=string::npos)
			do_send_cmd();
=======
                    output_command(_session, cmd);
                    for (int i = 0; i < length; ++i) {
                        if (_data[i] == '%'){
                            output_command(_session, "% ");
                            do_send_cmd();
                            break;
                        }
                    }
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
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
<<<<<<< HEAD
	line += '\n';
        output_command(_session, line);
        _socket.async_send(
                buffer(line),
                [this, self](boost::system::error_code ec, size_t){
		if (ec){ 
		    _socket.close();
		}
=======
        output_shell(_session, line);
        _socket.async_send(
                buffer(line),
                [this, self](boost::system::error_code ec, size_t length){
            if (!ec)
                do_read();
            else
                _socket.close();
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
        });
    }
};
class Client{
private:
    string hostname;
    string port;
    string filename;
    int session;
public:
<<<<<<< HEAD
    Client(string hostname_, string port_, string filename_, int session_){
=======
    Client(const string &hostname_, const string &port_, const string &filename_, int session_){
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
        hostname = hostname_;
        port = port_;
        filename = filename_;
        session = session_;
<<<<<<< HEAD
    }
    void start(){
        make_shared<ShellSession>(hostname, port, filename, session)->start();
    }
=======
    }
    void run(){
        make_shared<ShellSession>(hostname, port, filename, session)->start();
    }
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    string output_server(){
        string CSS = R"(            <th scope="col">)";
        CSS += hostname;
        CSS += R"(:)";
        CSS += port;
        CSS += R"(</th>)";
        return CSS;
    }
};

int main(){
    vector <Client> clients;
    string parse_parameter;
<<<<<<< HEAD
    char *tmp = getenv("QUERY_STRING");
    if (tmp != nullptr)
        parse_parameter = tmp;
    else
        parse_parameter = "";    
=======
    if (getenv("QUERY_STRING") != nullptr)
        parse_parameter = getenv("QUERY_STRING");
    else
        parse_parameter = "";
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b

    regex reg("((|&)\\w+=)([^&]+)");
    smatch m;
    int session = 0;
    while (regex_search(parse_parameter, m, reg)){
<<<<<<< HEAD
        string hostname = m[3].str();        

	parse_parameter = m.suffix().str();
        regex_search(parse_parameter, m, reg);
	string port = m[3].str();

        parse_parameter = m.suffix().str();
        regex_search(parse_parameter, m, reg);        
	string filename = m[3].str();
	parse_parameter = m.suffix().str();
         
	
	Client client(hostname, port, filename, session);
=======
        string hostname = m[3].str();
        parse_parameter = m.suffix().str();
        regex_search(parse_parameter, m, reg);
        string port = m[3].str();
        parse_parameter = m.suffix().str();
        regex_search(parse_parameter, m, reg);
        string filename = m[3].str();
        parse_parameter = m.suffix().str();
        regex_search(parse_parameter, m, reg);
        Client client(hostname, port, filename, session);
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
        clients.push_back(client);
        ++session;
    }

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
        <tr>)";
    for (int i = 0; i < session; ++i) {
        CSS += clients[i].output_server();
    }
    CSS+=R"(        </tr>
      </thead>
      <tbody>
        <tr>)";
    for (int i = 0; i < session; ++i) {
<<<<<<< HEAD
        CSS+=R"(            <td><pre id="s)";
	CSS+= to_string(i);
	CSS+=R"(" class="mb-0"></pre></td>)";
=======
        CSS+=R"(            <td><pre id="s0" class="mb-0"></pre></td>)";
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    }
    CSS+=R"(        </tr>
      </tbody>
    </table>
  </body>
</html>)";
    cout << "HTTP/1.1 200 OK" << endl;
    cout << "Content-type:text/html" << endl << endl;
    cout << CSS;
<<<<<<< HEAD
	
=======

>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    try {
	    
        for (int i=0; i<clients.size();++i) {
            clients[i].start();
        }
	
        global_io_service.run();
    } catch (exception& e){
<<<<<<< HEAD
        cout << "Error: " << e.what() << endl;
=======
        cout << "Error" << e.what() << endl;
>>>>>>> 9e425653c0befe8a265483f6d791be61c8301a8b
    }
}
