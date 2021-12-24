#include "../../solver.h"

#include <numeric>
#include <unordered_map>

namespace Year2021::Day14 {
    using std::string;
    using std::unordered_map;

    using Base = ::Common::Solver<size_t>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

        // Bits 15-8 represent first character
        // Bits 7-0 represent second character
        using EncodedCharacterPair = uint16_t;

        Answers solve(std::ifstream &input) const override {
            const auto [polymer_template, initial_pairs, insertion_rules] =
                read_encoded_template_pairs_and_rules(input);

            auto results = unordered_map<size_t, MinMaxOccurenceSnapshot>{{10, {}}, {40, {}}};

            auto char_frequency = unordered_map<char, size_t>{};
            for (char c : polymer_template) {
                ++char_frequency[c];
            }

            auto current = initial_pairs;
            auto scratch = initial_pairs;

            for (size_t step = 0; step < 40; ++step) {
                for (const auto &pair_count : current) {
                    const auto &rule = insertion_rules.find(pair_count.first);
                    if (rule != insertion_rules.end()) {
                        const char mid_char = rule->second;
                        const auto count = pair_count.second;
                        const auto encoded_pair = pair_count.first;

                        // Add the two new pairs created when we insert the rule char in the middle.
                        scratch[encode_character_pair(decode_char<0>(encoded_pair), mid_char)] += count;
                        scratch[encode_character_pair(mid_char, decode_char<1>(encoded_pair))] += count;

                        // The original count pairs are gone now since we've eliminated them by adding a char in the
                        // middle.
                        scratch[encoded_pair] -= count;

                        // We added count new mid_char's.
                        char_frequency[mid_char] += count;
                    }
                }

                // Done with this iteration; make scratch the new current.
                current = scratch;

                // If we're supposed to record occurences here, take a snapshot.
                if (results.contains(step + 1)) {
                    results[step + 1] = take_min_max_occurence_snapshot(char_frequency);
                }
            }

            return Answers{Answer{"Difference of most common & least common element (after step 10)",
                                  results.at(10).max_occurences - results.at(10).min_occurences},
                           Answer{"Difference of most common & least common element (after step 40)",
                                  results.at(40).max_occurences - results.at(40).min_occurences}};
        };

      private:
        struct MinMaxOccurenceSnapshot {
            size_t max_occurences;
            size_t min_occurences;
        };

        static MinMaxOccurenceSnapshot take_min_max_occurence_snapshot(const unordered_map<char, size_t> &frequency) {
            return MinMaxOccurenceSnapshot{
                std::accumulate(frequency.begin(),
                                frequency.end(),
                                size_t{0},
                                [](size_t acc, const auto &f) { return acc > f.second ? acc : f.second; }),
                std::accumulate(frequency.begin(),
                                frequency.end(),
                                size_t{std::numeric_limits<size_t>::max()},
                                [](size_t acc, const auto &f) { return acc < f.second ? acc : f.second; })};
        }

        struct PolymerTemplateAndRulesEncoded {
            string polymer_template;
            unordered_map<EncodedCharacterPair, size_t> pairs;
            unordered_map<EncodedCharacterPair, char> pair_insertion_rules;
        };

        static EncodedCharacterPair encode_character_pair(char a, char b) { return (a << 8) | b; }

        template <size_t idx> static char decode_char(EncodedCharacterPair v) {
            if (idx == 0) {
                return (v >> 8) & 0xFF;
            }
            return v & 0xFF;
        }

        static PolymerTemplateAndRulesEncoded read_encoded_template_pairs_and_rules(std::ifstream &input) {
            const auto [polymer_template, insertion_rules] = read_polymer_template_and_rules(input);

            auto out = PolymerTemplateAndRulesEncoded{polymer_template, {}, {}};
            if (polymer_template.size() >= 2) {
                for (size_t i = 1; i < polymer_template.size(); ++i) {
                    ++out.pairs[encode_character_pair(polymer_template[i - 1], polymer_template[i])];
                }
            }

            for (const auto &rule : insertion_rules) {
                out.pair_insertion_rules.emplace(encode_character_pair(rule.first[0], rule.first[1]), rule.second);
            }

            return out;
        }

        struct PolymerTemplateAndRules {
            string polymer_template;
            unordered_map<string, char> pair_insertion_rules;
        };

        static PolymerTemplateAndRules read_polymer_template_and_rules(std::ifstream &input) {
            auto out = PolymerTemplateAndRules{};

            if (!(input >> out.polymer_template)) {
                throw Error{"malformed_input"};
            }

            string pair_match, throwaway;
            char insertion_char;
            while ((input >> pair_match >> throwaway >> insertion_char) && throwaway == "->" &&
                   pair_match.size() == 2) {
                out.pair_insertion_rules.emplace(pair_match, insertion_char);
            }

            return out;
        }
    };
} // namespace Year2021::Day14

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day14::Solver{FILENAME}.print_answers();
    return 0;
}
