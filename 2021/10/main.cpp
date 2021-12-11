#include "../../solver.h"

#include <deque>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace Year2021::Day10 {
    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

        Answers solve(std::ifstream &input) const override {
            auto opening_characters = std::deque<char>{};

            auto syntax_error_score_for_corrupt_lines = (unsigned long long){0};
            auto completion_score = std::vector<unsigned long long>{};

            auto line = std::string{};
            while (input >> line) {
                for (const auto c : line) {
                    if (!VALID_CHARS.contains(c)) {
                        throw Error{"disallowed_character"};
                    }

                    if (STARTING_CHARS.contains(c)) {
                        opening_characters.emplace_front(c);
                        continue;
                    }

                    const auto starting_char_to_match = CLOSING_CHAR_TO_STARTING_CHAR_MAP.at(c);

                    if (!opening_characters.empty()) {
                        const auto found_starting_char = opening_characters.front();
                        opening_characters.pop_front();

                        if (found_starting_char != starting_char_to_match) {
                            // corrupted line
                            syntax_error_score_for_corrupt_lines += FIRST_ILLEGAL_CLOSING_CHAR_TO_SCORE.at(c);
                            opening_characters.clear(); // clear this so that we can detect incomplete lines properly
                            break;
                        }
                    } else {
                        throw Error{"unhandled_case_extra_closing_characters"};
                    }
                }

                if (!opening_characters.empty()) {
                    // incomplete line
                    completion_score.emplace_back(std::accumulate(
                        opening_characters.begin(),
                        opening_characters.end(),
                        (unsigned long long)0,
                        [](auto acc, auto c) { return acc * 5 + LEFT_OVER_STARTING_CHAR_TO_SCORE.at(c); }));
                }

                opening_characters.clear();
            }

            // Sort enough to get median
            std::nth_element(completion_score.begin(),
                             completion_score.begin() + (completion_score.size() / 2),
                             completion_score.end());
            const auto median = completion_score[completion_score.size() / 2];

            return Answers{Answer{"Total syntax error score for first illegal character for corrupt lines",
                                  syntax_error_score_for_corrupt_lines},
                           Answer{"Middle completion score", median}};
        }

        static const std::unordered_set<char> VALID_CHARS;
        static const std::unordered_set<char> STARTING_CHARS;
        static const std::unordered_map<char, char> CLOSING_CHAR_TO_STARTING_CHAR_MAP;
        static const std::unordered_map<char, unsigned> FIRST_ILLEGAL_CLOSING_CHAR_TO_SCORE;
        static const std::unordered_map<char, unsigned> LEFT_OVER_STARTING_CHAR_TO_SCORE;
    };

    const std::unordered_set<char> Solver::VALID_CHARS =
        std::unordered_set<char>{')', '(', ']', '[', '}', '{', '>', '<'};

    const std::unordered_set<char> Solver::STARTING_CHARS = std::unordered_set<char>{'(', '[', '{', '<'};

    const std::unordered_map<char, char> Solver::CLOSING_CHAR_TO_STARTING_CHAR_MAP =
        std::unordered_map<char, char>{{')', '('}, {']', '['}, {'}', '{'}, {'>', '<'}};

    const std::unordered_map<char, unsigned> Solver::FIRST_ILLEGAL_CLOSING_CHAR_TO_SCORE =
        std::unordered_map<char, unsigned>{{')', 3}, {']', 57}, {'}', 1197}, {'>', 25137}};

    const std::unordered_map<char, unsigned> Solver::LEFT_OVER_STARTING_CHAR_TO_SCORE =
        std::unordered_map<char, unsigned>{{'(', 1}, {'[', 2}, {'{', 3}, {'<', 4}};
} // namespace Year2021::Day10

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day10::Solver{FILENAME}.print_answers();
    return 0;
}
