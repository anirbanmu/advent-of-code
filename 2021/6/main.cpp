#include "../../solver.h"
#include "../../utils.h"

#include <array>
#include <numeric>
#include <unordered_map>

namespace Year2021::Day6 {
    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    const size_t MAX_FISH_BIO_TIMER = 8;

    // This will track how many fish are at a certain timer age (aka the array index)
    using FishBioTimerTracker = std::array<unsigned long long, MAX_FISH_BIO_TIMER + 1>;

    const size_t FISH_BIO_TIMER_CHILD_AT_BIRTH = 8;
    const size_t FISH_BIO_TIMER_PARENT_AT_BIRTH = 6;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Answers solve(std::ifstream &input) const override {
            auto tracker = read_initial_state(input);

            auto fish_count_at_certain_days = std::unordered_map<unsigned, unsigned long long>{{80, 0}, {256, 0}};

            const auto num_days_to_simulate = size_t{256};
            for (size_t i = 0; i < num_days_to_simulate; ++i) {
                auto new_tracker = FishBioTimerTracker();

                for (size_t timer = 0; timer < tracker.size(); ++timer) {
                    if (timer == 0) {
                        new_tracker[FISH_BIO_TIMER_PARENT_AT_BIRTH] += tracker[timer]; // Reset these parent fish
                        new_tracker[FISH_BIO_TIMER_CHILD_AT_BIRTH] += tracker[timer];  // Produce new fish
                        continue;
                    }

                    new_tracker[timer - 1] += tracker[timer];
                }

                tracker = new_tracker;

                if (fish_count_at_certain_days.find(i + 1) != fish_count_at_certain_days.end()) {
                    fish_count_at_certain_days[i + 1] =
                        std::accumulate(tracker.begin(), tracker.end(), (unsigned long long)0);
                }
            }

            return Answers{Answer{"Fish count after 80 days", fish_count_at_certain_days[80]},
                           Answer{"Fish count after 256 days", fish_count_at_certain_days[256]}};
        }

      private:
        static FishBioTimerTracker read_initial_state(std::ifstream &input) {
            auto tracker = FishBioTimerTracker{};

            std::string str;
            if (!(input >> str)) {
                throw Error{"malformed_input"};
            }

            const auto str_view = std::string_view{str};

            size_t pos = 0;
            size_t delim_pos = 0;
            while ((delim_pos = str_view.find(',', pos)) != std::string::npos) {
                const auto age = Utils::str_to_ull(str_view.substr(pos, delim_pos - pos));
                if (age > MAX_FISH_BIO_TIMER) {
                    throw Error{"malformed_input"};
                }
                ++tracker[age];
                pos = delim_pos + 1;
            }
            const auto age = Utils::str_to_ull(str_view.substr(pos, delim_pos - pos));
            if (age > MAX_FISH_BIO_TIMER) {
                throw Error{"malformed_input"};
            }
            ++tracker[age];

            return tracker;
        }
    };
} // namespace Year2021::Day6

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day6::Solver{FILENAME}.print_answers();
    return 0;
}
