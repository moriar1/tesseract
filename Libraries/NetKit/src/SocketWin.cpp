/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* SocketWin.cpp - Winsock TCP backend. Mirrors SocketUnix.cpp exactly:
   same Tess::Net signatures, same -1/false/empty failure contract.
   SOCKET values are cast to int (INVALID_SOCKET truncates to -1). */

#include "NetKit/Net.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace Tess::Net {

namespace {

bool WinsockUp() {
      static bool done = false;
      static bool ok = false;
      if (!done) {
            done = true;
            WSADATA wsa{};
            ok = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
      }
      return ok;
}

} // namespace

ConnectOutcome SocketConnect(const std::string &host, uint16_t port) {
      if (!WinsockUp()) {
            return ConnectOutcome{-1, Tess::Error::Err::ERR_NO_NETWORK};
      }
      struct addrinfo hint{};
      hint.ai_family = AF_UNSPEC;     // IPv4 or IPv6
      hint.ai_socktype = SOCK_STREAM; // TCP

      struct addrinfo *res = nullptr;
      std::string port_str = std::to_string(port);

      if (getaddrinfo(host.c_str(), port_str.c_str(), &hint, &res) != 0 || res == nullptr) {
            return ConnectOutcome{-1, Tess::Error::Err::ERR_NAME_NOT_RESOLVED};
      }

      SOCKET sock = INVALID_SOCKET;
      for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
            sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sock == INVALID_SOCKET) {
                  continue;
            }
            if (connect(sock, p->ai_addr, (int)p->ai_addrlen) == 0) {
                  break;
            }
            closesocket(sock);
            sock = INVALID_SOCKET;
      }

      freeaddrinfo(res);
      if (sock == INVALID_SOCKET) {
            return ConnectOutcome{-1, Tess::Error::Err::ERR_NO_NETWORK};
      }
      return ConnectOutcome{(int)sock, Tess::Error::Err::ERR_UNKNOWN};
}

bool SocketSendAll(int fd, std::string_view data) {
      SOCKET sock = (SOCKET)(UINT_PTR)fd;
      size_t sent = 0;
      while (sent < data.size()) {
            size_t chunk = data.size() - sent;
            if (chunk > INT_MAX) {
                  chunk = INT_MAX;
            }
            int n = send(sock, data.data() + sent, (int)chunk, 0);
            if (n <= 0) {
                  return false;
            }
            sent += (size_t)n;
      }
      return true;
}

std::string SockRecvAll(int fd) {
      SOCKET sock = (SOCKET)(UINT_PTR)fd;
      std::string out;
      char buf[8192];
      for (;;) {
            int n = recv(sock, buf, (int)sizeof(buf), 0);
            if (n <= 0) {
                  break; // 0 = server closed, <0 = error
            }
            out.append(buf, (size_t)n);
      }
      return out;
}

void SocketClose(int fd) {
      closesocket((SOCKET)(UINT_PTR)fd);
}

} // namespace Tess::Net
