#pragma once

#include "can_drive_framing.hpp"
#include "can_drive.pb.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

namespace dem {

class CanDriveClient {
 public:
  CanDriveClient() = default;
  ~CanDriveClient() { close(); }

  CanDriveClient(const CanDriveClient&) = delete;
  CanDriveClient& operator=(const CanDriveClient&) = delete;

  bool connect(const std::string& host, int port) {
    close();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res = nullptr;
    const std::string port_s = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_s.c_str(), &hints, &res) != 0) {
      return false;
    }

    int fd = -1;
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
      fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (fd < 0) {
        continue;
      }
      if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
        break;
      }
      ::close(fd);
      fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
      return false;
    }
    fd_ = fd;
    return true;
  }

  void close() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }

  bool connected() const { return fd_ >= 0; }

  bool query(const CanDriveRequest& req, CanDriveResponse& resp) {
    if (fd_ < 0) {
      return false;
    }
    if (!send_message(fd_, req)) {
      close();
      return false;
    }
    if (!recv_message(fd_, resp)) {
      close();
      return false;
    }
    return true;
  }

 private:
  int fd_ = -1;
};

}  // namespace dem
