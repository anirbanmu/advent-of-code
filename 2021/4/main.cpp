#include "../../solver.h"

#include <bitset>
#include <charconv>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace Year2021::Day4 {
    using BingoCardNumber = unsigned long long;
    using Base = ::Common::Solver<BingoCardNumber>;
    const size_t BINGO_CARD_ROWS = 5;
    const size_t BINGO_CARD_COLUMNS = 5;
    const size_t BINGO_CARD_TOTAL_NUMBERS = BINGO_CARD_ROWS * BINGO_CARD_COLUMNS;
    using BingoCardFillState = std::bitset<BINGO_CARD_TOTAL_NUMBERS>;
    using BingoCardNumbers = std::array<BingoCardNumber, BINGO_CARD_TOTAL_NUMBERS>;

    // We store fill state & numbers as [row0-number1, row0-number2,..., row1-number1... row4-number4]
    class BingoCard {
      public:
        BingoCard(const BingoCardNumbers &numbers) : fill_state{}, numbers{numbers} {}

        bool is_win() const {
            return std::any_of(winning_fill_states.begin(), winning_fill_states.end(), [&](const auto &winning_state) {
                return (winning_state & fill_state) == winning_state;
            });
        }

        const BingoCardNumbers all_numbers() const { return numbers; }

        unsigned long long sum_of_all_unfilled() const {
            unsigned long long sum = 0;
            for (size_t i = 0; i < numbers.size(); ++i) {
                if (fill_state[i]) {
                    continue;
                }
                sum += numbers[i];
            }
            return sum;
        }

        bool fill(size_t index) {
            fill_state.set(index);
            return is_win();
        }

      private:
        BingoCardFillState fill_state;
        const BingoCardNumbers numbers;

        static const std::array<BingoCardFillState, 10> winning_fill_states;
    };

    const std::array<BingoCardFillState, 10> BingoCard::winning_fill_states =
        std::array<BingoCardFillState, 10>{// Row wins
                                           BingoCardFillState("1111100000000000000000000"),
                                           BingoCardFillState("0000011111000000000000000"),
                                           BingoCardFillState("0000000000111110000000000"),
                                           BingoCardFillState("0000000000000001111100000"),
                                           BingoCardFillState("0000000000000000000011111"),
                                           // Column wins
                                           BingoCardFillState("1000010000100001000010000"),
                                           BingoCardFillState("0100001000010000100001000"),
                                           BingoCardFillState("0010000100001000010000100"),
                                           BingoCardFillState("0001000010000100001000010"),
                                           BingoCardFillState("0000100001000010000100001")};

    class Solver : public Base {
        using Base::Solver;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            const auto numbers = read_numbers_picked(input);
            auto cards = read_cards(input);
            auto number_to_cards_map = make_number_to_cards_map(cards);

            const auto scores = find_first_and_last_win_scores(numbers, number_to_cards_map, cards.size());
            return Base::Answers{Base::Answer{"Final score for first winning board", scores.first},
                                 Base::Answer{"Final score for last winning board", scores.second}};
        }

      private:
        static std::pair<unsigned long long, unsigned long long> find_first_and_last_win_scores(
            const std::vector<BingoCardNumber> &numbers,
            std::unordered_multimap<BingoCardNumber, std::tuple<BingoCard *, size_t>> &number_to_cards_map,
            size_t card_count) {
            unsigned long long first_win_score, last_win_score;

            auto winning_cards = std::unordered_set<const BingoCard *>{};

            for (const auto n : numbers) {
                auto cards_with_number = number_to_cards_map.equal_range(n);
                for (auto it = cards_with_number.first; it != cards_with_number.second; ++it) {
                    auto card_ptr = std::get<0>(it->second);

                    if (winning_cards.find(card_ptr) != winning_cards.end()) {
                        // This card already won, if we check it again, we would count it as another win.
                        continue;
                    }

                    if (card_ptr->fill(std::get<1>(it->second))) {
                        winning_cards.emplace(card_ptr);
                        const auto winning_cards_count = winning_cards.size();
                        if (winning_cards_count == 1) {
                            first_win_score = card_ptr->sum_of_all_unfilled() * n;
                        }
                        if (winning_cards_count == card_count) {
                            last_win_score = card_ptr->sum_of_all_unfilled() * n;
                        }
                    }
                }
            }

            return std::make_pair(first_win_score, last_win_score);
        }

        static std::vector<BingoCard> read_cards(std::ifstream &input) {
            auto cards = std::vector<BingoCard>{};

            auto current_card_numbers = BingoCardNumbers{};
            size_t current_read_count = 0;
            while (++current_read_count, input >> current_card_numbers[current_read_count - 1]) {
                if (current_read_count == BINGO_CARD_TOTAL_NUMBERS) {
                    current_read_count = 0;
                    cards.emplace_back(current_card_numbers);
                }
            }

            return cards;
        }

        static std::vector<BingoCardNumber> read_numbers_picked(std::ifstream &input) {
            std::string str;
            if (!(input >> str)) {
                throw Error{"malformed_input"};
            }

            const auto str_view = std::string_view{str};

            auto numbers = std::vector<BingoCardNumber>{};

            size_t pos = 0;
            size_t delim_pos = 0;
            while ((delim_pos = str_view.find(',', pos)) != std::string::npos) {
                numbers.emplace_back(str_to_ull(str_view.substr(pos, delim_pos - pos)));
                pos = delim_pos + 1;
            }
            numbers.emplace_back(str_to_ull(str_view.substr(pos)));

            return numbers;
        }

        static unsigned long long str_to_ull(const std::string_view &str) {
            unsigned long long number = 0;
            auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), number);
            if (err != std::errc{} || ptr != str.data() + str.size()) {
                throw Error{"malformed_input"};
            }
            return number;
        }

        static std::unordered_multimap<BingoCardNumber, std::tuple<BingoCard *, size_t>>
        make_number_to_cards_map(std::vector<BingoCard> &cards) {
            auto number_to_cards_map = std::unordered_multimap<BingoCardNumber, std::tuple<BingoCard *, size_t>>{};

            for (auto &card : cards) {
                const auto &card_numbers = card.all_numbers();
                for (size_t i = 0; i < card_numbers.size(); ++i) {
                    number_to_cards_map.emplace(std::piecewise_construct,
                                                std::forward_as_tuple(card_numbers[i]),
                                                std::forward_as_tuple(&card, i));
                }
            }

            return number_to_cards_map;
        }
    };
} // namespace Year2021::Day4

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day4::Solver{FILENAME}.print_answers();
    return 0;
}
