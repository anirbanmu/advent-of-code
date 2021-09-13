#include "../../solver.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <variant>

namespace Year2015::Day7 {
    using Base = ::Common::Solver<long>;

    class Solver : public Base {
        using Base::Solver;

        using signal_value_t = uint16_t;
        using wire_identifier_t = std::string;
        using operand_t = std::variant<wire_identifier_t, signal_value_t>;

        enum class UnaryOperator { NOT };
        enum class BinaryOperator { AND, LSHIFT, OR, RSHIFT };

        using UnaryOperation = struct {
            UnaryOperator op;
            operand_t operand;
        };

        using BinaryOperation = struct {
            BinaryOperator op;
            operand_t left_operand;
            operand_t right_operand;
        };

        // A wire's value can be either a raw value, another wire identifier or the result of a unary/binary op.
        using WireValue = std::variant<signal_value_t, wire_identifier_t, UnaryOperation, BinaryOperation>;
        using WireValueMap = std::unordered_map<wire_identifier_t, WireValue>;

      protected:
        Base::Answers solve(std::ifstream &input) const override {
            auto wires = WireValueMap{};

            for (std::string line; std::getline(input, line);) {
                const auto parsed = parse_line(line);
                wires[parsed.wire_identifier] = parsed.wire_value;
            }

            struct {
                signal_value_t part1, part2;
            } answers{};

            auto memoized_signals = std::unordered_map<wire_identifier_t, signal_value_t>{};
            answers.part1 = get_value_for(wires, memoized_signals, "a");

            memoized_signals.clear();
            wires["b"] = answers.part1;
            answers.part2 = get_value_for(wires, memoized_signals, "a");

            return Base::Answers{
                Base::Answer{"Wire \"a\" value", answers.part1},
                Base::Answer{"Wire \"a\" value", answers.part2},
            };
        }

      private:
        using ParsedLine = struct {
            wire_identifier_t wire_identifier;
            WireValue wire_value;
        };

        static ParsedLine parse_line(const std::string &line) {
            auto sstream = std::istringstream(line);

            std::string throwaway, wire_identifier;

            // Try longest to shortest to prevent short lines accidentally partially matching longer expected lines.

            // Try for binary op
            {
                std::string left_operand, binary_op, right_operand;
                if (sstream >> left_operand >> binary_op >> right_operand >> throwaway >> wire_identifier) {
                    return ParsedLine{wire_identifier,
                                      BinaryOperation{parse_binary_operator(binary_op),
                                                      parse_operand(left_operand),
                                                      parse_operand(right_operand)}};
                }
                sstream.clear();
                sstream.seekg(0);
            }

            // Try for unary op
            {
                std::string unary_op, operand;
                if (sstream >> unary_op >> operand >> throwaway >> wire_identifier) {
                    return ParsedLine{wire_identifier,
                                      UnaryOperation{parse_unary_operator(unary_op), parse_operand(operand)}};
                }
                sstream.clear();
                sstream.seekg(0);
            }

            // Try for just number
            {
                signal_value_t value;
                if (sstream >> value >> throwaway >> wire_identifier) {
                    return ParsedLine{wire_identifier, value};
                }
                sstream.clear();
                sstream.seekg(0);
            }

            // Try for just another wire
            {
                std::string value;
                if (sstream >> value >> throwaway >> wire_identifier) {
                    return ParsedLine{wire_identifier, value};
                }
            }

            throw Error{"malformed_error"};
        }

        static operand_t parse_operand(const std::string &operand) {
            if (std::all_of(operand.begin(), operand.end(), [](unsigned char c) { return std::isdigit(c); })) {
                return operand_t{static_cast<signal_value_t>(std::stoul(operand))};
            }

            return operand_t{operand};
        }

        static const std::unordered_map<std::string, BinaryOperator> BINARY_OPERATOR_STRING_MAP;
        static BinaryOperator parse_binary_operator(const std::string &op) { return BINARY_OPERATOR_STRING_MAP.at(op); }

        static const std::unordered_map<std::string, UnaryOperator> UNARY_OPERATOR_STRING_MAP;
        static UnaryOperator parse_unary_operator(const std::string &op) { return UNARY_OPERATOR_STRING_MAP.at(op); }

