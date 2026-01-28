

#include "virtual_com_port.hpp"
#include <iostream>
#include <stdexcept>
#include <windows.h>

namespace VirtualCOM {

VirtualComPort::VirtualComPort() 
    : handle_(INVALID_HANDLE_VALUE)
    , is_open_(false)
    , baud_rate_(9600) {
}

VirtualComPort::VirtualComPort(const std::string& port_name, int baud_rate) 
    : handle_(INVALID_HANDLE_VALUE)
    , is_open_(false)
    , baud_rate_(baud_rate)
    , port_name_(port_name) {
    
    std::string full_port_name = "\\\\.\\"+port_name;

    handle_ = CreateFileA(
        full_port_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (handle_ == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to open COM port " + port_name + ". Error: " + std::to_string(error));
    }

    if (!configureWindowsPort()) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
        throw std::runtime_error("Failed to configure COM port " + port_name);
    }

    is_open_ = true;
}

VirtualComPort::~VirtualComPort() {
    close();
}

void VirtualComPort::close() {
    if (is_open_ && handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
        is_open_ = false;
    }
}

bool VirtualComPort::isOpen() const {
    return is_open_;
}

int VirtualComPort::write(const std::string& data) {
    if (!is_open_) {
        return -1;
    }

    DWORD bytes_written = 0;
    BOOL success = WriteFile(
        handle_,
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytes_written,
        NULL
    );

    if (!success) {
        return -1;
    }

    return static_cast<int>(bytes_written);
}

std::string VirtualComPort::read(size_t max_bytes, int timeout_ms) {
    if (!is_open_) {
        return "";
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = timeout_ms;
    timeouts.ReadTotalTimeoutConstant = timeout_ms;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    SetCommTimeouts(handle_, &timeouts);

    std::string buffer;
    buffer.resize(max_bytes);
    
    DWORD bytes_read = 0;
    BOOL success = ReadFile(
        handle_,
        &buffer[0],
        static_cast<DWORD>(max_bytes),
        &bytes_read,
        NULL
    );

    if (!success || bytes_read == 0) {
        return "";
    }

    buffer.resize(bytes_read);
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

    dcb.BaudRate = baud_rate_;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(handle_, &dcb)) {
        return false;
    }

    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return true;
}

}
