#ifdef _WIN32

#include "../include/virtual_com_port.hpp"
#include <iostream>
#include <stdexcept>

namespace VirtualCOM {

VirtualComPort::VirtualComPort() 
    : handle_(INVALID_HANDLE_VALUE)
    , isOpen_(false)
    , baudRate_(9600) {
}

VirtualComPort::VirtualComPort(const std::string& portName, int baudRate) 
    : handle_(INVALID_HANDLE_VALUE)
    , isOpen_(false)
    , baudRate_(baudRate)
    , portName_(portName) {
    
    // Добавляем префикс \\..\ для Windows API
    std::string fullPortName = "\\\\.\\"+portName;

    handle_ = CreateFileA(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (handle_ == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to open COM port " + portName + ". Error: " + std::to_string(error));
    }

    if (!configureWindowsPort()) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
        throw std::runtime_error("Failed to configure COM port " + portName);
    }

    isOpen_ = true;
}

VirtualComPort::~VirtualComPort() {
    close();
}

void VirtualComPort::close() {
    if (isOpen_ && handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
        isOpen_ = false;
    }
}

bool VirtualComPort::isOpen() const {
    return isOpen_;
}

int VirtualComPort::write(const std::string& data) {
    if (!isOpen_) {
        return -1;
    }

    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        handle_,
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        NULL
    );

    if (!success) {
        return -1;
    }

    return static_cast<int>(bytesWritten);
}

std::string VirtualComPort::read(size_t maxBytes, int timeoutMs) {
    if (!isOpen_) {
        return "";
    }

    // Установка таймаута
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = timeoutMs;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    SetCommTimeouts(handle_, &timeouts);

    std::string buffer;
    buffer.resize(maxBytes);
    
    DWORD bytesRead = 0;
    BOOL success = ReadFile(
        handle_,
        &buffer[0],
        static_cast<DWORD>(maxBytes),
        &bytesRead,
        NULL
    );

    if (!success || bytesRead == 0) {
        return "";
    }

    buffer.resize(bytesRead);
    return buffer;
}

bool VirtualComPort::configurePort() {
    return configureWindowsPort();
}

bool VirtualComPort::configureWindowsPort() {
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(handle_, &dcb)) {
        return false;
    }

    dcb.BaudRate = baudRate_;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(handle_, &dcb)) {
        return false;
    }

    // Очистка буферов
    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return true;
}

} // namespace VirtualCOM

#endif // _WIN32
