/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Widget.cpp - retained single-line text field */

#include "DrawKit/Draw.hpp"
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_keyboard.h>
#include <algorithm>

namespace Tess::Draw {

namespace {

// Width of first n bytes rendered in txt (txt is set + measured).
float PrefixWidth(TTF_Text *txt, const std::string &s, size_t n) {
      if (n > s.size()) {
            n = s.size();
      }
      std::string part = s.substr(0, n);
      TTF_SetTextString(txt, part.c_str(), part.size());
      int w = 0, h = 0;
      TextSize(txt, w, h);
      return (float)w;
}

// Byte offset of char nearest to x (x in text space, already scrolled).
size_t OffsetAtX(TTF_Text *txt, const std::string &s, float x) {
      size_t best = 0;
      float best_dist = x >= 0.0f ? x : -x;
      for (size_t n = 0; n <= s.size(); ++n) {
            float dist = PrefixWidth(txt, s, n) - x;
            if (dist < 0.0f) {
                  dist = -dist;
            }
            if (dist < best_dist) {
                  best_dist = dist;
                  best = n;
            }
      }
      return best;
}

bool HasSel(const TextField &f) {
      return f.sel_anchor >= 0 && (size_t)f.sel_anchor != f.cursor;
}

void ClearSel(TextField &f) {
      f.sel_anchor = -1;
}

void DeleteSel(TextField &f) {
      if (!HasSel(f)) {
            return;
      }
      size_t a = f.cursor;
      size_t b = (size_t)f.sel_anchor;
      if (a > b) {
            std::swap(a, b);
      }
      f.value.erase(a, b - a);
      f.cursor = a;
      ClearSel(f);
}

} // namespace

bool FieldEvent(TextField &f, const SDL_Event &e, SDL_FRect box, TTF_Text *txt) {
      if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mx = e.button.x;
            float my = e.button.y;
            bool hit = mx >= box.x && mx <= box.x + box.w && my >= box.y && my <= box.y + box.h;
            f.focused = hit;
            if (hit && txt) {
                  // Caret to nearest char; scroll follows on next draw.
                  float pad = 8.0f;
                  f.cursor = OffsetAtX(txt, f.value, mx - (box.x + pad - f.scroll_x));
                  ClearSel(f);
            } else if (!hit) {
                  ClearSel(f);
            }
            return hit;
      }
      if (!f.focused) {
            return false;
      }
      if (e.type == SDL_EVENT_TEXT_INPUT) {
            DeleteSel(f);
            f.value.insert(f.cursor, e.text.text);
            f.cursor += std::string(e.text.text).size();
            return true;
      }
      if (e.type != SDL_EVENT_KEY_DOWN) {
            return false;
      }
      bool ctrl = (e.key.mod & SDL_KMOD_CTRL) != 0;
      bool shift = (e.key.mod & SDL_KMOD_SHIFT) != 0;
      SDL_Keycode key = e.key.key;

      if (ctrl && (key == SDLK_C || key == SDLK_X)) {
            if (HasSel(f)) {
                  size_t a = f.cursor;
                  size_t b = (size_t)f.sel_anchor;
                  if (a > b) {
                        std::swap(a, b);
                  }
                  SDL_SetClipboardText(f.value.substr(a, b - a).c_str());
                  if (key == SDLK_X) {
                        DeleteSel(f);
                  }
            }
            return true;
      }
      if (ctrl && key == SDLK_V) {
            char *clip = SDL_GetClipboardText();
            if (clip && *clip) {
                  DeleteSel(f);
                  f.value.insert(f.cursor, clip);
                  f.cursor += std::string(clip).size();
            }
            SDL_free(clip);
            return true;
      }
      if (ctrl && key == SDLK_A) {
            f.sel_anchor = 0;
            f.cursor = f.value.size();
            return true;
      }

      auto move = [&](long delta) {
            if (!shift) {
                  ClearSel(f);
            } else if (!HasSel(f)) {
                  f.sel_anchor = (long)f.cursor;
            }
            long next = (long)f.cursor + delta;
            if (next < 0) {
                  next = 0;
            }
            if (next > (long)f.value.size()) {
                  next = (long)f.value.size();
            }
            f.cursor = (size_t)next;
      };

      switch (key) {
      case SDLK_LEFT:
            move(-1);
            return true;
      case SDLK_RIGHT:
            move(1);
            return true;
      case SDLK_HOME:
            move(-(long)f.value.size());
            return true;
      case SDLK_END:
            move((long)f.value.size());
            return true;
      case SDLK_BACKSPACE:
            if (HasSel(f)) {
                  DeleteSel(f);
            } else if (f.cursor > 0) {
                  f.value.erase(f.cursor - 1, 1);
                  --f.cursor;
            }
            return true;
      case SDLK_DELETE:
            if (HasSel(f)) {
                  DeleteSel(f);
            } else if (f.cursor < f.value.size()) {
                  f.value.erase(f.cursor, 1);
            }
            return true;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
            ClearSel(f);
            f.submitted = true;
            return true;
      default:
            break;
      }
      return false;
}

void FieldDraw(TextField &f, SDL_Renderer *r, TTF_Text *txt, SDL_FRect box) {
      // Frame: dark fill, blue when focused.
      SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
      SDL_RenderFillRect(r, &box);
      if (f.focused) {
            SDL_SetRenderDrawColor(r, 80, 160, 255, 255);
      } else {
            SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
      }
      SDL_RenderRect(r, &box);

      // Clip text to the field.
      SDL_Rect clip = {(int)box.x, (int)box.y, (int)box.w, (int)box.h};
      SDL_SetRenderClipRect(r, &clip);

      float pad = 8.0f;
      float field_w = box.w - pad * 2.0f;
      // Keep caret visible: scroll follows cursor both directions.
      {
            float caret_x = PrefixWidth(txt, f.value, f.cursor);
            if (caret_x - f.scroll_x < 0.0f) {
                  f.scroll_x = caret_x;
            } else if (caret_x - f.scroll_x > field_w) {
                  f.scroll_x = caret_x - field_w;
            }
            if (f.scroll_x < 0.0f) {
                  f.scroll_x = 0.0f;
            }
      }
      float tx = box.x + pad - f.scroll_x;
      float ty = box.y + 5.0f;

      // Selection highlight behind text.
      if (HasSel(f)) {
            size_t a = f.cursor;
            size_t b = (size_t)f.sel_anchor;
            if (a > b) {
                  std::swap(a, b);
            }
            float sx = tx + PrefixWidth(txt, f.value, a);
            float ex = tx + PrefixWidth(txt, f.value, b);
            int unused_w = 0, unused_h = 0;
            TextSize(txt, unused_w, unused_h);
            SDL_FRect hi = {sx, ty, ex - sx, (float)unused_h};
            SDL_SetRenderDrawColor(r, 60, 120, 200, 255);
            SDL_RenderFillRect(r, &hi);
      }

      DrawText(r, txt, f.value, tx, ty);

      if (f.focused) {
            // Caret at cursor: measure the prefix, not the full text.
            std::string pre = f.value.substr(0, f.cursor);
            TTF_SetTextString(txt, pre.c_str(), pre.size());
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            DrawCaret(r, txt, tx, ty);
            // Restore full text so next measure starts clean.
            TTF_SetTextString(txt, f.value.c_str(), f.value.size());
      }
      SDL_SetRenderClipRect(r, nullptr);
}

} // namespace Tess::Draw
