#pragma once

// Length-prefixed Protobuf framing over a POSIX TCP socket.
// Frame: uint32 big-endian length + protobuf payload.

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <google/protobuf/message.h>

namespace dem {

inline constexpr std::size_t kMaxFrameBytes = 4u * 1024u * 1024u;

inline bool send_all(int fd, const void* data, std::size_t n) {
  const auto* p = static_cast<const std::uint8_t*>(data);
  std::size_t sent = 0;
  while (sent < n) {
    const ssize_t r = ::send(fd, p + sent, n - sent, 0);
    if (r <= 0) {
      return false;
    }
    sent += static_cast<std::size_t>(r);
  }
  return true;
}

inline bool recv_exact(int fd, void* data, std::size_t n) {
  auto* p = static_cast<std::uint8_t*>(data);
  std::size_t got = 0;
  while (got < n) {
    const ssize_t r = ::recv(fd, p + got, n - got, 0);
    if (r <= 0) {
      return false;
    }
    got += static_cast<std::size_t>(r);
  }
  return true;
}

inline bool send_message(int fd, const google::protobuf::Message& msg) {
  std::string body;
  if (!msg.SerializeToString(&body)) {
    return false;
  }
  if (body.size() > kMaxFrameBytes) {
    return false;
  }
  const std::uint32_t be = htonl(static_cast<std::uint32_t>(body.size()));
  if (!send_all(fd, &be, sizeof(be))) {
    return false;
  }
  return send_all(fd, body.data(), body.size());
}

inline bool recv_message(int fd, google::protobuf::Message& msg) {
  std::uint32_t be = 0;
  if (!recv_exact(fd, &be, sizeof(be))) {
    return false;
  }
  const std::uint32_t n = ntohl(be);
  if (n > kMaxFrameBytes) {
    return false;
  }
  std::vector<char> body(n);
  if (n > 0 && !recv_exact(fd, body.data(), n)) {
    return false;
  }
  return msg.ParseFromArray(body.data(), static_cast<int>(n));
}

}  // namespace dem
