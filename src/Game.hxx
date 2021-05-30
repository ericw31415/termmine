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

#ifndef GAME_HXX
#define GAME_HXX

#include <utility>
#include <vector>

namespace termmine {
class Game final {
public:
    Game(int rows, int cols, int mines) noexcept;

    int rows() const noexcept;
    int cols() const noexcept;
    const std::vector<std::vector<unsigned char>>& board() const noexcept;

    bool has_mine(int row, int col) const noexcept;

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
    * If the cell is uncertain - 1 bit
    * Number of adjacent mines - 4 bits
    */
    std::vector<std::vector<unsigned char>> board_;

    void set_mine(int row, int col) noexcept;
    std::vector<std::pair<int, int>> adjacent_cells(int row, int col)
        const noexcept;
    void set_mines_count(int row, int col) noexcept;
};
}

#endif
