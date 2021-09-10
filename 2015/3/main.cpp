#include "../../solver.h"

#include <fstream>
#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace Year2015::Day3 {
    using Base = ::Common::Solver<size_t>;

    struct Point {
        int x, y;

        bool operator==(const Point &other) const { return x == other.x && y == other.y; }

        struct Hash : public std::unary_function<Point, std::size_t> {
            std::size_t operator()(const Point &p) const {
                static_assert(sizeof(p.x) * 2 == sizeof(size_t),
                              "Point::Hash assumes that x & y can fit into hash key");
                static_assert(std::numeric_limits<decltype(p.x)>::digits + 1 == 32, "32 bit ints");
                return (size_t(p.x) << (std::numeric_limits<decltype(p.x)>::digits + 1)) | (0xFFFFFFFF & p.y);
            }
        };
    };

    class Solver : public Base {
        using Base::Solver;

        const std::unordered_map<char, std::function<Point(const Point &)>> ACTIONS =
            std::unordered_map<char, std::function<Point(const Point &)>>{{{'>',
                                                                            [](const Point &p) {
                                                                                return Point{p.x + 1, p.y};
                                                                            }},
                                                                           {'<',
                                                                            [](const Point &p) {
                                                                                return Point{p.x - 1, p.y};
                                                                            }},
                                                                           {'^',
                                                                            [](const Point &p) {
                                                                                return Point{p.x, p.y + 1};
                                                                            }},
                                                                           {'v', [](const Point &p) {
                                                                                return Point{p.x, p.y - 1};
                                                                            }}}};

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            struct {
                std::unordered_set<Point, Point::Hash> part1, part2;
            } houses_with_presents{std::unordered_set<Point, Point::Hash>{Point{0, 0}},
                                   std::unordered_set<Point, Point::Hash>{Point{0, 0}}};

            struct {
                struct {
                    Point santa;
                } part1;
                struct {
                    Point santa, robo;
                } part2;
            } current_position{};

            auto robo_turn{false};
            char c;
            while (input >> c) {
                const auto action = ACTIONS.at(c);

                current_position.part1.santa = action(current_position.part1.santa);
                houses_with_presents.part1.insert(current_position.part1.santa);

                if (robo_turn) {
                    current_position.part2.robo = action(current_position.part2.robo);
                    houses_with_presents.part2.insert(current_position.part2.robo);
                } else {
                    current_position.part2.santa = action(current_position.part2.santa);
                    houses_with_presents.part2.insert(current_position.part2.santa);
                }
                robo_turn = !robo_turn;
            }

            return Base::Answers{Base::Answer{"Houses delivered with just Santa", houses_with_presents.part1.size()},
                                 Base::Answer{"Houses delivered with Santa & Robo", houses_with_presents.part2.size()}};
        }
    };
} // namespace Year2015::Day3

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day3::Solver{FILENAME}.print_answers();
    return 0;
}
