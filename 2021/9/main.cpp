#include "../../solver.h"

#include <algorithm>
#include <deque>
#include <numeric>
#include <vector>

namespace Year2021::Day9 {
    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

      protected:
        class HeightMapGrid {
          public:
            HeightMapGrid(std::ifstream &input) {
                std::string s;
                if (!(input >> s)) {
                    throw Error{"malformed_input"};
                }

                // Assuming all rows are same size
                w = s.size() + 2;

                // Add a dummy max height row at top
                for (size_t i = 0; i < w; ++i) {
                    height_map.emplace_back(9);
                }
                parse_row(s);

                size_t rows = 2;
                while (input >> s) {
                    parse_row(s);
                    ++rows;
                }

                // Add a dummy max height row at top
                for (size_t i = 0; i < w; ++i) {
                    height_map.emplace_back(9);
                }
                h = rows + 1;
            }

            size_t width() const { return w; }
            size_t height() const { return h; }
            uint8_t at(size_t x, size_t y) const { return height_map[unique_idx(x, y)]; }
            size_t unique_idx(size_t x, size_t y) const { return x + y * w; }

          private:
            size_t w;
            size_t h;
            std::vector<uint8_t> height_map;

            void parse_row(const std::string &s) {
                height_map.emplace_back(9); // add 9 as first column
                for (auto c : s) {
                    height_map.emplace_back(c - '0');
                }
                height_map.emplace_back(9); // add 9 as last column
            }
        };

        Answers solve(std::ifstream &input) const override {
            const auto grid = HeightMapGrid(input);

            auto sum_of_low_point_risk = (unsigned long long){0};
            auto basin_sizes = std::vector<size_t>{};

            for (size_t x = 1; x < grid.width() - 1; ++x) {
                for (size_t y = 1; y < grid.height() - 1; ++y) {
                    const auto value = grid.at(x, y);
                    if (value < grid.at(x - 1, y) && value < grid.at(x + 1, y) && value < grid.at(x, y - 1) &&
                        value < grid.at(x, y + 1)) {
                        // Found a low point
                        sum_of_low_point_risk += 1 + value;
                        basin_sizes.emplace_back(basin_size_for_low_point(grid, x, y));
                    }
                }
            }

            if (basin_sizes.size() < 3) {
                throw Error{"not_enough_basins"};
            }

            std::sort(basin_sizes.begin(), basin_sizes.end());

            return Answers{Answer{"Sum of the risk levels of all low points", sum_of_low_point_risk},
                           Answer{"Product of 3 largest basin sizes",
                                  std::accumulate(basin_sizes.rbegin(),
                                                  basin_sizes.rbegin() + 3,
                                                  (unsigned long long)1,
                                                  std::multiplies<unsigned long long>())}};
        }

      private:
        static size_t basin_size_for_low_point(const HeightMapGrid &grid, size_t x, size_t y) {
            auto basin_size = size_t{};
            auto visited = std::vector<bool>(grid.width() * grid.height(), false);
            auto to_visit = std::deque<std::pair<size_t, size_t>>{{x, y}};

            auto add_to_queue = [&](uint8_t value, size_t other_x, size_t other_y) {
                const auto other_value = grid.at(other_x, other_y);
                if (value < other_value && other_value != 9) {
                    to_visit.emplace_back(other_x, other_y);
                }
            };

            while (!to_visit.empty()) {
                const auto &point = to_visit.front();
                const auto px = point.first;
                const auto py = point.second;
                const auto value = grid.at(px, py);

                if (!visited[grid.unique_idx(px, py)]) {
                    add_to_queue(value, px - 1, py); // left
                    add_to_queue(value, px + 1, py); // right
                    add_to_queue(value, px, py - 1); // above
                    add_to_queue(value, px, py + 1); // below

                    visited[grid.unique_idx(px, py)] = true;
                    ++basin_size;
                    continue;
                }

                to_visit.pop_front();
            }

            return basin_size;
        }
    };
} // namespace Year2021::Day9

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day9::Solver{FILENAME}.print_answers();
    return 0;
}
