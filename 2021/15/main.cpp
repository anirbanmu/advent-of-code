#include "../../solver.h"

#include <numeric>
#include <queue>

namespace Year2021::Day15 {
    using std::make_tuple;
    using std::priority_queue;
    using std::string;
    using std::vector;

    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Digit5xGrid {
      public:
        Digit5xGrid(std::ifstream &input) {
            std::string s;
            if (!(input >> s)) {
                throw std::runtime_error{"malformed_input"};
            }

            auto base_tile = vector<uint8_t>{};

            // Assuming all rows are same size
            tile_w = s.size();
            parse_row(base_tile, s);

            auto rows = 1;
            while (input >> s) {
                parse_row(base_tile, s);
                ++rows;
            }
            tile_h = rows;

            // Make a 5x grid
            w = tile_w * 5;
            h = tile_h * 5;
            grid.resize(w * h);

            for (size_t y = 0; y < h; ++y) {
                for (size_t x = 0; x < w; ++x) {
                    // Top-left tile
                    if (x < tile_w && y < tile_h) {
                        set(x, y, base_tile[x + y * tile_w]);
                        continue;
                    }

                    // Use tile above
                    if (x < tile_w) {
                        const size_t source_y = (y % tile_h) + (tile_h * ((y / tile_h) - 1));
                        uint8_t value = at(x, source_y) + 1;
                        value = value > 9 ? 1 : value;
                        set(x, y, value);
                        continue;
                    }

                    // Any other tile we use the source value from left tile
                    const size_t source_x = (x % tile_w) + (tile_w * ((x / tile_w) - 1));
                    uint8_t value = at(source_x, y) + 1;
                    value = value > 9 ? 1 : value;
                    set(x, y, value);
                    continue;
                }
            }
        }

        size_t width() const { return w; }
        size_t height() const { return h; }
        size_t tile_width() const { return tile_w; }
        size_t tile_height() const { return tile_h; }
        uint8_t at(size_t x, size_t y) const { return grid[flat_array_idx(x, y)]; }
        uint8_t set(size_t x, size_t y, uint8_t value) { return grid[flat_array_idx(x, y)] = value; }
        size_t flat_array_idx(size_t x, size_t y) const { return x + y * w; }

      private:
        size_t w;
        size_t h;
        size_t tile_w;
        size_t tile_h;
        std::vector<uint8_t> grid;

        static void parse_row(vector<uint8_t> &grid, const std::string &s) {
            for (auto c : s) {
                grid.emplace_back(c - '0');
            }
        }
    };

    class Solver : public Base {
        using Base::Solver;

        struct Point {
            size_t x, y;
        };

        Answers solve(std::ifstream &input) const override {
            const auto risk_grid = Digit5xGrid{input};
            if (risk_grid.width() == 0 || risk_grid.height() == 0) {
                throw Error{"malformed_input"};
            }

            return Answers{
                Answer{"Lowest total risk of any path from the top left to the bottom right",
                       shortest_path_total_distance(risk_grid,
                                                    Point{},
                                                    Point{risk_grid.tile_width() - 1, risk_grid.tile_height() - 1},
                                                    risk_grid.tile_width(),
                                                    risk_grid.tile_height())},
                Answer{"Lowest total risk of any path from the top left to the bottom right on 5x grid",
                       shortest_path_total_distance(risk_grid,
                                                    Point{},
                                                    Point{risk_grid.width() - 1, risk_grid.height() - 1},
                                                    risk_grid.width(),
                                                    risk_grid.height())}};
        };

      private:
        struct Node {
            size_t x, y;
            unsigned long long distance;

            Node(size_t x = 0, size_t y = 0, unsigned long long distance = 0) : x(x), y(y), distance(distance) {}
        };

        // Run Djikstra's to find shortest path
        static unsigned long long shortest_path_total_distance(
            const Digit5xGrid &distances, Point start, Point destination, size_t width, size_t height) {
            auto flat_index = [=](size_t x, size_t y) { return x + y * width; };

            auto visited = vector<bool>(width * height, false);
            auto distance_tracker =
                vector<unsigned long long>(width * height, std::numeric_limits<unsigned long long>::max());
            distance_tracker[flat_index(start.x, start.y)] = 0;

            auto cmp = [](const Node &lhs, const Node &rhs) {
                return -static_cast<long long>(lhs.distance) < -static_cast<long long>(rhs.distance);
            };
            auto to_visit = priority_queue<Node, std::vector<Node>, decltype(cmp)>(cmp);
            to_visit.emplace(start.x, start.y, 0);

            auto handle_neighbor = [&](const Node &current, size_t neighbor_x, size_t neighbor_y) {
                const size_t neighbor_flat_idx = flat_index(neighbor_x, neighbor_y);

                if (neighbor_x >= width || neighbor_y >= height || visited[neighbor_flat_idx]) {
                    return;
                }

                const auto distance_through_current = current.distance + distances.at(neighbor_x, neighbor_y);
                const auto previous_distance = distance_tracker[neighbor_flat_idx];
                const auto distance = distance_tracker[neighbor_flat_idx] =
                    std::min(distance_through_current, previous_distance);
                to_visit.emplace(neighbor_x, neighbor_y, distance);
            };

            while (!to_visit.empty()) {
                const auto node = to_visit.top();
                to_visit.pop();

                const size_t flat_idx = flat_index(node.x, node.y);

                if (visited[flat_idx]) {
                    continue;
                }

                if (node.x == destination.x && node.y == destination.y) {
                    break;
                }

                // Mark this node visited
                visited[flat_idx] = true;

                // Compute new distances to our neighbors
                if (node.x > 0) {
                    handle_neighbor(node, node.x - 1, node.y); // west
                }
                if (node.y > 0) {
                    handle_neighbor(node, node.x, node.y - 1); // north
                }
                handle_neighbor(node, node.x + 1, node.y); // east
                handle_neighbor(node, node.x, node.y + 1); // south
            }

            return distance_tracker[flat_index(destination.x, destination.y)];
        }
    };
} // namespace Year2021::Day15

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day15::Solver{FILENAME}.print_answers();
    return 0;
}
