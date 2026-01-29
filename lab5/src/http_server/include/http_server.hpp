#pragma once

#include <string>
#include <functional>
#include <map>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

namespace HttpServer
{

#ifdef _WIN32
typedef SOCKET socket_t;
#else
typedef int socket_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

/**
 * @brief HTTP запрос
 */
struct HttpRequest
{
    std::string method;
    std::string path;
    std::string query_string;
    std::map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief HTTP ответ
 */
struct HttpResponse
{
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : status_code(200), status_message("OK") {}

    void setJson(const std::string &json_data)
    {
        body = json_data;
        headers["Content-Type"] = "application/json; charset=utf-8";
    }

    void setHtml(const std::string &html_data)
    {
        body = html_data;
        headers["Content-Type"] = "text/html; charset=utf-8";
    }

    void setPlainText(const std::string &text)
    {
        body = text;
        headers["Content-Type"] = "text/plain; charset=utf-8";
    }

    std::string toString() const;
};

/**
 * @brief Callback для обработки HTTP запросов
 */
using HttpHandler = std::function<void(const HttpRequest &, HttpResponse &)>;

/**
 * @brief Простой кроссплатформенный HTTP сервер
 */
class HttpServer
{
public:
    HttpServer(int port = 8080);
    ~HttpServer();

    /**
     * @brief Запустить сервер
     * @return true если успешно
     */
    bool start();

    /**
     * @brief Остановить сервер
     */
    void stop();

    /**
     * @brief Зарегистрировать обработчик для пути
     */
    void registerHandler(const std::string &path, HttpHandler handler);

    /**
     * @brief Проверка, запущен ли сервер
     */
    bool isRunning() const { return running_; }

private:
    int port_;
    socket_t server_socket_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    std::map<std::string, HttpHandler> handlers_;

    void serverLoop();
    void handleClient(socket_t client_socket);
    HttpRequest parseRequest(const std::string &raw_request);
    void initializeSocket();
    void cleanupSocket();

#ifdef _WIN32
    WSADATA wsa_data_;
#endif
};

} // namespace HttpServer