        static const std::unordered_map<UnaryOperator, std::function<signal_value_t(signal_value_t)>> UNARY_ACTIONS;
        static const std::unordered_map<BinaryOperator, std::function<signal_value_t(signal_value_t, signal_value_t)>>
            BINARY_ACTIONS;

        // Meant to be recursively called to get the value for a wire
        static signal_value_t get_value_for(const WireValueMap &wire_value_map,
                                            std::unordered_map<wire_identifier_t, signal_value_t> &memoized,
                                            const operand_t &wire_identifier_or_value) {
            // Nothing to do if we get asked for the value of a raw value
            if (std::holds_alternative<signal_value_t>(wire_identifier_or_value)) {
                return std::get<signal_value_t>(wire_identifier_or_value);
            }

            // It's an identifier
            const auto &wire_identifier = std::get<wire_identifier_t>(wire_identifier_or_value);
            const auto cached = memoized.find(wire_identifier);
            if (cached != memoized.end()) {
                return cached->second;
            }

            const auto &wire_value = wire_value_map.at(wire_identifier);

            // Found the value
            if (std::holds_alternative<signal_value_t>(wire_value)) {
                return memoized[wire_identifier] = std::get<signal_value_t>(wire_value);
            }

            // Just redirect
            if (std::holds_alternative<wire_identifier_t>(wire_value)) {
                return memoized[wire_identifier] =
                           get_value_for(wire_value_map, memoized, std::get<wire_identifier_t>(wire_value));
            }

            // Execute unary op for the value for the operand
            if (std::holds_alternative<UnaryOperation>(wire_value)) {
                auto const &unary_operation = std::get<UnaryOperation>(wire_value);
                auto const &action = UNARY_ACTIONS.at(unary_operation.op);
                return memoized[wire_identifier] =
                           action(get_value_for(wire_value_map, memoized, unary_operation.operand));
            }

            // Must be binary op. Execute binary op for the value for the operands
            auto const &binary_operation = std::get<BinaryOperation>(wire_value);
            auto const &action = BINARY_ACTIONS.at(binary_operation.op);
            return memoized[wire_identifier] =
                       action(get_value_for(wire_value_map, memoized, binary_operation.left_operand),
                              get_value_for(wire_value_map, memoized, binary_operation.right_operand));
        }
    };

    const std::unordered_map<std::string, Solver::BinaryOperator> Solver::BINARY_OPERATOR_STRING_MAP =
        std::unordered_map<std::string, BinaryOperator>{{"AND", Solver::BinaryOperator::AND},
                                                        {"OR", Solver::BinaryOperator::OR},
                                                        {"LSHIFT", Solver::BinaryOperator::LSHIFT},
                                                        {"RSHIFT", Solver::BinaryOperator::RSHIFT}};

    const std::unordered_map<std::string, Solver::UnaryOperator> Solver::UNARY_OPERATOR_STRING_MAP =
        std::unordered_map<std::string, UnaryOperator>{{"NOT", Solver::UnaryOperator::NOT}};

    const std::unordered_map<Solver::UnaryOperator, std::function<Solver::signal_value_t(Solver::signal_value_t)>>
        Solver::UNARY_ACTIONS =
            std::unordered_map<Solver::UnaryOperator, std::function<Solver::signal_value_t(Solver::signal_value_t)>>{
                {Solver::UnaryOperator::NOT, [](Solver::signal_value_t v) { return ~v; }}};

    const std::unordered_map<Solver::BinaryOperator,
                             std::function<Solver::signal_value_t(Solver::signal_value_t, Solver::signal_value_t)>>
        Solver::BINARY_ACTIONS =
            std::unordered_map<Solver::BinaryOperator, std::function<signal_value_t(signal_value_t, signal_value_t)>>{
                {BinaryOperator::AND, [](signal_value_t a, signal_value_t b) { return a & b; }},
                {BinaryOperator::OR, [](signal_value_t a, signal_value_t b) { return a | b; }},
                {BinaryOperator::LSHIFT, [](signal_value_t a, signal_value_t b) { return a << b; }},
                {BinaryOperator::RSHIFT, [](signal_value_t a, signal_value_t b) { return a >> b; }}};
} // namespace Year2015::Day7

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2015::Day7::Solver{FILENAME}.print_answers();
    return 0;
}
