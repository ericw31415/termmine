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

#ifndef TERMMINE_GAME_HXX
#define TERMMINE_GAME_HXX

#include <chrono>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Timer.hxx"

namespace termmine {
class Game final {
public:
    Game(int rows, int cols, int mines) noexcept;

    int rows() const noexcept;
    int cols() const noexcept;
    int mines() const noexcept;
    const std::vector<std::vector<unsigned char>>& board() const noexcept;
    std::chrono::milliseconds::rep get_time() const noexcept;

    bool is_over() const noexcept;
    bool has_won() const noexcept;
    int flags() const noexcept;

    // Pass in coordinates of just-opened cell
    void check_win(int row, int col) noexcept;

    bool has_mine(int row, int col) const noexcept;
    bool is_open(int row, int col) const noexcept;
    bool has_flag(int row, int col) const noexcept;
    bool has_mark(int row, int col) const noexcept;
    int num_adj_mines(int row, int col) const noexcept;

    void open_cell(int row, int col);
    void flag_cell(int row, int col) noexcept;
    void mark_cell(int row, int col) noexcept;

private:
    const int rows_;
    const int cols_;

    const int mines_;

    /*
    * Uses bit packing to store each cell's information. Each bit represents,
    * from MSB to LSB:
    *
    * If the cell has a mine - 1 bit
    * If the cell is opened - 1 bit
    * If the cell is flagged - 1 bit
    * If the cell is marked - 1 bit
    * Number of adjacent mines - 4 bits
    */
    std::vector<std::vector<unsigned char>> board_;
    Timer timer_;

    bool game_over_ = false;
    bool won_ = false;
    int cells_flagged_ = 0;
    int open_cells_ = 0;

    void toggle_mine(int row, int col) noexcept;
    std::vector<std::pair<int, int>> adjacent_cells(int row, int col)
        const noexcept;
    void set_adj_mines_count(int row, int col) noexcept;
    std::pair<int, int> first_open_cell() const;
};

class BadGameState final : public std::logic_error {
public:
    BadGameState(const char* what) : logic_error{what} {}
};
}

#endif
