#include "../../solver.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Year2015::Day5 {
    using Base = ::Common::Solver<unsigned long>;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            std::string word;

            struct {
                unsigned long part1, part2;
            } nice_words_count{0, 0};

            while (input >> word) {
                auto const niceness = is_nice(word);
                nice_words_count.part1 += niceness.part1 ? 1 : 0;
                nice_words_count.part2 += niceness.part2 ? 1 : 0;
            }
            return Base::Answers{Base::Answer{"Nice strings count", nice_words_count.part1},
                                 Base::Answer{"Nice strings count", nice_words_count.part2}};
        }

      private:
        const std::unordered_set<char> VOWELS = std::unordered_set<char>{'a', 'e', 'i', 'o', 'u'};
        const std::unordered_set<std::string> DISALLOWED_TWO_TUPLES =
            std::unordered_set<std::string>{"ab", "cd", "pq", "xy"};

        using Niceness = struct { bool part1, part2; };

        Niceness is_nice(const std::string &s) const {
            struct {
                struct {
                    unsigned long vowel_count;
                    bool has_consecutive_repeated_character;
                    bool all_two_tuples_allowed;
                } part1;
                struct {
                    bool has_non_overlapping_repeated_two_tuple;
                    bool has_skip_character_repeat;
                } part2;
            } tracker{{0, false, true}, {false, false}};

            auto two_tuples = std::unordered_map<std::string, size_t>{};

            const auto char_count = s.size();
            for (size_t i = 0; i < char_count; ++i) {
                const auto current = s[i];

                // Track vowels
                if (VOWELS.find(current) != VOWELS.end()) {
                    ++tracker.part1.vowel_count;
                }

                if (i < char_count - 1) {
                    // Find 2 letter repeats
                    const auto next = s[i + 1];
                    if (current == next) {
                        tracker.part1.has_consecutive_repeated_character = true;
                    }

                    const auto two_tuple = s.substr(i, 2);

                    // Check for disallowed tuple
                    if (DISALLOWED_TWO_TUPLES.find(two_tuple) != DISALLOWED_TWO_TUPLES.end()) {
                        tracker.part1.all_two_tuples_allowed = false;
                    }

                    // Check for non-overlapping two tuple, otherwise store where we saw this tuple.
                    const auto &found = two_tuples.find(two_tuple);
                    if (found != two_tuples.end() && found->second != i - 1) {
                        tracker.part2.has_non_overlapping_repeated_two_tuple = true;
                    } else {
                        two_tuples.emplace(two_tuple, i);
                    }
                }

                if (i < char_count - 2) {
                    // Check for character repeat separated by one character
                    if (current == s[i + 2]) {
                        tracker.part2.has_skip_character_repeat = true;
                    }
                }
            }

            return Niceness{tracker.part1.vowel_count >= 3 && tracker.part1.has_consecutive_repeated_character &&
                                tracker.part1.all_two_tuples_allowed,
                            tracker.part2.has_non_overlapping_repeated_two_tuple &&
                                tracker.part2.has_skip_character_repeat};
        }
    };
} // namespace Year2015::Day5

const auto FILENAME{"input.txt"};

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day5::Solver{FILENAME}.print_answers();
    return 0;
}
