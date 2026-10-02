/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Net.cpp - NetKit networking stack */

#pragma once

#include "ErrorKit/Error.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Tess::Net {

/*=============== URL ===============*/
/*
  https://user:pass@example.com:8443/path/to/page?query=1&foo=bar#section
  \___/   \_______/ \_________/ \__/\____________/ \____________/ \_____/
  scheme   userinfo    host     port     path          query      fragment
*/
struct Url {
      std::string scheme;
      std::string host;
      std::string path;
      std::string query;
      std::string fragment;
      uint16_t port = 0; // 0 = "not specified"
};
std::optional<Url> ParseUrl(std::string_view raw);
std::string Resolve(const Url &base, const std::string &href);

/*=============== Fetch Response ===============*/
struct Response {
      int status;
      std::string body;
};
// err carries the precise failure; check it when the optional is empty.
std::optional<Response> FetchResponse(const Url &url, Tess::Error::Err &err);

/*=============== Socket ===============*/
struct ConnectOutcome {
      int fd = -1;
      Tess::Error::Err err = Tess::Error::Err::ERR_NO_NETWORK;
};
ConnectOutcome SocketConnect(const std::string &host, uint16_t port);
bool SocketSendAll(int fd, std::string_view data);
// till recv==0, empty on immediate error
std::string SockRecvAll(int fd);
void SocketClose(int fd);

} // namespace Tess::Net
