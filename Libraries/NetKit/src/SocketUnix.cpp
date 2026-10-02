/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

#include "NetKit/Net.hpp"

#include <cstddef>
#include <netdb.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace Tess::Net {

ConnectOutcome SocketConnect(const std::string &host, uint16_t port) {
      struct addrinfo hint{};
      hint.ai_family = AF_UNSPEC;     // IPv4 or IPv6
      hint.ai_socktype = SOCK_STREAM; // TCP

      struct addrinfo *res = nullptr;
      std::string port_str = std::to_string(port);

      int err = getaddrinfo(host.c_str(), port_str.c_str(), &hint, &res);
      if (err != 0 || res == nullptr) {
            return ConnectOutcome{-1, Tess::Error::Err::ERR_NAME_NOT_RESOLVED};
      }

      int fd = -1;
      for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd == -1) {
                  continue;
            }
            if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
                  break;
            }
            close(fd);
            fd = -1;
      }

      freeaddrinfo(res);
      if (fd == -1) {
            return ConnectOutcome{-1, Tess::Error::Err::ERR_NO_NETWORK};
      }
      return ConnectOutcome{fd, Tess::Error::Err::ERR_UNKNOWN};
}

bool SocketSendAll(int fd, std::string_view data) {
      size_t sent = 0;
      while (sent < data.size()) {
            ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
            if (n <= 0) {
                  return false;
            }
            sent += (size_t)n;
      }
      return true;
}

std::string SockRecvAll(int fd) {
      std::string out;
      char buf[8192];
      for (;;) {
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                  break; // 0 = server closed, <0 = error
            }
            out.append(buf, (size_t)n);
      }
      return out;
}

void SocketClose(int fd) {
      close(fd);
}

} // namespace Tess::Net
