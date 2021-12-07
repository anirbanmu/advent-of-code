#include "../../solver.h"

#include <numeric>

namespace Year2021::Day5 {
    using Base = ::Common::Solver<unsigned long long>;
    using Point = struct { long long x, y; };
    using Line = std::pair<Point, Point>;

    class OverlapTracker {
      public:
        OverlapTracker(const std::vector<Line> &lines)
            : dimensions(std::accumulate(lines.begin(),
                                         lines.end(),
                                         std::make_pair(size_t(0), size_t(0)),
                                         [](auto acc, const auto &l) {
                                             return std::make_pair(
                                                 std::max((size_t)std::max(l.first.x, l.second.x), acc.first),
                                                 std::max((size_t)std::max(l.first.y, l.second.y), acc.second));
                                         })),
              overlapping_points(dimensions.first * dimensions.second),
              overlapping_points_from_straight_lines(dimensions.first * dimensions.second) {}

        void inc_for_straight_lines(const Point &p) {
            ++overlapping_points_from_straight_lines[p.y * dimensions.first + p.x];
        }

        void inc_for_including_diagonals(const Point &p) { ++overlapping_points[p.y * dimensions.first + p.x]; }

        void inc_all(const Point &p) {
            inc_for_straight_lines(p);
            inc_for_including_diagonals(p);
        }

        unsigned long long straight_line_overlap_count() const {
            return std::accumulate(overlapping_points_from_straight_lines.begin(),
                                   overlapping_points_from_straight_lines.end(),
                                   (unsigned long long)0,
                                   [](unsigned long long acc, const auto &v) { return v >= 2 ? acc + 1 : acc; });
        }

        unsigned long long overlap_count() const {
            return std::accumulate(overlapping_points.begin(),
                                   overlapping_points.end(),
                                   (unsigned long long)0,
                                   [](unsigned long long acc, const auto &v) { return v >= 2 ? acc + 1 : acc; });
        }

      private:
        const std::pair<size_t, size_t> dimensions;
        std::vector<unsigned long long> overlapping_points;
        std::vector<unsigned long long> overlapping_points_from_straight_lines;
    };

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            const auto lines = read_lines(input);
            auto overlap_tracker = OverlapTracker{lines};

            auto point = Point{};

            for (const auto &line : lines) {
                const auto x_increment = line.first.x > line.second.x ? -1 : 1;
                const auto x_end = line.first.x > line.second.x ? line.second.x - 1 : line.second.x + 1;
                const auto y_increment = line.first.y > line.second.y ? -1 : 1;
                const auto y_end = line.first.y > line.second.y ? line.second.y - 1 : line.second.y + 1;

                if (line.first.x == line.second.x) {
                    for (auto y = line.first.y; y != y_end; y += y_increment) {
                        point.x = line.first.x;
                        point.y = y;
                        overlap_tracker.inc_all(point);
                    }
                    continue;
                }

                if (line.first.y == line.second.y) {
                    for (auto x = line.first.x; x != x_end; x += x_increment) {
                        point.x = x;
                        point.y = line.first.y;
                        overlap_tracker.inc_all(point);
                    }
                    continue;
                }

                const bool is_straight_line = (line.first.x == line.second.x || line.first.y == line.second.y);

                auto x = line.first.x;
                auto y = line.first.y;
                while (x != x_end && y != y_end) {
                    point.x = x;
                    point.y = y;

                    overlap_tracker.inc_for_including_diagonals(point);
                    if (is_straight_line) {
                        overlap_tracker.inc_for_straight_lines(point);
                    }

                    x += x_increment;
                    y += y_increment;
                }
            }

            return Base::Answers{
                Base::Answer{"Number of points covered by at least 2 straight lines",
                             overlap_tracker.straight_line_overlap_count()},
                Base::Answer{"Number of points covered by at least 2 lines", overlap_tracker.overlap_count()}};
        }

      private:
        static std::vector<Line> read_lines(std::ifstream &input) {
            auto lines = std::vector<Line>{};

            Point start_point, end_point;
            char throwaway_char;
            std::string throwaway_str;
            while (input >> start_point.x >> throwaway_char >> start_point.y >> throwaway_str >> end_point.x >>
                   throwaway_char >> end_point.y) {
                lines.emplace_back(start_point, end_point);
            }

            return lines;
        }
    };
} // namespace Year2021::Day5

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day5::Solver{FILENAME}.print_answers();
    return 0;
}
