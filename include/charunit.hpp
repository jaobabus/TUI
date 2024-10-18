#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "asciistyles.hpp"
#include "termcolor.hpp"


enum class EscapeSeqArgument : int
{
    // Usually argument in [0, 255]
    NumberStart = 0,
    // End of sequence
    Eos = -1,
    Eof = Eos,                 //< Unix-like constant :)
    // 255 numbers is reserved

    /// Ascii symbol: [-0x1000000:-257] encoded like Escape
    /// 0x1000000 - because utf-8
    AsciiModStart = -0x1000000,
    Escape = -0x1000000 + (int)'\e', //< '\e' symbol
};

inline constexpr EscapeSeqArgument encode_number(int number) {
    return static_cast<EscapeSeqArgument>(number + static_cast<int>(EscapeSeqArgument::NumberStart));
}

inline constexpr EscapeSeqArgument encode_char(wchar_t chr) {
    return static_cast<EscapeSeqArgument>(static_cast<uint32_t>(chr) + static_cast<int>(EscapeSeqArgument::AsciiModStart));
}

struct AixTermEscapeCost
{
    int bytes_len;
    int arg_count;
};

class EscapeSequence : public std::vector<EscapeSeqArgument>
{
public:
    EscapeSequence() {}

public:
    size_t encode_all(char* buffer, size_t buf_size) const {
        size_t pos = 0;
        for (const auto& arg : *this) {
            if ((int)arg == (int)EscapeSeqArgument::Eos) {
                break;
            }
            if (pos >= buf_size) {
                throw std::runtime_error("Buffer overflow while encoding escape sequence");
            }

            if ((int)arg >= (int)EscapeSeqArgument::NumberStart) {
                // Encode numbers as ASCII digits
                // Very slow!
                int number = static_cast<int>(arg);
                std::string num_str = std::to_string(number);
                if (pos + num_str.size() >= buf_size) {
                    throw std::runtime_error("Buffer overflow while encoding number");
                }
                for (char c : num_str) {
                    buffer[pos++] = c;
                }
            } else if ((int)arg > (int)EscapeSeqArgument::AsciiModStart
                       && (int)arg < -256) { //< arg is negative, -2..-256 is reserved
                // Encode ASCII character
                wchar_t chr = static_cast<wchar_t>((int)arg - (int)EscapeSeqArgument::AsciiModStart);
                if (chr <= 0x7F) {
                    buffer[pos++] = static_cast<char>(chr); // ASCII character
                } else {
                    throw std::runtime_error("Unsupported character encoding");
                }
            } else {
                throw std::runtime_error("Unsupported escape sequence argument");
            }
        }
        return pos;
    }
};

struct CharUnit {
    wchar_t character;               //< The character to be displayed
    struct {
        CharColor256Bg background;   //< Background color
        CharColor256Fg foreground;   //< Foreground color
        uint16_t styles;             //< Bitfield representing styles, where each bit corresponds to a style ID
    } meta;

    static CharUnit none;
};

/// Default state of console char
inline CharUnit CharUnit::none { L'\0', CharColor256Bg{0}, CharColor256Fg{15}, 0 };


inline constexpr EscapeSeqArgument encode_color_4bit(CharColor256Fg color) {
    if (color.term_color < 8)
        return encode_number(static_cast<int>(AsciiStyleCode::Foregrount4StartLower) + color.term_color);
    else
        return encode_number(static_cast<int>(AsciiStyleCode::Foregrount4StartUpper) + color.term_color);
}


inline constexpr EscapeSeqArgument encode_color_4bit(CharColor256Bg color) {
    if (color.term_color < 8)
        return encode_number(static_cast<int>(AsciiStyleCode::Background4StartLower) + color.term_color);
    else
        return encode_number(static_cast<int>(AsciiStyleCode::Background4StartUpper) + color.term_color);
}


/** to_aixterm_escape
 *
 * @param[out] sequence Output sequence
 * @param[in]  unit     Unit to encode
 * @param[in]  prev     Prev unit
 */
void to_aixterm_escape(EscapeSequence& sequence, const CharUnit& unit, const CharUnit& prev = CharUnit::none);

AixTermEscapeCost aixterm_escape_cost(const CharUnit& unit, const CharUnit& prev = CharUnit::none, bool fast = false);
