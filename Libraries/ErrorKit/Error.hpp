/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Error.cpp - List of errors */

#pragma once

namespace Tess::Error {

enum class Err {
      /* Browser Errors */
      ERR_INVALID_URL,
      ERR_NAME_NOT_RESOLVED,
      ERR_NO_NETWORK,
      ERR_NOT_FOUND,
      ERR_TIMEOUT,
      ERR_UNSUPPORTED,

      /* Universal */
      ERR_UNKNOWN,
};

inline const char *Message(Err err) {
      switch (err) {
      case Err::ERR_INVALID_URL:
            return "The address could not be understood.";
      case Err::ERR_NAME_NOT_RESOLVED:
            return "The site name could not be resolved.";
      case Err::ERR_NO_NETWORK:
            return "The server could not be reached.";
      case Err::ERR_NOT_FOUND:
            return "The page was not found.";
      case Err::ERR_TIMEOUT:
            return "The request timed out.";
      case Err::ERR_UNSUPPORTED:
            return "This address type is not supported yet.";
      case Err::ERR_UNKNOWN:
            return "Something went wrong.";
      }
      return "Something went wrong.";
}

inline const char *Name(Err err) {
      switch (err) {
      case Err::ERR_INVALID_URL:
            return "ERR_INVALID_URL";
      case Err::ERR_NAME_NOT_RESOLVED:
            return "ERR_NAME_NOT_RESOLVED";
      case Err::ERR_NO_NETWORK:
            return "ERR_NO_NETWORK";
      case Err::ERR_NOT_FOUND:
            return "ERR_NOT_FOUND";
      case Err::ERR_TIMEOUT:
            return "ERR_TIMEOUT";
      case Err::ERR_UNSUPPORTED:
            return "ERR_UNSUPPORTED";
      case Err::ERR_UNKNOWN:
            return "ERR_UNKNOWN";
      }
      return "ERR_UNKNOWN";
}

} // namespace Tess::Error
