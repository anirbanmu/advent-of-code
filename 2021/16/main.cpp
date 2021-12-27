#include "../../solver.h"

#include <deque>
#include <numeric>
#include <unordered_map>
#include <variant>

namespace Year2021::Day16 {
    using std::array;
    using std::deque;
    using std::unordered_map;
    using std::vector;

    using Base = ::Common::Solver<unsigned long long>;
    using Answers = Base::Answers;
    using Answer = Base::Answer;

    class Packet {
      public:
        enum class Type { literal, sum, product, minimum, maximum, greater_than, less_than, equal_to };

        Packet(uint8_t version, Packet::Type type, unsigned long long value)
            : version(version), type(type), inner_data(value) {}

        Packet(uint8_t version, Packet::Type type, const vector<Packet> &packets)
            : version(version), type(type), inner_data(packets) {}

        unsigned long long sum_of_all_version_numbers() const {
            auto sum = (unsigned long long){};

            auto to_process = deque<const Packet *>{this};
            while (!to_process.empty()) {
                const auto &current_packet = *to_process.front();
                sum += current_packet.version;

                if (std::holds_alternative<vector<Packet>>(current_packet.inner_data)) {
                    for (const auto &sub_packet : std::get<vector<Packet>>(current_packet.inner_data)) {
                        to_process.emplace_back(&sub_packet);
                    }
                }

                to_process.pop_front();
            }

            return sum;
        }

        unsigned long long evaluate() const {
            switch (type) {
            case Type::literal:
                return std::get<unsigned long long>(inner_data);
            case Type::sum:
                return std::accumulate(sub_packets().begin(),
                                       sub_packets().end(),
                                       (unsigned long long)0,
                                       [](auto acc, const Packet &p) { return acc + p.evaluate(); });
            case Type::product:
                return std::accumulate(sub_packets().begin(),
                                       sub_packets().end(),
                                       (unsigned long long)1,
                                       [](auto acc, const Packet &p) { return acc * p.evaluate(); });
            case Type::minimum:
                return std::accumulate(sub_packets().begin(),
                                       sub_packets().end(),
                                       std::numeric_limits<unsigned long long>::max(),
                                       [](auto acc, const Packet &p) { return std::min(acc, p.evaluate()); });
            case Type::maximum:
                return std::accumulate(sub_packets().begin(),
                                       sub_packets().end(),
                                       std::numeric_limits<unsigned long long>::min(),
                                       [](auto acc, const Packet &p) { return std::max(acc, p.evaluate()); });
            case Type::greater_than:
                return sub_packets()[0].evaluate() > sub_packets()[1].evaluate() ? 1 : 0;
            case Type::less_than:
                return sub_packets()[0].evaluate() < sub_packets()[1].evaluate() ? 1 : 0;
            case Type::equal_to:
                return sub_packets()[0].evaluate() == sub_packets()[1].evaluate() ? 1 : 0;
            }
        }

      private:
        const vector<Packet> &sub_packets() const { return std::get<vector<Packet>>(inner_data); }

        const uint8_t version;
        const Type type;
        const std::variant<unsigned long long, vector<Packet>> inner_data;
    };

    class PacketReader {
      public:
        PacketReader(const vector<bool> &bits) : bits(bits) {}

        template <typename T> struct ReadResult {
            T data;
            size_t end_index; // Past end of read
        };

        ReadResult<Packet> read_packet(size_t index) const {
            const auto version_data = read_version(index);
            const auto type_data = read_type(version_data.end_index);

            if (type_data.data == Packet::Type::literal) {
                return read_literal_packet(version_data.data, type_data.end_index);
            }

            return read_operator_packet(version_data.data, type_data.data, type_data.end_index);
        }

      private:
        const vector<bool> &bits;

        // leading bit indicates whether this is last hex digit (4 bits) or not
        // 0 - last
        // 1 - not last
        ReadResult<Packet> read_literal_packet(uint8_t version, size_t index) const {
            auto literal = (unsigned long long){};

            bool done;
            while (true) {
                done = !bits[index++];

                auto i = size_t{};
                for (i = index; i < index + 4; ++i) {
                    literal = (literal << 1) | (unsigned long long)bits[i];
                }
                index = i;

                if (done) {
                    break;
                }
            }

            return ReadResult<Packet>{Packet{version, Packet::Type::literal, literal}, index};
        }

