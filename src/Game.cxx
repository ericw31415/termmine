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

#include "Game.hxx"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

namespace termmine {
Game::Game(const int rows, const int cols, const int mines) noexcept
    : rows_{rows},
      cols_{cols},
      mines_{mines},
      board_(rows, std::vector<unsigned char>(cols, 0))
{
    // Assign a number to each cell and randomize mine placement
    std::vector<int> cells;
    cells.reserve(rows * cols);
    for (int i = 0; i < rows * cols; ++i)
        cells.push_back(i);

    std::mt19937 gen{std::random_device{}()};
    std::ranges::shuffle(cells, gen);
    for (int i = 0; i < mines; ++i)
        toggle_mine(cells[i] / cols, cells[i] % cols);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j)
            set_adj_mines_count(i, j);
    }
}

int Game::rows() const noexcept
{
    return rows_;
}

int Game::cols() const noexcept
{
    return cols_;
}

int Game::mines() const noexcept
{
    return mines_;
}

const std::vector<std::vector<unsigned char>>& Game::board() const noexcept
{
    return board_;
}

std::chrono::milliseconds::rep Game::get_time() const noexcept
{
    return timer_.elapsed();
}

bool Game::is_over() const noexcept
{
    return game_over_;
}

bool Game::has_won() const noexcept
{
    return won_;
}

int Game::flags() const noexcept
{
    return cells_flagged_;
}

void Game::check_win(const int row, const int col) noexcept
{
    if (open_cells_ + mines_ == rows_ * cols_ && !has_mine(row, col)) {
        won_ = true;
        game_over_ = true;

        // autoflag all unflagged cells
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                if (has_mine(i, j) && !has_flag(i, j))
                    flag_cell(i, j);
            }
        }
    }
}

bool Game::has_mine(const int row, const int col) const noexcept
{
    return board_[row][col] & (1u << 7);
}

bool Game::is_open(const int row, const int col) const noexcept
{
    return board_[row][col] & (1u << 6);
}

bool Game::has_flag(const int row, const int col) const noexcept
{
    return board_[row][col] & (1u << 5);
}

bool Game::has_mark(const int row, const int col) const noexcept
{
    return board_[row][col] & (1u << 4);
}

int Game::num_adj_mines(const int row, const int col) const noexcept
{
    return board_[row][col] & 0b1111u;
}

void Game::open_cell(const int row, const int col)
{
    if (is_open(row, col) || has_flag(row, col) || has_mark(row,col))
        return;

    if (open_cells_ == 0)
        timer_.start();

    board_[row][col] |= 1u << 6; // set opened flag
    ++open_cells_;
    if (has_mine(row, col)) {
        if (open_cells_ == 1) {
            // Prevent a first-move loss
            std::pair <int, int> open_cell = first_open_cell();
            toggle_mine(open_cell.first, open_cell.second);
            toggle_mine(row, col);
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j)
                    set_adj_mines_count(i, j);
            }
        } else {
            game_over_ = true;
            return;
        }
    }

    if (num_adj_mines(row, col) == 0) {
        for (const auto& adj : adjacent_cells(row, col))
            open_cell(adj.first, adj.second);
    }
}

void Game::chord_cell(const int row, const int col)
{
    if (!is_open(row, col))
        return;

    int flags = 0;
    for (const auto& adj : adjacent_cells(row, col))
        flags += has_flag(adj.first, adj.second);
    if (flags != num_adj_mines(row, col))
        return;

    for (const auto& adj : adjacent_cells(row, col))
        open_cell(adj.first, adj.second);
}

void Game::flag_cell(const int row, const int col) noexcept
{
    if (is_open(row, col))
        return;

    board_[row][col] &= ~(1u << 4); // unmark cell first
    board_[row][col] ^= 1u << 5;

    if ((board_[row][col] & (1u << 5)) == 1u << 5)
        ++cells_flagged_;
    else
        --cells_flagged_;
}

void Game::mark_cell(const int row, const int col) noexcept
{
    board_[row][col] &= ~(1u << 5); // unflag cell first
    board_[row][col] ^= 1u << 4;
}

void Game::toggle_mine(const int row, const int col) noexcept
{
    board_[row][col] ^= 1u << 7;
}

std::vector<std::pair<int, int>> Game::adjacent_cells(
    const int row, const int col) const noexcept
{
    std::vector<std::pair<int, int>> adj;

    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (i >= 0 && i < rows_ && j >= 0 && j < cols_
                && (i != row || j != col))
            adj.push_back(std::make_pair(i, j));
        }
    }
    return adj;
}

void Game::set_adj_mines_count(const int row, const int col) noexcept
{
    int num_mines = 0;
    for (const auto& adj : adjacent_cells(row, col)) {
        if (has_mine(adj.first, adj.second))
            ++num_mines;
    }
    board_[row][col] &= ~(0b1111u);
    board_[row][col] |= num_mines;
}

std::pair<int, int> Game::first_open_cell() const
{
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < rows_; ++j) {
            if (!has_mine(i, j))
                return std::make_pair(i, j);
        }
    }
    throw BadGameState("No safe cells present in board");
}
}
