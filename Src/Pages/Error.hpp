/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Error.hpp - internal error pages (unreachable URL, bad URL, ...) */

#pragma once

#include <string>

namespace Tess::Pages {

// HTML source for the cannot-reach page. Parsed through the normal
// Tokenize -> Parse pipe, painted with Page.dark (see RenderKit).
inline std::string CannotReachHtml(const std::string &url, const std::string &reason) {
      // NOTE: angle brackets in url/reason would parse as tags.
      // Escaping lands with HTML text-node rules later; internal
      // strings are trusted for now.
      return "<h1>Cannot reach this page</h1><p>" + url + "</p><p>" + reason
             + "</p><p>Check the address and try again.</p>";
}

} // namespace Tess::Pages