        ReadResult<Packet> read_operator_packet(uint8_t version, Packet::Type type, size_t index) const {
            auto sub_packets = vector<Packet>{};

            const uint8_t length_type_id = bits[index++];

            // Means we need to read N bits, which may contain unknown # of packets
            if (length_type_id == 0) {
                const auto bit_length_data = read_n_bits_as_integer<15>(index);
                const auto bit_length_of_sub_packets = bit_length_data.data;
                index = bit_length_data.end_index;

                const auto target_index = index + bit_length_of_sub_packets;
                while (index < target_index) {
                    auto [packet, end_index] = read_packet(index);
                    index = end_index;
                    sub_packets.emplace_back(packet);
                }
            }

            // Mean we need to read N packets
            if (length_type_id == 1) {
                const auto sub_packet_count_data = read_n_bits_as_integer<11>(index);
                const auto sub_packet_count = sub_packet_count_data.data;
                index = sub_packet_count_data.end_index;

                for (size_t i = 0; i < sub_packet_count; ++i) {
                    auto [packet, end_index] = read_packet(index);
                    index = end_index;
                    sub_packets.emplace_back(packet);
                }
            }

            return ReadResult<Packet>{Packet{version, type, sub_packets}, index};
        }

        // 3 bits
        ReadResult<uint8_t> read_version(size_t index) const {
            const auto [version, end_idx] = read_n_bits_as_integer<3>(index);
            return ReadResult<uint8_t>{static_cast<uint8_t>(version), end_idx};
        }

        // 3 bits
        static const unordered_map<uint8_t, Packet::Type> RAW_PACKET_TYPE_TO_ENUM;
        ReadResult<Packet::Type> read_type(size_t index) const {
            const auto [type, end_idx] = read_n_bits_as_integer<3>(index);
            return ReadResult<Packet::Type>{RAW_PACKET_TYPE_TO_ENUM.at(type), end_idx};
        }

        template <size_t bit_count> ReadResult<unsigned long long> read_n_bits_as_integer(size_t index) const {
            auto out = ReadResult<unsigned long long>{};
            for (out.end_index = index; out.end_index < index + bit_count; ++out.end_index) {
                out.data = (out.data << 1) | (unsigned long long)bits[out.end_index];
            }
            return out;
        }
    };

    const unordered_map<uint8_t, Packet::Type> PacketReader::RAW_PACKET_TYPE_TO_ENUM =
        unordered_map<uint8_t, Packet::Type>{{0, Packet::Type::sum},
                                             {1, Packet::Type::product},
                                             {2, Packet::Type::minimum},
                                             {3, Packet::Type::maximum},
                                             {4, Packet::Type::literal},
                                             {5, Packet::Type::greater_than},
                                             {6, Packet::Type::less_than},
                                             {7, Packet::Type::equal_to}};

    class Solver : public Base {
        using Base::Solver;

        Answers solve(std::ifstream &input) const override {
            const auto bits = read_binary(input);

            const auto packet = PacketReader{bits}.read_packet(0).data;

            return Answers{Answer{"Sum of version numbers in all packets", packet.sum_of_all_version_numbers()},
                           Answer{"Evaluated value of top-level packet", packet.evaluate()}};
        };

      private:
        static const unordered_map<char, array<uint8_t, 4>> HEX_DIGITS_TO_BINARY_BITS;

        static vector<bool> read_binary(std::ifstream &input) {
            auto out = vector<bool>{};

            auto c = char{};
            while (input >> c) {
                for (auto bit : HEX_DIGITS_TO_BINARY_BITS.at(c)) {
                    out.emplace_back(bit);
                }
            }

            return out;
        }
    };

    const unordered_map<char, array<uint8_t, 4>> Solver::HEX_DIGITS_TO_BINARY_BITS =
        unordered_map<char, array<uint8_t, 4>>{{'0', array<uint8_t, 4>{0, 0, 0, 0}},
                                               {'1', array<uint8_t, 4>{0, 0, 0, 1}},
                                               {'2', array<uint8_t, 4>{0, 0, 1, 0}},
                                               {'3', array<uint8_t, 4>{0, 0, 1, 1}},
                                               {'4', array<uint8_t, 4>{0, 1, 0, 0}},
                                               {'5', array<uint8_t, 4>{0, 1, 0, 1}},
                                               {'6', array<uint8_t, 4>{0, 1, 1, 0}},
                                               {'7', array<uint8_t, 4>{0, 1, 1, 1}},
                                               {'8', array<uint8_t, 4>{1, 0, 0, 0}},
                                               {'9', array<uint8_t, 4>{1, 0, 0, 1}},
                                               {'A', array<uint8_t, 4>{1, 0, 1, 0}},
                                               {'B', array<uint8_t, 4>{1, 0, 1, 1}},
                                               {'C', array<uint8_t, 4>{1, 1, 0, 0}},
                                               {'D', array<uint8_t, 4>{1, 1, 0, 1}},
                                               {'E', array<uint8_t, 4>{1, 1, 1, 0}},
                                               {'F', array<uint8_t, 4>{1, 1, 1, 1}}};
} // namespace Year2021::Day16

const auto FILENAME = "input.txt";

int main(__attribute__((unused)) int _argc, __attribute__((unused)) char **_argv) {
    Year2021::Day16::Solver{FILENAME}.print_answers();
    return 0;
}
