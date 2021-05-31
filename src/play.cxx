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
enum Color {
    color_unopened = 1,
    color_flagged,
    color_opened,
    color_mine
};

void define_colors()
{
    init_pair(color_unopened, COLOR_BLACK, COLOR_WHITE);
    init_pair(color_flagged, COLOR_RED, COLOR_WHITE);
    init_pair(color_opened, COLOR_WHITE, COLOR_BLACK);
    init_pair(color_mine, COLOR_BLACK, COLOR_RED);

    init_pair(color_unopened + 10, COLOR_BLACK, COLOR_YELLOW);
    init_pair(color_flagged + 10, COLOR_RED, COLOR_YELLOW);
    init_pair(color_opened + 10, COLOR_WHITE, COLOR_YELLOW);
}

/*
* Encode grid position to simplify draw_board() conditionals.
* Determines if pos is on the board edge, intersection, or other gridline.
*
* pos is a horizontal or vertical position.
* max is the width or height of the board.
*/
unsigned char encode_grid_pos(const int pos, const int max) noexcept
{
    if (pos == 0)
        return 0b00u;
    if (pos == max * 2)
        return 0b01u;
    if (pos % 2 == 0)
        return 0b10u;
    return 0b11u;
}

chtype decode_grid_symbol(const unsigned char encoded) noexcept
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
    default:
        return ' ';
    }
}
}

void draw_board(WINDOW* const board, const Game& game) noexcept
{
    for (int i = 0; i < game.rows() * 2 + 1; ++i) {
        for (int j = 0; j < game.cols() * 2 + 1; ++j) {
            // Encode each position into 4 bits to simplify check
            const unsigned char encoded = (encode_grid_pos(i, game.rows()) << 2)
                | encode_grid_pos(j, game.cols());

            mvwaddch(board, i, j, decode_grid_symbol(encoded));
        }
    }
}

void update_board(WINDOW* const board, const Game& game) noexcept
{
    for (int i = 0; i < game.rows(); ++i) {
        for (int j = 0; j < game.cols(); ++j) {
            if (game.is_open(i, j)) {
                wmove(board, i * 2 + 1, j * 2 + 1);
                if (game.has_mine(i, j)) {
                    wattron(board, COLOR_PAIR(color_mine));
                    waddch(board, '@');
                    wattroff(board, COLOR_PAIR(color_mine));
                } else {
                    wattron(board, COLOR_PAIR(color_opened));
                    const int adj_mines = game.num_adj_mines(i, j);
                    waddch(board, adj_mines > 0 ? '0' + adj_mines : ' ');
                    wattroff(board, COLOR_PAIR(color_opened));
                }
            } else if (game.has_flag(i, j)) {
                wattron(board, COLOR_PAIR(color_flagged));
                mvwaddch(board, i * 2 + 1, j * 2 + 1, 'P');
                wattroff(board, COLOR_PAIR(color_flagged));
            } else if (game.has_mark(i, j)) {
                wattron(board, COLOR_PAIR(color_unopened));
                mvwaddch(board, i * 2 + 1, j * 2 + 1, '?');
                wattroff(board, COLOR_PAIR(color_unopened));
            } else {
                wattron(board, COLOR_PAIR(color_unopened));
                mvwaddch(board, i * 2 + 1, j * 2 + 1, ' ');
                wattroff(board, COLOR_PAIR(color_unopened));
            }
        }
    }

#ifdef NDEBUG
    move(game.rows() * 2 + 4, 0);
    for (auto& row : game.board()) {
        for (auto col : row)
            printw("%02x ", col);
        addch('\n');
    }
#endif
}

void draw_cursor(WINDOW* const board, const Cursor& cursor) noexcept
{
    wmove(board, cursor.y * 2 + 1, cursor.x * 2 + 1);
    const chtype attrs = winch(board);
    wchgat(board, 1, attrs, PAIR_NUMBER(attrs & A_COLOR) + 10, nullptr);
}

void start_game()
{
    clear();
    define_colors();
    printw("Mines remaining:\n");
    printw("Time:\n");
    refresh();

    termmine::Game game{10, 15, 25};
    WINDOW* const board = newwin(game.rows() * 2 + 1, game.cols() * 2 + 1, 3,
                                 0);

    draw_board(board, game);
    wrefresh(board);

    Cursor cursor{0, 0};
    while (true) {
        update_board(board, game);
        draw_cursor(board, cursor);
        wrefresh(board);

        int c = getch();
        switch (c) {
        case KEY_LEFT:
            if (cursor.x > 0)
                --cursor.x;
            break;
        case KEY_RIGHT:
            if (cursor.x < game.cols() - 1)
                ++cursor.x;
            break;
        case KEY_UP:
            if (cursor.y > 0)
                --cursor.y;
            break;
        case KEY_DOWN:
            if (cursor.y < game.rows() - 1)
                ++cursor.y;
            break;

        case ' ':
            game.open_cell(cursor.y, cursor.x);
            break;
        case '1':
            game.flag_cell(cursor.y, cursor.x);
            break;
        case '2':
            game.mark_cell(cursor.y, cursor.x);
            break;

        case 'q':
            return;
        }
    }
}
}
