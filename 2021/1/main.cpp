#include "../../solver.h"

#include <deque>
#include <numeric>

namespace Year2021::Day1 {
    using Base = ::Common::Solver<size_t>;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            struct {
                size_t single_increases;
                size_t sliding_window_of_3_increases;
            } result_tracker{0, 0};

            unsigned long last = 0;
            unsigned long current = 0;

            auto last_two_window_numbers = std::deque<unsigned long>{};

            size_t i = 0;
            while (++i, input >> current) {
                // Part 1
                if (i > 1 && current > last) {
                    ++result_tracker.single_increases;
                }
                last = current;

                // Part 2
                last_two_window_numbers.emplace_back(current);

                // Do we have enough to have 2 sliding windows?
                if (last_two_window_numbers.size() == 4) {
                    // If sum of last 3 greater than sum of first 3?
                    if (std::accumulate(last_two_window_numbers.begin() + 1, last_two_window_numbers.end(), 0) >
                        std::accumulate(last_two_window_numbers.begin(), last_two_window_numbers.begin() + 3, 0)) {
                        ++result_tracker.sliding_window_of_3_increases;
                    }

                    // Always just keep the queue at length 3 or under.
                    last_two_window_numbers.pop_front();
                }
            }

            return Base::Answers{
                Base::Answer{"Number of measurements larger than previous", result_tracker.single_increases},
                Base::Answer{"Number of 3-size sliding window measurements larger than previous",
                             result_tracker.sliding_window_of_3_increases}};
        }
    };
} // namespace Year2021::Day1

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day1::Solver{FILENAME}.print_answers();
    return 0;
}
