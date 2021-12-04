#include "../../solver.h"

#include <bitset>
#include <functional>
#include <iterator>
#include <numeric>

namespace Year2021::Day3 {
    using Base = ::Common::Solver<unsigned long>;

    static const size_t MAX_BITS_COUNT = std::numeric_limits<unsigned long>::digits;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            const auto numbers = std::vector<std::string>{std::istream_iterator<std::string>(input),
                                                          std::istream_iterator<std::string>{}};

            const size_t bit_count =
                std::accumulate(numbers.begin(), numbers.end(), 0, [](size_t max, const std::string &s) {
                    return std::max(max, s.size());
                });

            if (bit_count > MAX_BITS_COUNT) {
                throw Error{"too_many_bits_in_input"};
            }

            const auto bit_frequency_indicator = calculate_bit_frequency_indicator(numbers);

            return Base::Answers{
                Base::Answer{"Power consumption (gamma x epsilon)", power_consumption(bit_frequency_indicator)}};
        }

      private:
        static std::vector<long> calculate_bit_frequency_indicator(const std::vector<std::string> &numbers) {
            // High bits stored at lower index.
            auto counter = std::vector<long>{};

            for (const auto &str : numbers) {
                if (str.size() > counter.size()) {
                    counter.resize(str.size());
                }

                for (size_t i = 0; i < counter.size(); ++i) {
                    const auto c = i < str.size() ? str[i] : '0'; // Assume 0 if string is smaller than others

                    // Add -1 for 0 bits, 1 for 1 bits.
                    // The end summation sign will tell us whether each position contained more 1s ot more 0s.
                    counter[i] += c == '1' ? 1 : -1;
                }
            }

            return counter;
        }

        static unsigned long power_consumption(const std::vector<long> &counter) {
            const auto bits_difference_from_max = MAX_BITS_COUNT - counter.size();
            const auto bits_mask =
                (std::numeric_limits<unsigned long>::max() << bits_difference_from_max) >> bits_difference_from_max;

            // Reconstruct gamma from the gamma counter
            const auto gamma =
                bits_mask & std::accumulate(counter.begin(), counter.end(), 0, [](unsigned long gamma, long v) {
                    return (gamma << 1) | (v > 0 ? 1 : 0);
                });

            // Epsilon is just the bitwise negation of gamma
            const auto epsilon = bits_mask & ~gamma;

            return gamma * epsilon;
        }
    };
} // namespace Year2021::Day3

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day3::Solver{FILENAME}.print_answers();
    return 0;
}
