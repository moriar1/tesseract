/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

#include "NetKit/Net.hpp"
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace Tess::Net {

/* Helper */
static std::optional<Response> FetchFile(const Url &url) {
      if (!url.host.empty() && url.host != "localhost") {
            return std::nullopt;
      }
      std::ifstream file(url.path, std::ios::binary);
      if (!file) {
            return std::nullopt;
      }
      std::ostringstream s;
      s << file.rdbuf();

      return Response{200, s.str()};
}

static std::optional<Response> FetchHttp(const Url &url, Tess::Error::Err &err) {
      ConnectOutcome conn = SocketConnect(url.host, url.port);
      if (conn.fd < 0) {
            err = conn.err;
            return std::nullopt;
      }
      int fd = conn.fd;
      std::string target = url.path.empty() ? "/" : url.path;
      if (!url.query.empty()) {
            target += "?" + url.query;
      }

      std::string req
            = "GET " + target + " HTTP/1.0\r\nHost: " + url.host + "\r\nConnection: close\r\n\r\n";
      if (!SocketSendAll(fd, req)) {
            SocketClose(fd);
            err = Tess::Error::Err::ERR_NO_NETWORK;
            return std::nullopt;
      }
      std::string raw = SockRecvAll(fd);
      SocketClose(fd);

      // Split headers/body, status from the first line. Headers dropped in v1.
      size_t split = raw.find("\r\n\r\n");
      if (split == std::string::npos) {
            err = Tess::Error::Err::ERR_NO_NETWORK;
            return std::nullopt;
      }
      size_t line_end = raw.find("\r\n");
      if (line_end == std::string::npos) {
            err = Tess::Error::Err::ERR_NO_NETWORK;
            return std::nullopt;
      }
      std::string_view status_line(raw.data(), line_end);
      size_t sp1 = status_line.find(' ');
      if (sp1 == std::string_view::npos) {
            err = Tess::Error::Err::ERR_NO_NETWORK;
            return std::nullopt;
      }
      size_t sp2 = status_line.find(' ', sp1 + 1);
      std::string_view code = status_line.substr(sp1 + 1, sp2 - sp1 - 1);
      if (code.empty()) {
            err = Tess::Error::Err::ERR_NO_NETWORK;
            return std::nullopt;
      }
      int status = 0;
      for (char c : code) {
            if (!std::isdigit((unsigned char)c)) {
                  err = Tess::Error::Err::ERR_NO_NETWORK;
                  return std::nullopt;
            }
            status = status * 10 + (c - '0');
      }
      return Response{status, raw.substr(split + 4)};
}

/* Main */
std::optional<Response> FetchResponse(const Url &url, Tess::Error::Err &err) {
      err = Tess::Error::Err::ERR_UNKNOWN;
      if (url.scheme == "file") {
            auto res = FetchFile(url);
            if (!res) {
                  err = Tess::Error::Err::ERR_NOT_FOUND;
            }
            return res;
      }
      if (url.scheme == "http") {
            return FetchHttp(url, err);
      }

      err = Tess::Error::Err::ERR_UNSUPPORTED; // https and the rest, later
      return std::nullopt;
}

} // namespace Tess::Net
