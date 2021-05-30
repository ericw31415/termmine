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
        set_mine(cells[i] / cols, cells[i] % cols);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j)
            set_mines_count(i, j);
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

const std::vector<std::vector<unsigned char>>& Game::board() const noexcept
{
    return board_;
}

bool Game::has_mine(const int row, const int col) const noexcept
{
    return board_[row][col] & (1u << 7);
}

void Game::set_mine(const int row, const int col) noexcept
{
    board_[row][col] |= (1u << 7);
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

void Game::set_mines_count(const int row, const int col) noexcept
{
    int num_mines = 0;
    for (auto& adj : adjacent_cells(row, col)) {
        if (has_mine(adj.first, adj.second))
            ++num_mines;
    }
    board_[row][col] |= num_mines;
}
}
