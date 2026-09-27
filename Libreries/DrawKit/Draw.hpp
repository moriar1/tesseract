/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Draw wrapper librery for helping in rendering */

#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

namespace Tess::Draw {

/* Outline.cpp */
void DrawArc(SDL_Renderer *r, float cx, float cy, float radius, float start_deg, float end_deg);
void DrawRoundedOutline(SDL_Renderer *r, SDL_FRect rect, float radius);
void DrawText(SDL_Renderer *r, TTF_Text *txt, const std::string &str, float x, float y);
void DrawCaret(SDL_Renderer *r, TTF_Text *txt, float x, float y);
void TextSize(TTF_Text *txt, int &w, int &h);

/* Fill.cpp */
void FillCircle(SDL_Renderer *r, float cx, float cy, float radius);
void FillRoundedRect(SDL_Renderer *r, SDL_FRect rect, float radius);

/* Widget.cpp - retained text field: value, caret, selection, clipboard */
struct TextField {
      std::string value;
      size_t cursor = 0;     // byte offset (ASCII-first; multibyte splits)
      long sel_anchor = -1;  // byte offset, -1 = no selection
      bool focused = false;
      float scroll_x = 0.0f;
      bool submitted = false; // Enter: caller consumes + clears
};
// Returns true when the field consumed the event.
// txt is borrowed for click-to-position measuring only.
bool FieldEvent(TextField &f, const SDL_Event &e, SDL_FRect box, TTF_Text *txt);
void FieldDraw(TextField &f, SDL_Renderer *r, TTF_Text *txt, SDL_FRect box);

} // namespace Tess::Draw
