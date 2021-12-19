#include "../../solver.h"
#include "../../utils.h"

#include <cstring>
#include <numeric>
#include <sstream>
#include <unordered_set>

namespace Year2021::Day13 {
    using std::string;
    using std::tuple;
    using std::unordered_set;
    using std::vector;

    using Base = ::Common::Solver<string>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

        struct FoldedDotTracker {
            struct Hash : public std::unary_function<tuple<int, int>, std::size_t> {
                std::size_t operator()(const tuple<int, int> &p) const {
                    static_assert(sizeof(std::get<0>(p)) * 2 == sizeof(size_t),
                                  "Point::Hash assumes that x & y can fit into hash key");
                    static_assert(sizeof(std::get<0>(p)) == 4, "32 bit ints");
                    return (size_t(std::get<0>(p)) << 32) | (0xFFFFFFFF & std::get<1>(p));
                }
            };

            using dots_set = unordered_set<tuple<int, int>, Hash>;

            dots_set dots[2];
            size_t next_idx;

            FoldedDotTracker(const vector<tuple<int, int>> &d) : next_idx(1) {
                std::copy(d.begin(), d.end(), std::inserter(dots[0], dots[0].end()));
            }

            const dots_set &current_dots() const { return dots[(next_idx - 1) % 2]; }
            dots_set &next_dots() { return dots[next_idx % 2]; }
            void swap() { ++next_idx; }

            std::tuple<int, int> current_maximum() const {
                return std::accumulate(
                    current_dots().begin(),
                    current_dots().end(),
                    std::make_tuple(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()),
                    [](const auto &acc, const auto &dot) {
                        return std::make_tuple(std::max(std::get<0>(acc), std::get<0>(dot)),
                                               std::max(std::get<1>(acc), std::get<1>(dot)));
                    });
            }
        };

        //
        // Written with the assumption that we're instructed to fold such that the paper never folds past the left or
        // top edge.
        //
        Answers solve(std::ifstream &input) const override {
            const auto [dots, instructions] = read_initial_state_and_instructions(input);

            auto tracker = FoldedDotTracker(dots);

            const auto get_folded_value = [](int old_value, int fold_along_value) {
                if (old_value <= fold_along_value) {
                    // Nothing changes
                    return old_value;
                }
                return fold_along_value - old_value + fold_along_value;
            };

            size_t dot_count_after_first_fold = 0;

            for (size_t i = 0; i < instructions.size(); ++i) {
                const auto &inst = instructions[i];

                auto &next = tracker.next_dots();
                next.clear();

                const auto fold_along_value = inst.value;

                switch (inst.direction) {
                case FoldAlong::x:
                    for (const auto &dot : tracker.current_dots()) {
                        const auto x = get_folded_value(std::get<0>(dot), fold_along_value);
                        next.emplace(x, std::get<1>(dot));
                    }
                    break;
                case FoldAlong::y:
                    for (const auto &dot : tracker.current_dots()) {
                        const auto y = get_folded_value(std::get<1>(dot), fold_along_value);
                        next.emplace(std::get<0>(dot), y);
                    }
                    break;
                }

                tracker.swap();

                if (i == 0) {
                    dot_count_after_first_fold = tracker.current_dots().size();
                }
            }

            const auto final_pattern = [&]() {
                auto final_pattern_sstream = std::ostringstream{};
                const auto &final_dots = tracker.current_dots();
                const auto maximum = tracker.current_maximum();

                final_pattern_sstream << std::endl;
                for (auto y = 0; y <= std::get<1>(maximum); ++y) {
                    for (auto x = 0; x <= std::get<0>(maximum); ++x) {
                        if (final_dots.contains(std::make_tuple(x, y))) {
                            final_pattern_sstream << '#';
                            continue;
                        }
                        final_pattern_sstream << ' ';
                    }
                    final_pattern_sstream << std::endl;
                }
                return final_pattern_sstream.str();
            }();

            return Answers{Answer{"Dots visible after first fold", std::to_string(dot_count_after_first_fold)},
                           Answer{"Folded pattern", final_pattern}};
        };

      private:
        static const char *const FOLD_INSTRUCTION_X_PREFIX;
        static const char *const FOLD_INSTRUCTION_Y_PREFIX;
        static const size_t FOLD_INSTRUCTION_PREFIX_LENGTH = std::strlen("fold along x=");

        enum class FoldAlong { x, y };
        struct FoldInstruction {
            FoldAlong direction;
            int value;

            FoldInstruction(FoldAlong d, int v) : direction(d), value(v) {}
        };

        struct DotsAndInstructions {
            vector<tuple<int, int>> dots;
            vector<FoldInstruction> instructions;
        };

        static DotsAndInstructions read_initial_state_and_instructions(std::ifstream &input) {
            auto out = DotsAndInstructions{};

            auto line = string{};
            for (string line; std::getline(input, line);) {
                const auto line_view = std::string_view{line};
                const auto fold_line_prefix = line_view.substr(0, FOLD_INSTRUCTION_PREFIX_LENGTH);
                if (fold_line_prefix == FOLD_INSTRUCTION_X_PREFIX) {
                    out.instructions.emplace_back(
                        FoldAlong::x, Utils::str_to_int<int>(line_view.substr(FOLD_INSTRUCTION_PREFIX_LENGTH)));
                    continue;
                }
                if (fold_line_prefix == FOLD_INSTRUCTION_Y_PREFIX) {
                    out.instructions.emplace_back(
                        FoldAlong::y, Utils::str_to_int<int>(line_view.substr(FOLD_INSTRUCTION_PREFIX_LENGTH)));
                    continue;
                }

                auto line_stream = std::istringstream(line);
                int x, y;
                char comma;
                if (line_stream >> x >> comma >> y && comma == ',') {
                    out.dots.emplace_back(x, y);
                }
            }

            return out;
        }
    };

    const char *const Solver::FOLD_INSTRUCTION_X_PREFIX = "fold along x=";
    const char *const Solver::FOLD_INSTRUCTION_Y_PREFIX = "fold along y=";
} // namespace Year2021::Day13

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day13::Solver{FILENAME}.print_answers();
    return 0;
}
