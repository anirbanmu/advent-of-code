#include "../../solver.h"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <unordered_map>

namespace Year2021::Day8 {
    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

        static const size_t SIGNAL_VALUE_COUNT = 10;
        static const size_t OUTPUT_VALUE_DIGITS = 4;
        static const size_t SEGMENT_COUNT = 7;
        using RawSignal = std::bitset<SEGMENT_COUNT>; // Represents which segments are lit, a == 0, b == 1 .. g = 6

      protected:
        Answers solve(std::ifstream &input) const override {
            size_t unique_segment_number_count = 0;
            unsigned long long total_sum = 0;

            while (input.good() && input.peek() != std::char_traits<char>::eof()) {
                const auto digits = calculate_output_digits(input);

                auto full_number = (unsigned long long){0};
                for (size_t i = 0; i < digits.size(); ++i) {
                    const auto x = digits[i];
                    if (x == 1 || x == 4 || x == 7 || x == 8) {
                        ++unique_segment_number_count;
                    }
                    full_number *= 10;
                    full_number += x;
                }
                total_sum += full_number;
            }
            return Answers{Answer{"Count of 1,4,7,8 in output", unique_segment_number_count},
                           Answer{"Sum of all output numbers", total_sum}};
        }

      private:
        static std::array<uint8_t, OUTPUT_VALUE_DIGITS> calculate_output_digits(std::ifstream &input) {
            auto [signals, digits] = read_signals_and_digits(input);

            const auto signal_to_digit_mapping = calculate_signal_to_digit_mapping(signals);

            auto digits_as_integers = std::array<uint8_t, OUTPUT_VALUE_DIGITS>{};
            for (size_t i = 0; i < digits.size(); ++i) {
                digits_as_integers[i] = signal_to_digit_mapping.at(digits[i]);
            }

            return digits_as_integers;
        }

        // These representations have unique # of segments so can be found just by counting segments.
        enum class KnownSegmentCounterDigit : uint8_t { one = 2, four = 4, seven = 3, eight = 7 };

        static RawSignal
        known_segment_count_signal(KnownSegmentCounterDigit digit,
                                   const std::unordered_map<uint8_t, std::vector<RawSignal>> &raw_signals_by_segments) {
            const auto segment_count = static_cast<uint8_t>(digit);
            const auto all = raw_signals_by_segments.find(segment_count);
            if (all == raw_signals_by_segments.end() || all->second.size() != 1) {
                throw Error{"malformed_input"};
            }
            return all->second.front();
        }

        struct Mapping {
            std::array<RawSignal, SIGNAL_VALUE_COUNT> digit_to_signal;
            std::unordered_map<RawSignal, uint8_t> signal_to_digit;

            void add(RawSignal signal, uint8_t digit) {
                digit_to_signal[digit] = signal;
                signal_to_digit[signal] = digit;
            }
        };

        // Create mapping from seven-segment bitset to the digit it represents.
        static std::unordered_map<RawSignal, uint8_t> calculate_signal_to_digit_mapping(
            const std::unordered_map<uint8_t, std::vector<RawSignal>> &raw_signals_by_segments) {
            auto mapping = Mapping{};
            mapping.add(known_segment_count_signal(KnownSegmentCounterDigit::one, raw_signals_by_segments), 1);
            mapping.add(known_segment_count_signal(KnownSegmentCounterDigit::four, raw_signals_by_segments), 4);
            mapping.add(known_segment_count_signal(KnownSegmentCounterDigit::seven, raw_signals_by_segments), 7);
            mapping.add(known_segment_count_signal(KnownSegmentCounterDigit::eight, raw_signals_by_segments), 8);

            find_non_unique_mappings(raw_signals_by_segments, mapping);

            return mapping.signal_to_digit;
        }

        // mappings for 1,4,7,8 are expected to already be in accumulated_mapping
        static void
        find_non_unique_mappings(const std::unordered_map<uint8_t, std::vector<RawSignal>> &raw_signals_by_segments,
                                 Mapping &accumulated_mapping) {
            // 0, 6, 9 have 6 segments each.
            {
                auto six_segment_signals = raw_signals_by_segments.at(6);

                // If we subtract the segments of 1 from these, only the one that represents 6 will have 5 segments.
                auto found = std::find_if(six_segment_signals.begin(), six_segment_signals.end(), [&](auto &v) {
                    return 5 == (v & ~accumulated_mapping.digit_to_signal[1]).count();
                });
                if (found == six_segment_signals.end()) {
                    throw Error{"malformed_input"};
                }
                accumulated_mapping.add(*found, 6);
                six_segment_signals.erase(found);

                // If we subtract the segments of 4 from the rest, only 0 will have 3 segments.
                found = std::find_if(six_segment_signals.begin(), six_segment_signals.end(), [&](auto &v) {
                    return 3 == (v & ~accumulated_mapping.digit_to_signal[4]).count();
                });
                if (found == six_segment_signals.end()) {
                    throw Error{"malformed_input"};
                }
                accumulated_mapping.add(*found, 0);
                six_segment_signals.erase(found);

                // One left over is 9
                accumulated_mapping.add(six_segment_signals.front(), 9);
            }

            // 2, 3, 5 have 5 segments each.
            {
                auto five_segment_signals = raw_signals_by_segments.at(5);

                // If we subtract the segments of 1 from these, only the one that represents 3 will have 3 segments
                auto found = std::find_if(five_segment_signals.begin(), five_segment_signals.end(), [&](auto &v) {
                    return 3 == (v & ~accumulated_mapping.digit_to_signal[1]).count();
                });
                if (found == five_segment_signals.end()) {
                    throw Error{"malformed_input"};
                }
                accumulated_mapping.add(*found, 3);
                five_segment_signals.erase(found);

                // If we subtract the segments of 9 from the rest, only 5 will have 0 segments.
                found = std::find_if(five_segment_signals.begin(), five_segment_signals.end(), [&](auto &v) {
                    return 0 == (v & ~accumulated_mapping.digit_to_signal[9]).count();
                });
                if (found == five_segment_signals.end()) {
                    throw Error{"malformed_input"};
                }
                accumulated_mapping.add(*found, 5);
                five_segment_signals.erase(found);

                // One left over is 2
                accumulated_mapping.add(five_segment_signals.front(), 2);
            }
        }

        struct RawSignalsAndDigits {
            std::unordered_map<uint8_t, std::vector<RawSignal>> signals;
            std::vector<RawSignal> digits;
        };

        static RawSignalsAndDigits read_signals_and_digits(std::ifstream &input) {
            auto out = RawSignalsAndDigits{};

            auto str = std::string{};
            for (size_t i = 0; i < SIGNAL_VALUE_COUNT; ++i) {
                if (!(input >> str)) {
                    throw Error{"malformed_input"};
                }

                auto &bit_representation = out.signals[str.size()].emplace_back();
                for (const auto c : str) {
                    const auto idx = c - 'a';
                    bit_representation.set(idx);
                }
            }
            if (!(input >> str) || str != "|") {
                throw Error{"malformed_input"};
            }

            for (size_t i = 0; i < OUTPUT_VALUE_DIGITS; ++i) {
                if (!(input >> str)) {
                    throw Error{"malformed_input"};
                }

                auto &bit_representation = out.digits.emplace_back();
                for (const auto c : str) {
                    const auto idx = c - 'a';
                    bit_representation.set(idx);
                }
            }

            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            return out;
        }
    };
} // namespace Year2021::Day8

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day8::Solver{FILENAME}.print_answers();
    return 0;
}
