#include "../../solver.h"

#include <fstream>
#include <unordered_map>

namespace Year2015::Day1 {
    using Base = ::Common::Solver<long>;

    class Solver : public Base {
        using Base::Solver;
        const std::unordered_map<char, int> FLOOR_CHANGE_MAP = std::unordered_map<char, int>{{'(', 1}, {')', -1}};

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            auto floor = 0;
            auto i = 0;
            auto basement_reaching_idx = -1;
            char c;
            while (++i, input >> c) {
                floor += FLOOR_CHANGE_MAP.at(c);
                if (floor == -1 && basement_reaching_idx == -1) {
                    basement_reaching_idx = i;
                }
            }

            return Base::Answers{Base::Answer{"Santa floor", floor},
                                 Base::Answer{"Santa basement reaching position", basement_reaching_idx}};
        }
    };
} // namespace Year2015::Day1

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day1::Solver{FILENAME}.print_answers();
    return 0;
}
