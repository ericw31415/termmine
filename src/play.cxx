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

#include "play.hxx"

#include <cinttypes>

#include <array>
#include <exception>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <ncurses.h>

#include "Game.hxx"

namespace termmine {
namespace {
enum Color {
    color_unopened = 1,
    color_flagged,
    color_opened,
    color_mine,
    color_mine_wrong,

    color_one,
    color_two,
    color_three,
    color_four,
    color_five,
    color_six,
    color_seven,
    color_eight
};

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

constexpr int ctrl(const int c) noexcept
{
    return c & 0x1f;
}

void define_colors() noexcept
{
    init_pair(color_unopened, COLOR_BLACK, COLOR_WHITE);
    init_pair(color_flagged, COLOR_RED, COLOR_WHITE);
    init_pair(color_opened, COLOR_WHITE, COLOR_BLACK);
    init_pair(color_mine, COLOR_BLACK, COLOR_RED);
    init_pair(color_mine_wrong, COLOR_RED, COLOR_BLACK);

    // Cell number colors
    init_pair(color_one, COLOR_BLUE, COLOR_BLACK);
    init_pair(color_two, COLOR_GREEN, COLOR_BLACK);
    init_pair(color_three, COLOR_RED, COLOR_BLACK);
    init_pair(color_four, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(color_five, COLOR_RED, COLOR_BLACK);
    init_pair(color_six, COLOR_CYAN, COLOR_BLACK);
    init_pair(color_seven, COLOR_WHITE, COLOR_BLACK);
    init_pair(color_eight, COLOR_WHITE, COLOR_BLACK);

    init_pair(color_unopened + 20, COLOR_BLACK, COLOR_YELLOW);
    init_pair(color_flagged + 20, COLOR_RED, COLOR_YELLOW);
    init_pair(color_opened + 20, COLOR_WHITE, COLOR_YELLOW);

    init_pair(color_one + 20, COLOR_BLUE, COLOR_YELLOW);
    init_pair(color_two + 20, COLOR_GREEN, COLOR_YELLOW);
    init_pair(color_three + 20, COLOR_RED, COLOR_YELLOW);
    init_pair(color_four + 20, COLOR_MAGENTA, COLOR_YELLOW);
    init_pair(color_five + 20, COLOR_RED, COLOR_YELLOW);
    init_pair(color_six + 20, COLOR_CYAN, COLOR_YELLOW);
    init_pair(color_seven + 20, COLOR_WHITE, COLOR_YELLOW);
    init_pair(color_eight + 20, COLOR_WHITE, COLOR_YELLOW);
}

void update_time(const Game& game) noexcept
{
    std::ostringstream oss;
    auto time = game.get_time();
    oss.fill('0');
    if (time >= 60000)
        oss << time / 60000 << ':' << std::setw(2);
    oss << time % 60000 / 1000 << '.' << std::setw(3) << time % 1000;
    move(1, 6);
    clrtoeol();
    printw("%s", oss.str().c_str());
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
    move(0, 17);
    clrtoeol();
    printw("%d", game.mines() - game.flags());
    for (int i = 0; i < game.rows(); ++i) {
        for (int j = 0; j < game.cols(); ++j) {
            if (game.is_open(i, j)) {
                wmove(board, i * 2 + 1, j * 2 + 1);
                if (game.has_mine(i, j)) {
                    wattron(board, COLOR_PAIR(color_mine));
                    waddch(board, '@');
                    wattroff(board, COLOR_PAIR(color_mine));
                } else {
                    const int adj_mines = game.num_adj_mines(i, j);
                    chtype color = COLOR_PAIR(adj_mines > 0
                        ? adj_mines + color_one - 1 : color_opened);

                    wattron(board, color);
                    waddch(board, adj_mines > 0 ? '0' + adj_mines : ' ');
                    wattroff(board, color);
                }
            } else if (game.is_over() && !game.has_won()
                && game.has_mine(i, j)) {
                wattron(board, COLOR_PAIR(color_opened));
                mvwaddch(board, i * 2 + 1, j * 2 + 1, '@');
                wattroff(board, COLOR_PAIR(color_opened));
            } else if (game.has_flag(i, j)) {
                if (game.is_over() && !game.has_mine(i, j)) {
                    wattron(board, COLOR_PAIR(color_mine_wrong));
                    mvwaddch(board, i * 2 + 1, j * 2 + 1, 'X');
                    wattroff(board, COLOR_PAIR(color_mine_wrong));
                } else {
                    wattron(board, COLOR_PAIR(color_flagged));
                    mvwaddch(board, i * 2 + 1, j * 2 + 1, 'P');
                    wattroff(board, COLOR_PAIR(color_flagged));
                }
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
    for (int i = 0; const auto& row : game.board()) {
        move(i + 5, game.cols() * 2 + 3);
        for (const auto col : row)
            printw("%02x ", col);
        addch('\n');
        ++i;
    }
#endif
}

void draw_cursor(WINDOW* const board, const Cursor cursor) noexcept
{
    wmove(board, cursor.y * 2 + 1, cursor.x * 2 + 1);
    const chtype attrs = winch(board);
    wchgat(board, 1, attrs, PAIR_NUMBER(attrs & A_COLOR) + 20, nullptr);
}

void show_seed(const Game& game) noexcept
{
    mvprintw(3, game.cols() * 2 + 3, "Seed: %" PRIuFAST64 "\n", game.seed());
}

void new_game(const int rows, const int cols, const int mines,
              const std::optional<std::uint_fast64_t> seed)
{
    clear();
    define_colors();
    refresh();
    printw("Mines remaining:\n");
    printw("Time:\n");

    Game game{seed ? Game{rows, cols, mines, *seed} : Game{rows, cols, mines}};
    WINDOW *const board = newwin(game.rows() * 2 + 1, game.cols() * 2 + 1,
                                 3, 0);

#ifdef NDEBUG
    show_seed(game);
    refresh();
#endif

    draw_board(board, game);
    wrefresh(board);

    Cursor cursor{0, 0};
    wattron(board, A_BOLD);
    while (!game.is_over()) {
        update_time(game);
        refresh();
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
            if (game.is_open(cursor.y, cursor.x))
                game.chord_cell(cursor.y, cursor.x);
            else
                game.open_cell(cursor.y, cursor.x);
            game.check_win(cursor.y, cursor.x);
            break;
        case '1':
            game.flag_cell(cursor.y, cursor.x);
            break;
        case '2':
            game.mark_cell(cursor.y, cursor.x);
            break;

        case ctrl('q'):
            show_seed(game);
            refresh();
            move(game.rows() * 2 + 4, 0);
            return;
        }
    }

    update_board(board, game);
    wrefresh(board);
    show_seed(game);
    move(game.rows() * 2 + 4, 0);
    if (game.has_won())
        printw("You swept through the minefield safely. You won!\n");
    else
        printw("You exploded. Game over.\n");
    refresh();
}

void game_menu(const int rows, const int cols, const int mines,
               const std::optional<std::uint_fast64_t> seed)
{
    while (true) {
        nodelay(stdscr, true);
        new_game(rows, cols, mines, seed);
        nodelay(stdscr, false);

        clrtoeol();
        printw("Play again? (y/n)\n");
        bool valid{};
        do {
            valid = true;
            int c = getch();
            switch (c) {
                case 'y':
                    break;
                case 'n':
                    return;
                default:
                    valid = false;
            }
        } while (!valid);
    }
}

void create_custom_board()
{
    const std::array<const std::string, 4> prompts{
        "Number of rows: ",
        "Number of columns: ",
        "Number of mines: ",
        "Seed (leave blank for random): "};
    auto size_validate = [](std::istringstream& iss, std::optional<int>& num)
        -> bool
        {
            int tmp{};
            iss >> tmp;
            num = tmp;
            return iss && iss.eof() && tmp > 0;
        };

    printw(prompts[0].c_str());
    auto rows = get_valid_num<int>(prompts[0].length(), size_validate);

    printw(prompts[1].c_str());
    auto cols = get_valid_num<int>(prompts[1].length(), size_validate);

    printw(prompts[2].c_str());
    auto mines = get_valid_num<int>(prompts[2].length(), size_validate);

    printw(prompts[3].c_str());
    auto seed = get_valid_num<std::uint_fast64_t>(
        prompts[3].length(),
        [](std::istringstream& iss, std::optional<std::uint_fast64_t>& num)
            -> bool
        {
            if (iss.peek() == '-') // prevent negative wrapping
                return false;
            if (iss.eof()) // allow blank seed
                return true;
            std::uint_fast64_t tmp{};
            iss >> tmp;
            num = tmp;
            return iss && iss.eof();
        });

    if (!rows || !cols || ! mines)
        throw BadGameState{"Cannot specify rows, cols, or mines as blank"};

    // Make sure at least one cell is safe
    if (mines >= *rows * *cols)
        mines = *rows * *cols - 1;

    game_menu(*rows, *cols, *mines, seed);
}

void main_menu_select(int& option, const int num_options) noexcept
{
    bool option_chosen = false;
    while (!option_chosen) {
        mvchgat(option + 2, 0, -1, A_REVERSE, 0, nullptr);

        int c = getch();
        mvchgat(option + 2, 0, -1, A_NORMAL, 0, nullptr);
        switch (c) {
        case KEY_UP:
            if (option == 0)
                option = num_options - 1;
            else
                --option;
            break;
        case KEY_DOWN:
            if (option == num_options - 1)
                option = 0;
            else
                ++option;
            break;
        case '\n':
            option_chosen = true;
        }
    }
}

void main_menu() noexcept
{
    constexpr std::array options{
        "Beginner\t9 x 9\t\t10 mines",
        "Intermediate\t16 x 16\t\t40 mines",
        "Advanced\t16 x 30\t\t99 mines",
        "Custom board",
        "Quit"
    };

    int option = 0; // remember chosen option after game ends
    bool quit = false;
    while (!quit) {
        clear();

        // Draw title and menu options
        attron(A_BOLD);
        printw("@ ");
        addch('T' | COLOR_PAIR(color_one));
        addch('e' | COLOR_PAIR(color_two));
        addch('r' | COLOR_PAIR(color_three));
        addch('m' | COLOR_PAIR(color_four));
        attron(COLOR_PAIR(color_flagged));
        printw("Mine");
        attroff(COLOR_PAIR(color_flagged));
        printw(" @\n\n");
        attroff(A_BOLD);

        for (const auto opt : options) {
            printw(opt);
            addch('\n');
        }

        // Option select
        main_menu_select(option, options.size());
        try {
            switch (option) {
            case 0:
                game_menu(9, 9, 10);
                break;
            case 1:
                game_menu(16, 16, 40);
                break;
            case 2:
                game_menu(16, 30, 99);
                break;
            case 3:
                move(options.size() + 3, 0);
                create_custom_board();
                break;
            default:
                return;
            }
        } catch (const BadGameState& err) {
            clear();
            printw("Error: %s\n", err.what());
            printw("Press any key to exit...");
            getch();
            return;
        }
    }
}
}
