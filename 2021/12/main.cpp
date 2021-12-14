#include "../../solver.h"

#include <unordered_map>

namespace Year2021::Day12 {
    using std::string;
    using std::unordered_map;
    using std::vector;

    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Solver : public Base {
        using Base::Solver;

        Answers solve(std::ifstream &input) const override {
            const auto graph = read_and_construct_graph(input);

            return Answers{Answer{"Paths that only visit small caves at most once", total_path_count<false>(graph)},
                           Answer{"Paths that can visit one small cave twice", total_path_count<true>(graph)}};
        };

      private:
        struct Vertex {
            size_t idx;
            vector<size_t> edges;
            bool repeatable : 1, end : 1;
        };

        struct Graph {
            vector<Vertex> vertices;
            size_t start_idx;
        };

        template <bool allow_twice> static unsigned long long total_path_count(const Graph &graph) {
            auto total_paths = (unsigned long long){};
            auto disallowed = vector<bool>(graph.vertices.size(), false);

            auto find_path_count = [&](auto &&find_path_count, size_t vertex_idx, bool used_small_cave_twice) {
                const auto &vertex = graph.vertices[vertex_idx];

                if (vertex.end) {
                    ++total_paths;
                    return;
                }

                for (auto edge_idx : vertex.edges) {
                    const auto &edge = graph.vertices[edge_idx];
                    if (disallowed[edge_idx]) {
                        // If we are allowing one specific cave to be visited twice, we'll allow it here.
                        if (allow_twice && !used_small_cave_twice) {
                            find_path_count(find_path_count, edge_idx, true);
                        }
                        continue;
                    }

                    if (!edge.repeatable) {
                        disallowed[edge_idx] = true;
                    }

                    find_path_count(find_path_count, edge_idx, used_small_cave_twice);

                    disallowed[edge_idx] = false;
                }
            };

            find_path_count(find_path_count, graph.start_idx, false);

            return total_paths;
        }

        static unordered_map<string, vector<string>> read_simple_graph(std::ifstream &input) {
            auto graph = unordered_map<string, vector<string>>{};

            const auto add_edge = [&](const std::string &from, const std::string &to) {
                if (from == "end" || to == "start") {
                    graph.emplace(from, vector<string>{});
                    return;
                }

                graph[from].emplace_back(to);
            };

            string s;
            while (input >> s) {
                const auto delim_pos = s.find('-');
                const auto vertex_0 = s.substr(0, delim_pos);
                const auto vertex_1 = s.substr(delim_pos + 1);

                add_edge(vertex_0, vertex_1);
                add_edge(vertex_1, vertex_0);
            }

            return graph;
        }

        static Graph read_and_construct_graph(std::ifstream &input) {
            const auto simple_graph = read_simple_graph(input);

            const auto id_map = [&]() {
                auto out = unordered_map<string, size_t>{};
                auto i = size_t{0};
                for (const auto &v : simple_graph) {
                    out[v.first] = i++;
                }
                return out;
            }();

            auto graph = vector<Vertex>{};

            {
                for (size_t i = 0; i < simple_graph.size(); ++i) {
                    graph.emplace_back();
                }

                for (const auto &v : simple_graph) {
                    const auto idx = id_map.at(v.first);
                    auto &vertex = graph[idx];
                    vertex.idx = idx;
                    vertex.repeatable =
                        std::all_of(v.first.begin(), v.first.end(), [](char c) { return !std::islower(c); });
                    vertex.end = v.first == "end";

                    for (const auto &e : v.second) {
                        vertex.edges.emplace_back(id_map.at(e));
                    }
                }
            }

            return Graph{graph, id_map.at("start")};
        }
    };
} // namespace Year2021::Day12

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day12::Solver{FILENAME}.print_answers();
    return 0;
}
