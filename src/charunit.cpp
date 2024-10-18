#include "charunit.hpp"

#include <bit>



AixTermEscapeCost aixterm_escape_cost(const CharUnit& unit, const CharUnit& prev, bool fast)
{
    AixTermEscapeCost cost = {0, 0};

    { // Solve arg_count
        // Escape character always costs 3 arguments: "\e[" and 'm'
        cost.arg_count += 3;

        // Calculate the cost of styles
        uint16_t changed_styles = unit.meta.styles ^ prev.meta.styles;
        cost.arg_count += std::popcount(changed_styles);

        // Calculate the cost of background color
        if (unit.meta.background.term_color != prev.meta.background.term_color) {
            cost.arg_count += (unit.meta.background.term_color < 16 ? 1 : 6); // 4-bit color is 1 arg, 8-bit is 6 args ("\e[38;5;<color>m")
        }

        // Calculate the cost of foreground color
        if (unit.meta.foreground.term_color != prev.meta.foreground.term_color) {
            cost.arg_count += (unit.meta.foreground.term_color < 16 ? 1 : 6); // 4-bit color is 1 arg, 8-bit is 6 args
        }

        // Add cost for the character itself
        cost.arg_count += 1;
    }

    if (fast or 1) // slow mode not needed
    {
        /// bytes_by_arg_coeff
        /// Let's assume that the distribution of numbers is uniform
        constexpr auto average_number_len = ((9 - -1) * 1 + (99 - 9) * 2 + (255 - 99) * 3) / 256.0;
        /// For simplicity, let's assume that the probability of a 4-bit color appearing is 16 / 256, and for an 8-bit color is (256 - 16) / 256
        constexpr auto average_4color_len = ((9 - -1) * 1 + (16 - 9) * 2) / 16.0 * (16 / 256.0);
        constexpr auto average_8color_len = ((99 - 16) * 2 + (255 - 99) * 3) / (256 - 16) * ((256 - 16) / 256.0);
        /// The probability of style appearance is also uniformly distributed for simplicity, and we have 7 groups
        constexpr auto average_style_len = average_number_len * ((7 * 64) / 128); // Total number of set bits divided by the total number of bits (approximately 3.5, logically)
        /// 3 ('\e[m') + 4bit_len + 1 (;) + 8bit_len + 1 + 8bit_chance * 3 ('\e[m') + style_len
        constexpr auto bytes_by_arg_coeff = (average_4color_len + 1) * 2 / 1 + (average_8color_len + 1) * 2 / 6 + ((256 - 16) / 256.0) * 3 + average_style_len / 14; // ~6.8
        cost.bytes_len = 3 + cost.arg_count * (int)(bytes_by_arg_coeff + 0.5); // To increase performance
    }

    return cost;
}


void to_aixterm_escape(EscapeSequence& sequence, const CharUnit& unit, const CharUnit& _prev)
{
    CharUnit prev = _prev;
    {
        auto saved_size = sequence.size();
        // Add the escape character
        sequence.push_back(EscapeSeqArgument::Escape);
        sequence.push_back(encode_char(L'['));

        if (unit.meta.background.term_color != prev.meta.background.term_color && unit.meta.background.term_color < 16) {
            if (unit.meta.background.term_color == 0) {
                sequence.push_back(encode_char(L'0'));
                sequence.push_back(encode_char(L';'));
            }
            else {
                sequence.push_back(encode_color_4bit(unit.meta.background));
                sequence.push_back(encode_char(L';'));
            }
            prev.meta = CharUnit::none.meta;
        }

        // Encode styles using XOR to determine changes
        uint16_t changed = unit.meta.styles ^ prev.meta.styles;
        for (uint8_t i = 0; changed; ++i, changed >>= 1) {
            if (changed & 1) {
                const auto& style = ascii_styles[i];
                if (unit.meta.styles & (1 << i)) {
                    sequence.push_back(encode_number(static_cast<int>(style.enable)));
                } else {
                    sequence.push_back(encode_number(static_cast<int>(style.disable)));
                }
                sequence.push_back(encode_char(L';'));
            }
        }

        // Encode color in the previous scope if possible
        if (unit.meta.foreground.term_color != prev.meta.foreground.term_color && unit.meta.foreground.term_color < 16) {
            sequence.push_back(encode_color_4bit(unit.meta.foreground));
            sequence.push_back(encode_char(L';'));
        }

        sequence.back() = encode_char(L'm');
        if (saved_size == (sequence.size() - 2))
            sequence.erase(sequence.end() - 2, sequence.end());
    }

    // Encode background color if it changed
    if (unit.meta.background.term_color != prev.meta.background.term_color && unit.meta.background.term_color >= 16) {
        sequence.push_back(EscapeSeqArgument::Escape);
        sequence.push_back(encode_char(L'['));
        sequence.push_back(encode_number(static_cast<int>(AsciiStyleCode::Background8Or24)));
        sequence.push_back(encode_char(L';'));
        sequence.push_back(encode_number(5));
        sequence.push_back(encode_char(L';'));
        sequence.push_back(encode_number(unit.meta.background.term_color));
        sequence.push_back(encode_char(L'm'));
    }

    // Encode foreground color if it changed
    if (unit.meta.foreground.term_color != prev.meta.foreground.term_color && unit.meta.foreground.term_color >= 16) {
        sequence.push_back(EscapeSeqArgument::Escape);
        sequence.push_back(encode_char(L'['));
        sequence.push_back(encode_number(static_cast<int>(AsciiStyleCode::Foreground8Or24)));
        sequence.push_back(encode_char(L';'));
        sequence.push_back(encode_number(5));
        sequence.push_back(encode_char(L';'));
        sequence.push_back(encode_number(unit.meta.foreground.term_color));
        sequence.push_back(encode_char(L'm'));
    }

    // Add the character itself
    sequence.push_back(encode_char(unit.character));
}
