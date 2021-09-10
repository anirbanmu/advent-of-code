#include "../../solver.h"

#include <fstream>
#include <sstream>
#include <string>

#include <openssl/md5.h>

namespace Year2015::Day4 {
    using Base = ::Common::Solver<unsigned long>;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            std::string s;
            if (!(input >> s)) {
                throw Error{"bad_input"};
            }

            struct {
                struct {
                    unsigned long value;
                    bool found;
                } lowest_five_zeroes, lowest_six_zeroes;
            } answers{};

            unsigned char md5sum[MD5_DIGEST_LENGTH]{};
            std::ostringstream current{};
            for (unsigned long i = 0; !answers.lowest_five_zeroes.found || !answers.lowest_six_zeroes.found;
                 ++i, current.str("")) {
                current << s << i;
                const auto current_string{current.str()};
                const auto buf = current_string.c_str();
                MD5(reinterpret_cast<const unsigned char *>(buf), current_string.size(), md5sum);

                if (md5sum[0] == 0 && md5sum[1] == 0) {
                    // Five zeroes
                    if (!answers.lowest_five_zeroes.found && (md5sum[2] & 0xF0) == 0) {
                        answers.lowest_five_zeroes = {i, true};
                    }

                    // Six zeroes
                    if (!answers.lowest_six_zeroes.found && md5sum[2] == 0) {
                        answers.lowest_six_zeroes = {i, true};
                    }
                }
            }

            return Base::Answers{
                Base::Answer{"Lowest number to produce 5 zeroes MD5", answers.lowest_five_zeroes.value},
                Base::Answer{"Lowest number to produce 6 zeroes MD5", answers.lowest_six_zeroes.value}};
        }
    };
} // namespace Year2015::Day4

const auto FILENAME{"input.txt"};

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day4::Solver{FILENAME}.print_answers();
    return 0;
}
