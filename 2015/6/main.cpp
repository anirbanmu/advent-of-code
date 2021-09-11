#include "../../solver.h"

#include <array>
#include <bitset>
#include <fstream>
#include <functional>
#include <numeric>
#include <unordered_map>

namespace Year2015::Day6 {
    using Base = ::Common::Solver<unsigned long>;

    template <size_t width, size_t height> class BitGrid {
      public:
        BitGrid &set(size_t x, size_t y, bool value = true) {
            arr.set(idx(x, y), value);
            return *this;
        }

        BitGrid &toggle(size_t x, size_t y) {
            arr.flip(idx(x, y));
            return *this;
        }

        BitGrid &reset(size_t x, size_t y) {
            arr.reset(idx(x, y));
            return *this;
        }

        size_t count() const { return arr.count(); }

      private:
        size_t idx(size_t x, size_t y) const { return (y * width) + x; }

        static const size_t total = width * height;
        std::bitset<total> arr;
    };

    template <size_t width, size_t height> class BrightnessGrid {
      public:
        BrightnessGrid &increase_brightness(size_t x, size_t y, unsigned long change) {
            const auto i = idx(x, y);
            brightness[i] = brightness[i] + change;
            return *this;
        }

        BrightnessGrid &decrease_brightness(size_t x, size_t y, unsigned long change) {
            const auto i = idx(x, y);
            const auto current = brightness[i];
            brightness[i] = change > current ? 0 : current - change;
            return *this;
        }

        unsigned long total_brightness() const { return std::accumulate(brightness.begin(), brightness.end(), 0); }

      private:
        size_t idx(size_t x, size_t y) const { return (y * width) + x; }

        static const size_t total = width * height;
        std::array<unsigned long, total> brightness;
    };

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            std::string line;

            auto bitgrid = BitGrid<1000, 1000>{};
            auto brightness_grid = BrightnessGrid<1000, 1000>{};

            const struct {
                std::unordered_map<Opcode, std::function<void(decltype(bitgrid) &, size_t x, size_t y)>> part1;
                std::unordered_map<Opcode, std::function<void(decltype(brightness_grid) &, size_t x, size_t y)>> part2;
            } executors{
                std::unordered_map<Opcode, std::function<void(decltype(bitgrid) &, size_t x, size_t y)>>{
                    {Opcode::on, [](decltype(bitgrid) &grid, size_t x, size_t y) { grid.set(x, y); }},
                    {Opcode::off, [](decltype(bitgrid) &grid, size_t x, size_t y) { grid.reset(x, y); }},
                    {Opcode::toggle, [](decltype(bitgrid) &grid, size_t x, size_t y) { grid.toggle(x, y); }}},
                std::unordered_map<Opcode, std::function<void(decltype(brightness_grid) &, size_t x, size_t y)>>{
                    {Opcode::on,
                     [](decltype(brightness_grid) &grid, size_t x, size_t y) { grid.increase_brightness(x, y, 1); }},
                    {Opcode::off,
                     [](decltype(brightness_grid) &grid, size_t x, size_t y) { grid.decrease_brightness(x, y, 1); }},
                    {Opcode::toggle,
                     [](decltype(brightness_grid) &grid, size_t x, size_t y) { grid.increase_brightness(x, y, 2); }}}};

            while (input.good() && input.peek() != std::char_traits<char>::eof()) {
                const auto instruction = read_instruction(input);

                const struct {
                    std::function<void(decltype(bitgrid) &, size_t x, size_t y)> part1;
                    std::function<void(decltype(brightness_grid) &, size_t x, size_t y)> part2;
                } actions{executors.part1.at(instruction.opcode), executors.part2.at(instruction.opcode)};

                for (size_t y = instruction.rect.start.y; y <= instruction.rect.end.y; ++y) {
                    for (size_t x = instruction.rect.start.x; x <= instruction.rect.end.x; ++x) {
                        actions.part1(bitgrid, x, y);
                        actions.part2(brightness_grid, x, y);
                    }
                }
            }

            return Base::Answers{Base::Answer{"Total lights lit", bitgrid.count()},
                                 Base::Answer{"Total brightness", brightness_grid.total_brightness()}};
        }

      private:
        enum class Opcode : char { on, off, toggle };

        using Rect = struct {
            struct {
                unsigned long x, y;
            } start, end;
        };

        using Instruction = struct {
            Opcode opcode;
            Rect rect;
        };

        static const std::unordered_map<std::string, Opcode> INSTRUCTION_STRING_MAP;

        static Instruction read_instruction(std::ifstream &input) {
            const auto instruction = Instruction{read_opcode(input), read_rect(input)};
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return instruction;
        }

        static Opcode read_opcode(std::ifstream &input) {
            std::string instruction;
            if (!(input >> instruction)) {
                throw Error{"malformed_input"};
            };
            if (instruction == "turn") {
                if (!(input >> instruction)) {
                    throw Error{"malformed_input"};
                };
            }

            return INSTRUCTION_STRING_MAP.at(instruction);
        }

        static Rect read_rect(std::ifstream &input) {
            std::string throwaway_str;
            char throwaway_char;

            auto out = Rect{};
            if (!(input >> out.start.x >> throwaway_char >> out.start.y >> throwaway_str >> out.end.x >>
                  throwaway_char >> out.end.y)) {
                throw Error{"malformed_input"};
            }
            return out;
        }
    };

    const std::unordered_map<std::string, Solver::Opcode> Solver::INSTRUCTION_STRING_MAP =
        std::unordered_map<std::string, Opcode>{{"on", Opcode::on}, {"off", Opcode::off}, {"toggle", Opcode::toggle}};
} // namespace Year2015::Day6

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day6::Solver{FILENAME}.print_answers();
    return 0;
}
