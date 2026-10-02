/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

#include "NetKit/Net.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace Tess::Net {

/* Helper */
namespace {

std::optional<uint16_t> ParsePort(std::string_view s) {
      if (s.empty() || s.size() > 5) {
            return std::nullopt;
      }
      long port = 0;
      for (char c : s) {
            if (!std::isdigit((unsigned char)c)) {
                  return std::nullopt;
            }
            port = port * 10 + (c - '0');
      }
      if (port <= 0 || port > 65535) {
            return std::nullopt;
      }
      return (uint16_t)port;
}

std::string Sterialize(const Url &url, const std::string &path) {
      std::string out = url.scheme + "://" + url.host;
      bool def = (url.scheme == "http" && url.port == 80)
            || (url.scheme == "https" && url.port == 443);

      if (url.port != 0 && !def) {
            out += ":" + std::to_string(url.port);
      }
      return out + path;
}

} // namespace

/* Main */
std::optional<Url> ParseUrl(std::string_view raw) {
      Url url;
      size_t pos = 0;

      // scheme
      auto scheme_end = raw.find("://");
      if (scheme_end == std::string::npos) {
            return std::nullopt;
      }
      url.scheme = raw.substr(0, scheme_end)
            | std::views::transform([](unsigned char c) { return std::tolower(c); })
            | std::ranges::to<std::string>();
      pos = scheme_end + 3;

      // authority (upto / ? # or end)
      auto auth_end = raw.find_first_of("/?#", pos);
      if (auth_end == std::string::npos) {
            auth_end = raw.size();
      }
      std::string authority{raw.substr(pos, auth_end - pos)};
      pos = auth_end;

      // strip userinfo if present
      auto at_pos = authority.find('@');
      if (at_pos != std::string::npos) {
            authority = authority.substr(at_pos + 1); // discarding user:pass for now
      }

      // host + optional port, handling [ipv6] (brackets stripped)
      if (!authority.empty() && authority[0] == '[') {
            auto close_bracket = authority.find(']');
            if (close_bracket == std::string::npos) {
                  return std::nullopt;
            }
            url.host = authority.substr(1, close_bracket - 1);
            if (close_bracket + 1 < authority.size() && authority[close_bracket + 1] == ':') {
                  auto port = ParsePort(authority.substr(close_bracket + 2));
                  if (!port) {
                        return std::nullopt;
                  }
                  url.port = *port;
            }
      } else {
            auto colon_pos = authority.find(':');
            if (colon_pos != std::string::npos) {
                  url.host = authority.substr(0, colon_pos);
                  auto port = ParsePort(authority.substr(colon_pos + 1));
                  if (!port) {
                        return std::nullopt;
                  }
                  url.port = *port;
            } else {
                  url.host = authority;
            }
      }

      // default ports when unspecified
      if (url.port == 0) {
            if (url.scheme == "http") {
                  url.port = 80;
            } else if (url.scheme == "https") {
                  url.port = 443;
            }
      }

      // path
      if (pos < raw.size() && raw[pos] == '/') {
            size_t path_end = raw.find_first_of("?#", pos);
            if (path_end == std::string::npos) {
                  path_end = raw.size();
            }
            url.path = raw.substr(pos, path_end - pos);
            pos = path_end;
      }

      // query
      if (pos < raw.size() && raw[pos] == '?') {
            size_t query_end = raw.find('#', pos);
            if (query_end == std::string::npos) {
                  query_end = raw.size();
            }
            url.query = raw.substr(pos + 1, query_end - pos - 1);
            pos = query_end;
      }

      // fragment
      if (pos < raw.size() && raw[pos] == '#') {
            url.fragment = raw.substr(pos + 1);
      }

      return url;
}

std::string Resolve(const Url &base, const std::string &href) {
      if (href.empty()) {
            return "";
      }
      if (href.find("://") != std::string::npos) {
            return ParseUrl(href) ? href : "";
      }
      if (base.scheme.empty()) {
            return "";
      }
      bool is_file = (base.scheme == "file");
      if (!is_file && base.host.empty()) {
            return "";
      }
      if (href[0] == '/') {
            return Sterialize(base, href);
      }

      // fold ./ and ../
      std::string dir = base.path;
      size_t slash = dir.rfind('/');
      dir = (slash == std::string::npos) ? "/" : dir.substr(0, slash + 1);
      std::string merged = dir + href;
      std::vector<std::string> segs;
      size_t i = 0;
      while (i < merged.size()) {
            size_t j = merged.find('/', i);
            if (j == std::string::npos) {
                  j = merged.size();
            }
            std::string part = merged.substr(i, j - i);
            if (part.empty() || part == ".") {
                  // skip
            } else if (part == "..") {
                  if (!segs.empty()) {
                        segs.pop_back();
                  }
            } else {
                  segs.push_back(part);
            }
            i = j + 1;
      }

      std::string path = "/";
      for (size_t k = 0; k < segs.size(); ++k) {
            if (k > 0) {
                  path += '/';
            }
            path += segs[k];
      }
      return Sterialize(base, path);
}

} // namespace Tess::Net
