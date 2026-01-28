
#include "virtual_com_port.hpp"
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif
#include <sys/select.h>

namespace VirtualCOM
{

    VirtualComPort::VirtualComPort()
        : fd_(-1), is_open_(false), baud_rate_(9600)
    {
    }

    VirtualComPort::VirtualComPort(const std::string &portName, int baudRate)
        : fd_(-1), is_open_(false), baud_rate_(baudRate), port_name_(portName)
    {

        fd_ = ::open(portName.c_str(), O_RDWR | O_NOCTTY);
        if (fd_ == -1)
        {
            throw std::runtime_error("Failed to open port " + portName + ": " + std::string(strerror(errno)));
        }

        if (!configurePort())
        {
            ::close(fd_);
            fd_ = -1;
            throw std::runtime_error("Failed to configure port " + portName);
        }

        is_open_ = true;
    }

    VirtualComPort::~VirtualComPort()
    {
        close();
    }

    void VirtualComPort::close()
    {
        if (is_open_ && fd_ != -1)
        {
            ::close(fd_);
            fd_ = -1;
            is_open_ = false;
        }
    }

    bool VirtualComPort::isOpen() const
    {
        return is_open_;
    }

    int VirtualComPort::write(const std::string &data)
    {
        if (!is_open_ || fd_ == -1)
        {
            return -1;
        }

        ssize_t bytesWritten = ::write(fd_, data.c_str(), data.size());
        return static_cast<int>(bytesWritten);
    }

    std::string VirtualComPort::read(size_t maxBytes, int timeoutMs)
    {
        if (!is_open_ || fd_ == -1)
        {
            return "";
        }

        // Использование select для таймаута
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd_, &readSet);

        struct timeval timeout;
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;

        int selectResult = select(fd_ + 1, &readSet, NULL, NULL, &timeout);

        if (selectResult <= 0)
        {
            return ""; // Таймаут или ошибка
        }

        std::string buffer;
        buffer.resize(maxBytes);

        ssize_t bytesRead = ::read(fd_, &buffer[0], maxBytes);

        if (bytesRead <= 0)
        {
            return "";
        }

        buffer.resize(bytesRead);
        return buffer;
    }

    bool VirtualComPort::configurePort()
    {
        struct termios tty;

        if (tcgetattr(fd_, &tty) != 0)
        {
            return false;
        }

        old_termios_ = tty;

        // Установка скорости передачи
        speed_t speed;
        switch (baud_rate_)
        {
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
            speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        default:
            speed = B9600;
            break;
        }

        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        // 8N1
        tty.c_cflag &= ~PARENB; // Без четности
        tty.c_cflag &= ~CSTOPB; // 1 стоп-бит
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8; // 8 бит данных

        tty.c_cflag &= ~CRTSCTS;       // Без аппаратного управления потоком
        tty.c_cflag |= CREAD | CLOCAL; // Включить чтение, игнорировать модемные линии

        // Raw mode
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_lflag &= ~ECHOE;
        tty.c_lflag &= ~ECHONL;
        tty.c_lflag &= ~ISIG;

        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

        tty.c_oflag &= ~OPOST;
        tty.c_oflag &= ~ONLCR;

        // Таймауты
        tty.c_cc[VTIME] = 10;
        tty.c_cc[VMIN] = 0;

        if (tcsetattr(fd_, TCSANOW, &tty) != 0)
        {
            return false;
        }

        return true;
    }

} // namespace VirtualCOM
