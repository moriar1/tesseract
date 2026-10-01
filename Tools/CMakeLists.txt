##
## Tesseract - A simple browser engine.
## https://github.com/tanvir-techbro/tesseract
##
## Copyright (c) 2026-present tanvir-techbro (Tanvir)
## Released under the MIT License.
##

# Developer tools (lexdump, treedump, ...). Built with the main project.

# MinGW std::print workaround (see Src/WinPrint.cpp).
if(WIN32)
      set(WINPRINT_SRC ../Src/WinPrint.cpp)
else()
      set(WINPRINT_SRC "")
endif()

add_executable(lexdump
    Debug/LexDump.cpp
    ../Libraries/HtmlKit/src/Tokenize.cpp
    ${WINPRINT_SRC}
)

set_target_properties(lexdump PROPERTIES OUTPUT_NAME LexDump)

target_include_directories(lexdump PRIVATE
    ../Libraries
)

add_executable(treedump
    Debug/TreeDump.cpp
    ../Libraries/HtmlKit/src/Tokenize.cpp
    ../Libraries/HtmlKit/src/Parse.cpp
    ${WINPRINT_SRC}
)

set_target_properties(treedump PROPERTIES OUTPUT_NAME TreeDump)

target_include_directories(treedump PRIVATE
    ../Libraries
)

add_executable(urldump
    Debug/UrlDump.cpp
    ../Libraries/NetKit/src/Url.cpp
    ${WINPRINT_SRC}
)

set_target_properties(urldump PROPERTIES OUTPUT_NAME UrlDump)

target_include_directories(urldump PRIVATE
    ../Libraries
)
