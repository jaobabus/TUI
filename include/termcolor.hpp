#pragma once

#include <cstdint>


struct CharColor256 {
    uint8_t term_color; // Represents 4-bit or 8-bit terminal color

    // Constructors
    constexpr CharColor256() : term_color(0) {}
    constexpr explicit CharColor256(uint8_t color) : term_color(color) {}
    constexpr CharColor256(const CharColor256& other) : term_color(other.term_color) {}

    // Assignment operators
    CharColor256& operator=(const CharColor256& other) {
        term_color = other.term_color;
        return *this;
    }
    CharColor256& operator=(uint8_t color) {
        term_color = color;
        return *this;
    }
};


// Foreground Color Class
struct CharColor256Fg : public CharColor256 {
    using CharColor256::CharColor256;
    using CharColor256::operator=;
    CharColor256Fg() : CharColor256(15) {}

    bool is_default() const noexcept {
        return term_color == 15;
    }
};


// Background Color Class
struct CharColor256Bg : public CharColor256 {
    using CharColor256::CharColor256;
    using CharColor256::operator=;
    CharColor256Bg() : CharColor256(0) {}

    bool is_default() const noexcept {
        return term_color == 0;
    }
};


// Color constants
constexpr inline CharColor256 color_black{0};
constexpr inline CharColor256 color_red{1};
constexpr inline CharColor256 color_green{2};
constexpr inline CharColor256 color_yellow{3};
constexpr inline CharColor256 color_blue{4};
constexpr inline CharColor256 color_magenta{5};
constexpr inline CharColor256 color_cyan{6};
constexpr inline CharColor256 color_white{7};
constexpr inline CharColor256 color_bright_black{8};
constexpr inline CharColor256 color_bright_red{9};
constexpr inline CharColor256 color_bright_green{10};
constexpr inline CharColor256 color_bright_yellow{11};
constexpr inline CharColor256 color_bright_blue{12};
constexpr inline CharColor256 color_bright_magenta{13};
constexpr inline CharColor256 color_bright_cyan{14};
constexpr inline CharColor256 color_bright_white{15};
