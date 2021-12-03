#include "../../solver.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Year2021::Day2 {
    using Base = ::Common::Solver<long>;

    class Solver : public Base {
        using Base::Solver;

        using SubPosition = struct {
            struct {
                long depth;
                long horizontal;
            } part1;
            struct {
                long depth;
                long horizontal;
                long aim;
            } part2;
        };

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            auto pos = SubPosition{{0, 0}, {0, 0, 0}};

            while (input.good() && input.peek() != std::char_traits<char>::eof()) {
                const auto instruction = read_instruction(input);

                auto executor = OPERATION_EXECUTORS.at(instruction.op);
                executor(pos, instruction.value);
            }

            return Base::Answers{
                Base::Answer{"Final horizontal x final depth", pos.part1.depth * pos.part1.horizontal},
                Base::Answer{"Final horizontal x final depth", pos.part2.depth * pos.part2.horizontal}};
        }

      private:
        enum class Operation : char { forward, up, down };
        static const std::unordered_map<std::string, Operation> OPERATION_STRING_MAP;
        static const std::unordered_map<Operation, std::function<void(SubPosition &, long value)>> OPERATION_EXECUTORS;

        using Instruction = struct {
            Operation op;
            long value;
        };

        static Instruction read_instruction(std::ifstream &input) {
            long v;
            auto s = std::string{};
            if (!(input >> s >> v)) {
                throw Error{"malformed_input"};
            }

            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            return Instruction{OPERATION_STRING_MAP.at(s), v};
        }
    };

    const std::unordered_map<std::string, Solver::Operation> Solver::OPERATION_STRING_MAP =
        std::unordered_map<std::string, Operation>{
            {"forward", Operation::forward}, {"up", Operation::up}, {"down", Operation::down}};

    const std::unordered_map<Solver::Operation, std::function<void(Solver::SubPosition &, long value)>>
        Solver::OPERATION_EXECUTORS =
            std::unordered_map<Operation, std::function<void(Solver::SubPosition &, long value)>>{
                {Operation::forward,
                 [](SubPosition &pos, long v) {
                     pos.part1.horizontal += v;
                     pos.part2.horizontal += v;
                     pos.part2.depth += pos.part2.aim * v;
                 }},
                {Operation::down,
                 [](SubPosition &pos, long v) {
                     pos.part1.depth += v;
                     pos.part2.aim += v;
                 }},
                {Operation::up,
                 [](SubPosition &pos, long v) {
                     pos.part1.depth -= v;
                     pos.part2.aim -= v;
                 }},
            };
} // namespace Year2021::Day2

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day2::Solver{FILENAME}.print_answers();
    return 0;
}
