#include "../../solver.h"

#include <deque>
#include <unordered_set>

namespace Year2021::Day11 {
    using Base = ::Common::Solver<long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class OctopusGrid {
      public:
        OctopusGrid(std::ifstream &input) {
            std::string s;
            if (!(input >> s)) {
                throw std::runtime_error{"malformed_input"};
            }

            // Assuming all rows are same size
            w = s.size();
            parse_row(s);

            auto rows = 1;
            while (input >> s) {
                parse_row(s);
                ++rows;
            }
            h = rows;
        }

        void print() {
            for (size_t y = 0; y < height(); ++y) {
                for (size_t x = 0; x < width(); ++x) {
                    std::cout << (unsigned)at(x, y) << ",";
                }
                std::cout << std::endl;
            }
        }

        size_t width() const { return w; }
        size_t height() const { return h; }
        uint8_t at(size_t x, size_t y) const { return grid[flat_array_idx(x, y)]; }
        uint8_t at(size_t flat_array_index) const { return grid[flat_array_index]; }
        uint8_t set(size_t x, size_t y, uint8_t value) { return grid[flat_array_idx(x, y)] = value; }
        uint8_t set(size_t flat_array_index, uint8_t value) { return grid[flat_array_index] = value; }
        size_t flat_array_idx(size_t x, size_t y) const { return x + y * w; }

      private:
        size_t w;
        size_t h;
        std::vector<uint8_t> grid;

        void parse_row(const std::string &s) {
            for (auto c : s) {
                grid.emplace_back(c - '0');
            }
        }
    };

    class Solver : public Base {
        using Base::Solver;

        static const uint8_t FLASH_ENERGY_LEVEL = 9;
        static const uint8_t RESET_ENERGY_LEVEL = 0;

        Answers solve(std::ifstream &input) const override {
            auto grid = OctopusGrid{input};
            const auto cell_count = grid.width() * grid.height();

            auto total_flashes_after_step_100 = (long long){-1};
            auto first_simultaneous_flash_step = (long long){-1};

            auto total_flashes = size_t{0};
            for (size_t step = 1; total_flashes_after_step_100 == -1 || first_simultaneous_flash_step == -1; ++step) {
                const auto flashes = account_for_flashes(grid);
                total_flashes += flashes;

                if (flashes == cell_count && first_simultaneous_flash_step == -1) {
                    first_simultaneous_flash_step = step;
                }

                if (step == 100 && total_flashes_after_step_100 == -1) {
                    total_flashes_after_step_100 = total_flashes;
                }
            }

            return Answers{Answer{"Total flashes are after 100 steps", total_flashes_after_step_100},
                           Answer{"First step during which all octopuses flash", first_simultaneous_flash_step}};
        };

      private:
        // Returns number of flashes this step
        static size_t account_for_flashes(OctopusGrid &grid) {
            auto to_flash = std::deque<std::pair<size_t, size_t>>{};
            auto flashed = std::vector<bool>(grid.width() * grid.height(), false);

            auto increment_cell = [&](size_t x, size_t y) {
                if (x >= grid.width() || y >= grid.height()) {
                    return;
                }

                const auto unique_idx = grid.flat_array_idx(x, y);
                const auto new_value = grid.at(unique_idx) + 1;
                if (new_value > FLASH_ENERGY_LEVEL && !flashed[unique_idx]) {
                    to_flash.emplace_back(x, y);
                    flashed[unique_idx] = true;
                }
                grid.set(unique_idx, new_value);
            };

            // Increment all cells
            for (size_t y = 0; y < grid.height(); ++y) {
                for (size_t x = 0; x < grid.width(); ++x) {
                    increment_cell(x, y);
                }
            }

            // Account for flashes that make increment adjacent neighbors
            while (!to_flash.empty()) {
                const auto &point = to_flash.front();
                const auto px = point.first;
                const auto py = point.second;

                if (px > 0) {
                    increment_cell(px - 1, py + 1); // SW
                    increment_cell(px - 1, py);     // W
                }

                if (py > 0) {
                    increment_cell(px + 1, py - 1); // NE
                    increment_cell(px, py - 1);     // N
                    if (px > 0) {
                        increment_cell(px - 1, py - 1); // NW
                    }
                }

                increment_cell(px + 1, py);     // E
                increment_cell(px + 1, py + 1); // SE
                increment_cell(px, py + 1);     // S

                to_flash.pop_front();
            }

            auto flash_count = size_t{0};
            // Set everything that flashed back to energy 0
            for (size_t i = 0; i < flashed.size(); ++i) {
                if (flashed[i]) {
                    ++flash_count;
                    grid.set(i, 0);
                }
            }

            return flash_count;
        }
    };
} // namespace Year2021::Day11

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day11::Solver{FILENAME}.print_answers();
    return 0;
}
