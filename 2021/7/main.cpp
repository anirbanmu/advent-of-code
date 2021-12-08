#include "../../solver.h"
#include "../../utils.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace Year2021::Day7 {
    using Base = ::Common::Solver<long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Answers solve(std::ifstream &input) const override {
            const auto positions = read_numbers(input);

            return Answers{Answer{"Least fuel required to align with constant fuel usage",
                                  least_constant_fuel_to_align(positions)},
                           Answer{"Least fuel required to align with linearly increasing fuel usage",
                                  least_linear_fuel_to_align(positions)}};
        }

      private:
        static long long least_constant_fuel_to_align(const std::vector<long long> &positions) {
            auto positions_sortable = positions;

            // Sort enough to find median
            std::nth_element(positions_sortable.begin(),
                             positions_sortable.begin() + (positions_sortable.size() / 2),
                             positions_sortable.end());
            const auto median = positions_sortable[positions_sortable.size() / 2];

            return std::accumulate(positions_sortable.begin(),
                                   positions_sortable.end(),
                                   (long long)0,
                                   [=](auto acc, auto p) { return acc + std::abs(median - p); });
        }

        static long long least_linear_fuel_to_align(const std::vector<long long> &positions) {
            const auto mean =
                std::accumulate(positions.begin(), positions.end(), (long long)0) / (long double)positions.size();
            const auto mean_floor = static_cast<long long>(std::floor(mean));
            const auto mean_ceil = static_cast<long long>(std::ceil(mean));

            return std::min(std::accumulate(positions.begin(),
                                            positions.end(),
                                            (long long)0,
                                            [=](auto acc, auto p) {
                                                return acc + linear_fuel_usage_for_steps(std::abs(mean_floor - p));
                                            }),
                            std::accumulate(positions.begin(), positions.end(), (long long)0, [=](auto acc, auto p) {
                                return acc + linear_fuel_usage_for_steps(std::abs(mean_ceil - p));
                            }));
        }

        static long long linear_fuel_usage_for_steps(long long steps) { return steps * (steps + 1) / 2; }

        static std::vector<long long> read_numbers(std::ifstream &input) {
            auto numbers = std::vector<long long>{};

            std::string str;
            if (!(input >> str)) {
                throw Error{"malformed_input"};
            }

            const auto str_view = std::string_view{str};

            size_t pos = 0;
            size_t delim_pos = 0;
            while ((delim_pos = str_view.find(',', pos)) != std::string::npos) {
                numbers.emplace_back(Utils::str_to_int<long long>(str_view.substr(pos, delim_pos - pos)));
                pos = delim_pos + 1;
            }
            numbers.emplace_back(Utils::str_to_int<long long>(str_view.substr(pos, delim_pos - pos)));

            return numbers;
        }
    };
} // namespace Year2021::Day7

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day7::Solver{FILENAME}.print_answers();
    return 0;
}
