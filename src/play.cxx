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

#include <ncurses.h>
#include "Game.hxx"
#include "play.hxx"

namespace termmine {
namespace {
/*
* Encode grid position to simplify draw_board() conditionals.
* Determines if pos is on the board edge, intersection, or other gridline.
*
* pos is a horizontal or vertical position.
* max is the width or height of the board.
*/
unsigned encode_grid_pos(const int pos, const int max) noexcept
{
    if (pos == 0)
        return 0b00u;
    if (pos == max * 2)
        return 0b01u;
    if (pos % 2 == 0)
        return 0b10u;
    return 0b11u;
}

chtype decode_grid_symbol(const unsigned encoded)
{
    switch (encoded) {
    case 0b0000u:
        return ACS_ULCORNER;
    case 0b0001u:
        return ACS_URCORNER;
    case 0b0100u:
        return ACS_LLCORNER;
    case 0b0101u:
        return ACS_LRCORNER;

    case 0b0010u:
        return ACS_TTEE;
    case 0b1000u:
        return ACS_LTEE;
    case 0b1001u:
        return ACS_RTEE;
    case 0b0110u:
        return ACS_BTEE;

    case 0b0011u:
    case 0b0111u:
    case 0b1011u:
        return ACS_HLINE;
    case 0b1100u:
    case 0b1101u:
    case 0b1110u:
        return ACS_VLINE;

    case 0b1010u:
        return ACS_PLUS;
    case 0b1111u:
        return ' ';
    }
}
/*
void update_board(WINDOW* board, int x, int y)
{
    for (int i = 0; i < board_width * 2 + 1; ++i) {
        for (int j = 0; j < board_height * 2 + 1; ++j) {
            mvwchgat(board, y, x, 1, A_REVERSE, 0, NULL);
            if (i % 2 == 0 && j % 2 == 0) {
                if (i == x && j == y)
                    mvwaddch(board, j, i, ACS_PLUS | A_REVERSE);
                else
                    mvwaddch(board, j, i, ACS_PLUS);
            }
        }
    }
    wrefresh(board);
}*/
}

void draw_board(WINDOW* const board, const Game& game)
{
    for (int i = 0; i < game.rows() * 2 + 1; ++i) {
        for (int j = 0; j < game.cols() * 2 + 1; ++j) {
            // Encode each position into 4 bits to simplify check
            const unsigned encoded = (encode_grid_pos(i, game.rows()) << 2)
                | encode_grid_pos(j, game.cols());

            mvwaddch(board, i, j, decode_grid_symbol(encoded));
        }
    }
}

void update_board(WINDOW* const board, const Game& game, const Cursor& cursor)
{}

void start_game()
{
    clear();
    printw("Mines remaining:\n");
    printw("Time:\n");
    refresh();

    const termmine::Game game{10, 15};
    WINDOW* const board = newwin(game.rows() * 2 + 1, game.cols() * 2 + 1, 3,
                                 0);
    draw_board(board, game);
    // init_pair(1, COLOR_BLACK, COLOR_RED);
    // wbkgd(board, ACS_PLUS | COLOR_PAIR(1));
    wrefresh(board);

    int x = 0;
    int y = 0;

    while (true) {
        // update_board(board, x, y);
        int c = getch();

        switch (c) {
        case KEY_LEFT:
            --x;
            break;
        case KEY_RIGHT:
            ++x;
            break;
        case KEY_UP:
            --y;
            break;
        case KEY_DOWN:
            ++y;
            break;
        case 'q':
            return;
        }
    }
}
}
