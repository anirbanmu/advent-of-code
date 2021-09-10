#include "../../solver.h"

#include <fstream>

namespace Year2015::Day2 {
    using Base = ::Common::Solver<long>;

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            auto surface_area{0};
            auto ribbon_length{0};

            int l, w, h;
            char delims[2];
            while (input >> l >> delims[0] >> w >> delims[1] >> h && delims[0] == 'x' && delims[1] == 'x') {
                const auto lw = l * w;
                const auto wh = w * h;
                const auto hl = h * l;

                // total surface area + min area of a surface
                surface_area += 2 * lw + 2 * wh + 2 * hl + std::min(std::min(lw, wh), hl);

                // volume + minumum perimeter
                ribbon_length += lw * h + std::min(std::min(2 * l + 2 * w, 2 * w + 2 * h), 2 * h + 2 * l);
            }

            return Base::Answers{Base::Answer{"Total area needed", surface_area},
                                 Base::Answer{"Total ribbon needed", ribbon_length}};
        }
    };
} // namespace Year2015::Day2

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day2::Solver{FILENAME}.print_answers();
    return 0;
}
