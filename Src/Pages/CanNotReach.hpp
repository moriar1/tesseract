/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* CanNotReach.hpp - internal error pages (unreachable URL, bad URL, ...) */

#pragma once

#include "ErrorKit/Error.hpp"

#include <string>

namespace Tess::Pages {

// HTML source for the cannot-reach page. Parsed through the normal
// Tokenize -> Parse pipe, painted with Page.dark (see RenderKit).
inline std::string CannotReachHtml(const std::string &url, Tess::Error::Err err,
                                   int http_status = 0) {
      // NOTE: angle brackets in url would parse as tags. Escaping lands
      // with HTML text-node rules later; internal strings trusted for now.
      std::string page = "<h1>Cannot reach this page</h1><p>" + url + "</p><p>"
            + Tess::Error::Message(err) + "</p>";
      if (http_status != 0) {
            page += "<p>HTTP " + std::to_string(http_status) + "</p>";
      }
      return page + "<p>Check the address and try again.</p><p>" + Tess::Error::Name(err) + "</p>";
}

} // namespace Tess::Pages
