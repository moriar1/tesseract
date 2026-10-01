/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Tokenize.cpp - Html tokenizer */

#include "HtmlKit/Html.hpp"
#include <cctype>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Tess::Html {

/* Helper */
namespace {

bool IsName(char c) {
      return std::isalnum((unsigned char)c);
}
std::string Lower(std::string_view s) {
      std::string r;
      r.reserve(s.size());
      for (char c : s) {
            r += (char)std::tolower((unsigned char)c);
      }
      return r;
}

// Parse '<tag ...>' at src front (caller guarantees '<' + letter).
// Consumes through '>'. Null when no tag name follows.
std::optional<Token> ParseTagOpen(std::string_view &src) {
      src.remove_prefix(1); // Skip '<'

      size_t name_len = 0;
      while (name_len < src.size() && IsName(src[name_len])) {
            ++name_len;
      }
      std::string_view name = src.substr(0, name_len);
      if (name.empty()) {
            return std::nullopt;
      }

      // Attributes: name="v" | name='v' | name=v | name
      std::map<std::string, std::string> attrs;
      src.remove_prefix(name_len);
      while (!src.empty() && src[0] != '>') {
            while (!src.empty() && std::isspace((unsigned char)src[0])) {
                  src.remove_prefix(1);
            }
            if (src.empty() || src[0] == '>' || src[0] == '/') {
                  break;
            }

            size_t an = 0;
            while (an < src.size() && (IsName(src[an]) || src[an] == '-')) {
                  ++an;
            }
            std::string key = Lower(src.substr(0, an));
            src.remove_prefix(an);
            while (!src.empty() && std::isspace((unsigned char)src[0])) {
                  src.remove_prefix(1);
            }
            std::string val;
            if (!src.empty() && src[0] == '=') {
                  src.remove_prefix(1);
                  while (!src.empty() && std::isspace((unsigned char)src[0])) {
                        src.remove_prefix(1);
                  }
                  if (!src.empty() && (src[0] == '"' || src[0] == '\'')) {
                        char q = src[0];
                        src.remove_prefix(1);
                        size_t e = src.find(q);
                        size_t n = (e == std::string_view::npos) ? src.size() : e;
                        val = std::string(src.substr(0, n));
                        src.remove_prefix(e == std::string_view::npos ? n : n + 1);
                  } else {
                        size_t n = 0;
                        while (n < src.size() && !std::isspace((unsigned char)src[n])
                               && src[n] != '>') {
                              ++n;
                        }
                        val = std::string(src.substr(0, n));
                        src.remove_prefix(n);
                  }
            }
            if (!key.empty()) {
                  attrs[key] = val;
            }
      }
      if (!src.empty() && src[0] == '>') {
            // (strip optional '/' before it for <x/> later)
            src.remove_prefix(1);
      }
      return Token{TokenKind::TK_TagOpen, Lower(name), std::move(attrs)};
}

} // namespace

/* Main */
std::vector<Token> Tokenize(std::string_view src) {
      std::vector<Token> out;

      while (!src.empty()) {
            // skip comment
            if (src.starts_with("<!--")) {
                  size_t e = src.find("-->", 4);
                  size_t skip = (e == std::string_view::npos) ? src.size() : e + 3;
                  src.remove_prefix(skip);
            }
            // skip <!DOCTYPE ...> and other <! declarations
            else if (src.starts_with("<!")) {
                  size_t gt = src.find('>');
                  size_t skip = (gt == std::string_view::npos) ? src.size() : gt + 1;
                  src.remove_prefix(skip);
            }
            // closing tag
            else if (src.starts_with("</")) {
                  src.remove_prefix(2);

                  size_t name_len = 0;
                  while (name_len < src.size() && IsName(src[name_len])) {
                        ++name_len;
                  }
                  std::string_view name = src.substr(0, name_len);

                  // Skip to and past the closing '>'
                  size_t gt = src.find('>');
                  size_t skip = (gt == std::string_view::npos) ? src.size() : gt + 1;
                  src.remove_prefix(skip);

                  if (!name.empty()) {
                        out.push_back({TokenKind::TK_TagClose, Lower(name)});
                  }
            }
            // opening tag: '<' followed by a letter, else it's plain text
            else if (src.size() > 1 && src[0] == '<' && std::isalpha((unsigned char)src[1])) {
                  if (auto tok = ParseTagOpen(src)) {
                        out.push_back(std::move(*tok));
                  }
            }
            // text
            else {
                  // text up to the next '<'; a lone '<' becomes literal text
                  size_t j = src.find('<');
                  size_t text_len = (j == std::string_view::npos) ? src.size() : j;
                  if (text_len == 0) {
                        text_len = 1;
                  }
                  out.push_back({TokenKind::TK_Text, std::string(src.substr(0, text_len))});
                  src.remove_prefix(text_len);
            }
      }

      return out;
}

} // namespace Tess::Html
