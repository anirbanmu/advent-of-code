#include "../../solver.h"

#include <bitset>
#include <functional>
#include <iterator>
#include <numeric>
#include <unordered_set>

namespace Year2021::Day3 {
    using Base = ::Common::Solver<unsigned long>;

    static const size_t MAX_BITS_COUNT = std::numeric_limits<unsigned long>::digits;

    class Solver : public Base {
        using Base::Solver;
        using BitFrequencyIndicatorValue = long;
        using BitFrequencyIndicator = std::vector<BitFrequencyIndicatorValue>;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            const auto numbers = std::vector<std::string>{std::istream_iterator<std::string>(input),
                                                          std::istream_iterator<std::string>{}};
            if (numbers.empty()) {
                throw Error{"no_data"};
            }

            const size_t bit_count = numbers.front().size();
            if (bit_count > MAX_BITS_COUNT ||
                !std::all_of(numbers.begin(), numbers.end(), [=](const auto &s) { return s.size() == bit_count; })) {
                throw Error{"malformed_number_of_bits_in_input"};
            }

            const auto bit_frequency_indicator = calculate_bit_frequency_indicator(numbers);

            return Base::Answers{
                Base::Answer{"Power consumption (gamma x epsilon)", power_consumption(bit_frequency_indicator)},
                Base::Answer{"Life support rating (o2 rating x co2 rating)",
                             life_support_rating(numbers, bit_frequency_indicator)}};
        }

      private:
        static unsigned long power_consumption(const BitFrequencyIndicator &bit_frequency_indicator) {
            const auto bits_difference_from_max = MAX_BITS_COUNT - bit_frequency_indicator.size();
            const auto bits_mask =
                (std::numeric_limits<unsigned long>::max() << bits_difference_from_max) >> bits_difference_from_max;

            // Reconstruct gamma from the gamma counter
            const auto gamma = bits_mask &
                std::accumulate(bit_frequency_indicator.begin(),
                                bit_frequency_indicator.end(),
                                0,
                                [](unsigned long gamma, BitFrequencyIndicatorValue v) {
                                    return (gamma << 1) | (v > 0 ? 1 : 0);
                                });

            // Epsilon is just the bitwise negation of gamma
            const auto epsilon = bits_mask & ~gamma;

            return gamma * epsilon;
        }

        static unsigned long life_support_rating(const std::vector<std::string> &numbers,
                                                 const BitFrequencyIndicator &bit_frequency_indicator) {
            return oxygen_generator_rating(numbers, bit_frequency_indicator) *
                carbon_dioxide_scrubber_rating(numbers, bit_frequency_indicator);
        }

        static unsigned long oxygen_generator_rating(const std::vector<std::string> &numbers,
                                                     const BitFrequencyIndicator &bit_frequency_indicator) {
            return find_rating_by_rule(numbers, bit_frequency_indicator, [](auto v) { return v > -1 ? '1' : '0'; });
        }

        static unsigned long carbon_dioxide_scrubber_rating(const std::vector<std::string> &numbers,
                                                            const BitFrequencyIndicator &bit_frequency_indicator) {
            return find_rating_by_rule(numbers, bit_frequency_indicator, [](auto v) { return v > -1 ? '0' : '1'; });
        }

        static unsigned long find_rating_by_rule(const std::vector<std::string> &numbers,
                                                 const BitFrequencyIndicator &bit_frequency_indicator,
                                                 std::function<char(BitFrequencyIndicatorValue)> bit_to_keep) {
            auto counter = BitFrequencyIndicator(bit_frequency_indicator);

            auto valid_tracker = std::vector<bool>(numbers.size(), true);
            size_t valid_count = numbers.size();
            size_t bit_index = 0;
            while (valid_count > 1) {
                const auto char_to_keep = bit_to_keep(counter[bit_index]);

                for (size_t i = 0; i < valid_tracker.size(); ++i) {
                    if (!valid_tracker[i]) {
                        continue;
                    }

                    const auto &str = numbers[i];
                    const auto c = str[bit_index];
                    if (c != char_to_keep) {
                        decrement_bit_frequency_counter(counter, str);
                        valid_tracker[i] = false;
                        --valid_count;
                    }
                }

                ++bit_index;
            }

            for (size_t i = 0; i < valid_tracker.size(); ++i) {
                if (valid_tracker[i]) {
                    return std::bitset<MAX_BITS_COUNT>{numbers[i]}.to_ulong();
                }
            }

            throw Error{"should_not_be_reachable"};
            return 0;
        }

        static const BitFrequencyIndicatorValue FREQ_VALUE_FOR_0 = -1;
        static const BitFrequencyIndicatorValue FREQ_VALUE_FOR_1 = 1;

        // Calculates an integer to indicate frequency of 0 or 1 in each bit position.
        // negative value means 0 is more frequent
        // positive value means 1 is more frequent
        // 0 means 0 & 1 are equally frequent
        static BitFrequencyIndicator calculate_bit_frequency_indicator(const std::vector<std::string> &numbers) {
            // High bits stored at lower index.
            auto counter = BitFrequencyIndicator(numbers.front().size(), 0);

            for (const auto &str : numbers) {
                increment_bit_frequency_counter(counter, str);
            }

            return counter;
        }

        static void increment_bit_frequency_counter(BitFrequencyIndicator &counter, const std::string &str) {
            for (size_t i = 0; i < counter.size(); ++i) {
                counter[i] += str[i] == '1' ? FREQ_VALUE_FOR_1 : FREQ_VALUE_FOR_0;
            }
        }

        static void decrement_bit_frequency_counter(BitFrequencyIndicator &counter, const std::string &str) {
            for (size_t i = 0; i < counter.size(); ++i) {
                counter[i] += str[i] == '1' ? -FREQ_VALUE_FOR_1 : -FREQ_VALUE_FOR_0;
            }
        }
    };
} // namespace Year2021::Day3

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day3::Solver{FILENAME}.print_answers();
    return 0;
}
