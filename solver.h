#ifndef _SOLVER_H_
#define _SOLVER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Common {
    template <typename T> class Solver {
      public:
        using Answer = struct {
            std::string descriptor;
            T value;
        };
        using Answers = std::vector<Answer>;
        using Error = std::runtime_error;

        Solver(const char *const input_file_path) : input_file_path{input_file_path} {}

        Answers get_answers() {
            auto input = std::ifstream{input_file_path, std::ios::in};
            if (!input.is_open()) {
                throw Error{"file_open_failed"};
            }

            return solve(input);
        }

        void print_answers() {
            const auto answers = get_answers();
            const auto size = answers.size();
            for (size_t i = 0; i < size; ++i) {
                const auto &answer = answers[i];
                std::cout << "[Part " << i + 1 << "] " << answer.descriptor << ": " << answer.value << std::endl;
            }
        }

      protected:
        virtual Answers solve(std::ifstream &input) const = 0;

      private:
        std::string input_file_path;
    };
} // namespace Common

#endif
