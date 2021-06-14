/*
* MIT License
*
* Copyright (c) 2021 Eric Wan
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef TERMMINE_PLAY_HXX
#define TERMMINE_PLAY_HXX

#include <cstddef>
#include <cstdint>

#include <optional>
#include <sstream>
#include <string>

#include <ncurses.h>

#include "Game.hxx"

namespace termmine {
struct Cursor {
    int x;
    int y;
};

constexpr int ctrl(int c) noexcept;

void define_colors() noexcept;

void update_time(const Game& game) noexcept;
void draw_board(WINDOW* board, const Game& game) noexcept;
void update_board(WINDOW* board, const Game& game) noexcept;
void draw_cursor(WINDOW* board, Cursor cursor) noexcept;

void new_game(int rows, int cols, int mines,
              std::optional<std::uint_fast64_t> seed);

// Handles leaving or playing again
void game_menu(int rows, int cols, int mines,
               std::optional<std::uint_fast64_t> seed = std::nullopt);

template <typename T, typename Val>
std::optional<T> get_valid_num(int prompt_len, Val&& validate) noexcept;
void create_custom_board();

// Handles selection of main menu options
void main_menu_select(int& option, int num_options) noexcept;
void main_menu() noexcept;

template <typename T, typename Val>
std::optional<T> get_valid_num(const int prompt_len, Val&& validate) noexcept
{
    curs_set(1);
    keypad(stdscr, false);

    std::optional<T> num;
    bool valid = false;
    int ypos = getcury(stdscr);

    do {
        clrtoeol();

        std::string input;
        int i = 0;
        int c{};
        while ((c = getch()) != '\n') {
            if (c != ctrl('h') && c != 127) {
                addch(c);
                input.push_back(c);
                ++i;
            } else if (i > 0) {
                // Backspace pressed and characters available to delete
                --i;
                move(ypos, prompt_len + i);
                delch();
                input.pop_back();
            }
        }
        addch('\n');

        std::istringstream iss{input};
        if (validate(iss, num)) {
            valid = true;
        } else {
            printw("Invalid input. Try again.");

            // Move to previous line
            move(ypos, prompt_len);
        }
    } while (!valid);

    keypad(stdscr, true);
    curs_set(0);
    return num;
}
}

#endif
