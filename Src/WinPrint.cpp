/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* WinPrint.cpp - WORKAROUND, Windows/MinGW only.
   mingw-w64-gcc 16.x libstdc++ declares std::print terminal helpers
   but does not define them (undefined __open_terminal/__write_to_terminal
   at link). Returning null selects the plain-fwrite path, which is
   correct for our ASCII logs. Recheck each toolchain bump; delete when
   the linker stops complaining. */

#ifdef _WIN32

#include <cstdio>
#include <span>

// NOLINTNEXTLINE: must match libstdc++ <print> internals exactly.
namespace std {

void *__open_terminal(FILE *) {
      return nullptr; // not a terminal -> fwrite fallback
}

// NOLINTNEXTLINE: must match libstdc++ <print> internals exactly.
bool __write_to_terminal(void *, span<char>) {
      return false;
}

} // namespace std

#endif
