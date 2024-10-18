#pragma once

#include <cstdint>
#include "asciiescape.hpp"


struct AsciiStyle {
    const char* name;
    uint8_t group_id;
    uint8_t id;
    AsciiStyleCode enable;
    AsciiStyleCode disable;
};

// Groups
constexpr inline uint8_t ascii_group_bold = 0;
constexpr inline uint8_t ascii_group_ideogram = 1;
constexpr inline uint8_t ascii_group_underline = 2;
constexpr inline uint8_t ascii_group_italic = 3;
constexpr inline uint8_t ascii_group_blink = 4;
constexpr inline uint8_t ascii_group_reverse = 5;
constexpr inline uint8_t ascii_group_strike = 6;

// Style constants
constexpr inline uint8_t bold_id = 0;
constexpr inline AsciiStyle bold { "bold", ascii_group_bold, bold_id, AsciiStyleCode::Bold, AsciiStyleCode::NotFaint };

constexpr inline uint8_t faint_id = 1;
constexpr inline AsciiStyle faint { "faint", ascii_group_bold, faint_id, AsciiStyleCode::Faint, AsciiStyleCode::NotFaint };

constexpr inline uint8_t italic_id = 2;
constexpr inline AsciiStyle italic { "italic", ascii_group_italic, italic_id, AsciiStyleCode::Italic, AsciiStyleCode::NotItalic };

constexpr inline uint8_t underline_id = 3;
constexpr inline AsciiStyle underline { "underline", ascii_group_underline, underline_id, AsciiStyleCode::Underline, AsciiStyleCode::NotUnderline };

constexpr inline uint8_t double_underline_id = 4;
constexpr inline AsciiStyle double_underline { "double-underline", ascii_group_underline, double_underline_id, AsciiStyleCode::DoubleUnderlineOrNotBold, AsciiStyleCode::NotUnderline };

constexpr inline uint8_t blink_id = 5;
constexpr inline AsciiStyle blink { "blink", ascii_group_blink, blink_id, AsciiStyleCode::Blink, AsciiStyleCode::NotBlinking };

constexpr inline uint8_t rapid_blink_id = 6;
constexpr inline AsciiStyle rapid_blink { "rapid-blink", ascii_group_blink, rapid_blink_id, AsciiStyleCode::RapidBlink, AsciiStyleCode::NotBlinking };

constexpr inline uint8_t reverse_id = 7;
constexpr inline AsciiStyle reverse { "reverse", ascii_group_reverse, reverse_id, AsciiStyleCode::Reverse, AsciiStyleCode::NotReversed };

constexpr inline uint8_t conceal_id = 8;
constexpr inline AsciiStyle conceal { "conceal", ascii_group_reverse, conceal_id, AsciiStyleCode::ConcealOrHide, AsciiStyleCode::Reveral };

constexpr inline uint8_t strike_id = 9;
constexpr inline AsciiStyle strike { "strike", ascii_group_strike, strike_id, AsciiStyleCode::Strike, AsciiStyleCode::NotStrike };

constexpr inline uint8_t ideogram_underline_id = 10;
constexpr inline AsciiStyle ideogram_underline { "ideogram-underline", ascii_group_ideogram, ideogram_underline_id, AsciiStyleCode::IdeogramUnderline, AsciiStyleCode::NotIdeogram };

constexpr inline uint8_t ideogram_double_underline_id = 11;
constexpr inline AsciiStyle ideogram_double_underline { "ideogram-double-underline", ascii_group_ideogram, ideogram_double_underline_id, AsciiStyleCode::IdeogramDoubleUnderline, AsciiStyleCode::NotIdeogram };

constexpr inline uint8_t ideogram_overline_id = 12;
constexpr inline AsciiStyle ideogram_overline { "ideogram-overline", ascii_group_ideogram, ideogram_overline_id, AsciiStyleCode::IdeogramOverline, AsciiStyleCode::NotIdeogram };

constexpr inline uint8_t ideogram_double_overline_id = 13;
constexpr inline AsciiStyle ideogram_double_overline { "ideogram-double-overline", ascii_group_ideogram, ideogram_double_overline_id, AsciiStyleCode::IdeogramDoubleOverline, AsciiStyleCode::NotIdeogram };

constexpr inline uint8_t ideogram_stress_marking_id = 14;
constexpr inline AsciiStyle ideogram_stress_marking { "ideogram-stress-marking", ascii_group_ideogram, ideogram_stress_marking_id, AsciiStyleCode::IdeogramSterssMarking, AsciiStyleCode::NotIdeogram };


// Array of all styles
constexpr inline AsciiStyle ascii_styles[] = {
    [bold_id] = bold,
    [faint_id] = faint,
    [italic_id] = italic,
    [underline_id] = underline,
    [double_underline_id] = double_underline,
    [blink_id] = blink,
    [rapid_blink_id] = rapid_blink,
    [reverse_id] = reverse,
    [conceal_id] = conceal,
    [strike_id] = strike,
    [ideogram_underline_id] = ideogram_underline,
    [ideogram_double_underline_id] = ideogram_double_underline,
    [ideogram_overline_id] = ideogram_overline,
    [ideogram_double_overline_id] = ideogram_double_overline,
    [ideogram_stress_marking_id] = ideogram_stress_marking
};
