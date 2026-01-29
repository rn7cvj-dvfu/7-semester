#include "http_server.hpp"
#include <iostream>
#include <sstream>
#include <vector>

namespace HttpServer
{

std::string HttpResponse::toString() const
{
    std::ostringstream oss;

    // Status line
    oss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";

    // Headers
    for (const auto &header : headers)
    {
        oss << header.first << ": " << header.second << "\r\n";
    }

    // Content-Length
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";

    // Body
    oss << body;

    return oss.str();
}

HttpServer::HttpServer(int port)
    : port_(port), server_socket_(INVALID_SOCKET), running_(false)
{
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
}

HttpServer::~HttpServer()
{
    stop();
    WSACleanup();
}

bool HttpServer::start()
{
    if (running_)
        return false;

    initializeSocket();

    if (server_socket_ == INVALID_SOCKET)
    {
        return false;
    }

    running_ = true;
    server_thread_ = std::thread(&HttpServer::serverLoop, this);

    std::cout << "HTTP Server started on port " << port_ << std::endl;
    return true;
}

void HttpServer::stop()
{
    if (!running_)
        return;

    running_ = false;

    cleanupSocket();

    if (server_thread_.joinable())
    {
        server_thread_.join();
    }

    std::cout << "HTTP Server stopped" << std::endl;
}

void HttpServer::registerHandler(const std::string &path, HttpHandler handler)
{
    handlers_[path] = handler;
}

void HttpServer::initializeSocket()
{
    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_socket_ == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return;
    }

    // Разрешить повторное использование адреса
    BOOL opt = TRUE;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (SOCKADDR *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
        return;
    }

    if (listen(server_socket_, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen on socket: " << WSAGetLastError() << std::endl;
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
        return;
    }
}

void HttpServer::cleanupSocket()
{
    if (server_socket_ != INVALID_SOCKET)
    {
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
    }
}

void HttpServer::serverLoop()
{
    while (running_)
    {
        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        socket_t client_socket = accept(server_socket_, (SOCKADDR *)&client_addr, &client_len);

        if (client_socket == INVALID_SOCKET)
        {
            if (running_)
            {
                std::cerr << "Failed to accept connection: " << WSAGetLastError() << std::endl;
            }
            continue;
        }

        // Обрабатываем клиента в отдельном потоке
        std::thread(&HttpServer::handleClient, this, client_socket).detach();
    }
}

void HttpServer::handleClient(socket_t client_socket)
{
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        std::string raw_request(buffer);

        HttpRequest request = parseRequest(raw_request);
        HttpResponse response;

        // Найти обработчик
        auto it = handlers_.find(request.path);
        if (it != handlers_.end())
        {
            it->second(request, response);
        }
        else
        {
            response.status_code = 404;
            response.status_message = "Not Found";
            response.setPlainText("404 Not Found");
        }

        // Отправить ответ
        std::string response_str = response.toString();
        send(client_socket, response_str.c_str(), static_cast<int>(response_str.size()), 0);
    }

    closesocket(client_socket);
}

HttpRequest HttpServer::parseRequest(const std::string &raw_request)
{
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;

    // Парсинг первой строки (метод, путь, версия)
    if (std::getline(stream, line))
    {
        std::istringstream first_line(line);
        first_line >> request.method >> request.path;

        // Извлечь query string
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos)
        {
            request.query_string = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
    }

    // Парсинг заголовков
    while (std::getline(stream, line) && line != "\r")
    {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // Убрать пробелы
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            request.headers[key] = value;
        }
    }

    // Тело запроса (если есть)
    std::string remaining((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    request.body = remaining;

    return request;
}

} // namespace HttpServer
