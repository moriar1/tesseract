/*
** Tesseract - A simple browser engine.
** https://github.com/tanvir-techbro/tesseract
**
** Copyright (c) 2026-present tanvir-techbro (Tanvir)
** Released under the MIT License.
*/

/* Tess.cpp file - Main */

#include "DrawKit/Draw.hpp"
#include "HtmlKit/Html.hpp"
#include "NetKit/Net.hpp"
#include "RenderKit/Render.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <print>
#include <string>

int main(int argc, char *argv[]) {

      /* Window Initialization */
      if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::println(stderr, "SDL Init failed: {}", SDL_GetError());
            return 1;
      }
      if (!TTF_Init()) {
            std::println(stderr, "TTF Init failed: {}", SDL_GetError());
            return 1;
      }

      int window_width = 800;
      int window_height = 600;

      SDL_Window *window = SDL_CreateWindow("tesseract", window_width, window_height, SDL_WINDOW_RESIZABLE);
      if (!window) {
            std::println(stderr, "Window creation failed: {}", SDL_GetError());
            SDL_Quit();
            return 1;
      }
      SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
      if (!renderer) {
            std::println(stderr, "Renderer creation failed: {}", SDL_GetError());
            SDL_Quit();
            return 1;
      }

      TTF_Font *font = TTF_OpenFont("Assets/fonts/CaskaydiaCoveNerdFontMono-Regular.ttf", 16);
      if (!font) {
            font = TTF_OpenFont("../Assets/fonts/CaskaydiaCoveNerdFontMono-Regular.ttf", 16);
      }
      if (!font) {
            std::println(stderr, "font failed: {}", SDL_GetError());
      }
      TTF_TextEngine *eng = TTF_CreateRendererTextEngine(renderer);
      TTF_Text *txt = TTF_CreateText(eng, font, "", 0);
      TTF_SetTextColor(txt, 255, 255, 255, 255);
      // Page prose typeface: sans fallback (no CSS yet, monospace stays in chrome)
      std::string page_font_path = "Assets/fonts/NotoSans-Regular.ttf";
      {
            std::ifstream probe(page_font_path, std::ios::binary);
            if (!probe) {
                  page_font_path = "../Assets/fonts/NotoSans-Regular.ttf";
            }
      }
      Tess::Render::Typeface page_face(eng, page_font_path);

      /* Main drawing and events and element stuff */
      bool running = true;
      SDL_Event event;
      SDL_StartTextInput(window);

      Tess::Draw::TextField url_bar;
      url_bar.focused = true;
      float scroll_y = 0.0f;

      // Page source: navigated from the URL bar on Enter (sample first)
      Tess::Html::Document doc
            = Tess::Html::Parse(Tess::Html::Tokenize("<h1>tesseract</h1><p>type a file:// url, hit enter</p>"));
      Tess::Render::Page content;
      bool page_dirty = true;

      while (running) {
            // Chrome slots: [back][forward] url... [menu], 24px bar at y=5
            SDL_FRect back_box = {5.0f, 5.0f, 24.0f, 24.0f};
            SDL_FRect fwd_box = {31.0f, 5.0f, 24.0f, 24.0f};
            SDL_FRect menu_box = {(float)window_width - 39.0f, 5.0f, 34.0f, 24.0f};
            SDL_FRect url_box = {
                  57.0f,
                  5.0f,
                  (float)window_width - 57.0f - 41.0f,
                  28.0f,
            };
            auto HitBox = [](SDL_FRect b, float mx, float my) {
                  return mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h;
            };

            while (SDL_PollEvent(&event)) {
                  if (event.type == SDL_EVENT_QUIT) {
                        running = false;
                  }
                  // 1. Update window dimensions when resized
                  else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                        window_width = event.window.data1;
                        window_height = event.window.data2;
                        page_dirty = true;
                  } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                        scroll_y -= event.wheel.y * 40.0f;
                  } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                        float mx = event.button.x;
                        float my = event.button.y;
                        if (HitBox(back_box, mx, my)) {
                              // TODO(history): go back when history exists
                        } else if (HitBox(fwd_box, mx, my)) {
                              // TODO(history): go forward when history exists
                        } else if (HitBox(menu_box, mx, my)) {
                              // TODO(settings): open menu when settings exist
                        } else {
                              Tess::Draw::FieldEvent(url_bar, event, url_box, txt);
                        }
                  } else if (Tess::Draw::FieldEvent(url_bar, event, url_box, txt)) {
                        if (url_bar.submitted) {
                              url_bar.submitted = false;
                              std::string raw = url_bar.value;
                              // bare path -> absolute file:// URL
                              if (!raw.empty()) {
                                    if (raw.find("://") == std::string::npos) {
                                          std::error_code ec;
                                          raw = "file://" + std::filesystem::absolute(raw, ec).string();
                                    }
                                    if (auto u = Tess::Net::ParseUrl(raw)) {
                                          if (auto res = Tess::Net::FetchResponse(*u)) {
                                                doc = Tess::Html::Parse(Tess::Html::Tokenize(res->body));
                                                page_dirty = true;
                                          } else {
                                                std::println(stderr, "fetch failed: {}", url_bar.value);
                                          }
                                    } else {
                                          std::println(stderr, "bad url: {}", url_bar.value);
                                    }
                              }
                        }
                  }
            }

            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
            SDL_RenderClear(renderer);

            // URL bar owns white/16pt; page text uses its own Typeface
            TTF_SetFontSize(font, 16);
            // Chrome buttons: back/fwd dimmed (no history yet), menu white + bigger
            TTF_SetTextColor(txt, 100, 100, 100, 255);
            Tess::Draw::DrawText(renderer, txt, "←", back_box.x + 6, back_box.y + 2);
            Tess::Draw::DrawText(renderer, txt, "→", fwd_box.x + 6, fwd_box.y + 2);
            TTF_SetFontSize(font, 20);
            TTF_SetTextColor(txt, 255, 255, 255, 255);
            Tess::Draw::DrawText(renderer, txt, "≡", menu_box.x + 9, menu_box.y + 1);
            TTF_SetFontSize(font, 16);
            TTF_SetTextColor(txt, 255, 255, 255, 255);
            Tess::Draw::FieldDraw(url_bar, renderer, txt, url_box);

            // Page viewport: 5px under URL bar, 5px off left/right/bottom
            float page_y = url_box.y + url_box.h + 5.0f;
            SDL_FRect page = {
                  5.0f,
                  page_y,
                  (float)window_width - 10.0f,
                  (float)window_height - page_y - 5.0f,
            };
            if (page.w < 0.0f) {
                  page.w = 0.0f;
            }
            if (page.h < 0.0f) {
                  page.h = 0.0f;
            }
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &page);

            // Clip all page content to the viewport
            SDL_Rect clip = {(int)page.x, (int)page.y, (int)page.w, (int)page.h};
            SDL_SetRenderClipRect(renderer, &clip);

            // Layout only on content/resize; paint replays cached lines
            if (page_dirty) {
                  Tess::Render::Layout(content, page_face, doc, page.x + 8, page.y + 8, page.w - 16);
                  page_dirty = false;
            }
            // Clamp scroll to content (8px pads top/bottom)
            {
                  float max_scroll = content.content_h + 16.0f - page.h;
                  if (max_scroll < 0.0f) {
                        max_scroll = 0.0f;
                  }
                  scroll_y = std::clamp(scroll_y, 0.0f, max_scroll);
            }
            Tess::Render::Paint(renderer, content, scroll_y);

            SDL_SetRenderClipRect(renderer, nullptr);

            // Display the rendered frame
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
      }

      Tess::Render::ClearPage(content);
      TTF_DestroyText(txt);
      page_face.Close(); // before TTF_Quit: stack dtor would run after it
      TTF_DestroyRendererTextEngine(eng);
      TTF_CloseFont(font);
      TTF_Quit();
      SDL_StopTextInput(window);
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();

      return 0;
}
