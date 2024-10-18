#include <textmap.hpp>



std::string rjump(Vector2i off)
{
    std::string result;
    if (off.x)
        result += "\033[" + std::to_string(abs(off.x)) + "CD"[off.x < 0];
    if (off.y)
        result += "\033[" + std::to_string(abs(off.y)) + "BA"[off.y < 0];
    return result;
}

std::string jump(Vector2i off)
{
    std::string result;
    if (off.x)
        result += "\033[" + std::to_string(abs(off.x)) + "CD"[off.x < 0];
    if (off.y)
        result += "\033[" + std::to_string(abs(off.y)) + "BA"[off.y < 0];
    return result;
}

void to_aixterm_escape_jump(EscapeSequence& seq, int x, int y)
{
    // Generate escape sequence to move the cursor to the desired position (x, y)
    seq.push_back(encode_char(L'\e')); // ESC character
    seq.push_back(encode_char(L'['));
    seq.push_back(encode_number(y + 1)); // Rows are 1-based
    seq.push_back(encode_char(L';'));
    seq.push_back(encode_number(x + 1)); // Columns are 1-based
    seq.push_back(encode_char(L'H')); // Command to move cursor
}

std::string encode_utf8(wchar_t ch)
{
    if (ch > (wchar_t)127)
        return {L'?'};
    return {(char)ch};
}

std::string STDIOTextArea::print(std::string&& old) const
{
    std::string result = std::move(old);
    auto [bytes_len, arg_cnt] = aixterm_escape_cost(CharUnit{ L'\0', { { CharColor256Bg{0} }, CharColor256Fg{64}, 3 } });
    EscapeSequence sequence; // Estimate average length
    sequence.reserve(arg_cnt * size.x * size.y);
    if (result.size() == 0)
        result.reserve(bytes_len * size.x * size.y * 1.15); // Reserve memory to avoid frequent reallocations

    Vector2u last_pos{0, 0};
    CharUnit last_unit = CharUnit::none; // Track the last rendered character unit
    sequence.push_back(encode_char(L'\e'));
    sequence.push_back(encode_char(L'['));
    sequence.push_back(encode_char(L'0'));
    sequence.push_back(encode_char(L'm'));

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            CharUnit unit = _map[y * size.x + x];

            // Handle empty characters with defined background
            if (unit.character == L'\0') {
                unit.character = L' '; // Replace with a space for rendering
            }

            // Handle character position jump if needed
            if (x != last_pos.x || y != last_pos.y) {
                to_aixterm_escape_jump(sequence, x, y);
            }

            // Call to_aixterm_escape for handling style/color changes
            to_aixterm_escape(sequence, unit, last_unit);
            last_unit = unit;

            // Update last position
            last_pos = {x + 1, y};
        }
    }

    // Encode all accumulated escape sequences at the end
    result.resize(result.capacity());
    result.resize(sequence.encode_all(result.data(), result.size()));

    return result;
}

TextMap TextMap::create_from(const std::string& tex)
{
    TextMap map{{240, 120}};
    ::draw(&map, {0, 0}, tex);
    return map.strip();
}

TextMap TextMap::strip() const
{
    Vector2i left_min{0x7F7F7FFF, 0x7F7F7FFF}, right_max{-1, -1};
    bool found = false;
    for (unsigned y = 0; y < size.y; y++)
        for (unsigned x = 0; x < size.x; x++)
        {
            if (get_char({x, y}).character != L'\0') {
                if ((int)x > right_max.x)
                    right_max.x = x;
                if ((int)y > right_max.y)
                    right_max.y = y;
                if (x < left_min.x)
                    left_min.x = x;
                if (y < left_min.y)
                    left_min.y = y;
                found = true;
            }
        }
    if (found) {
        TextMap result{(right_max + Vector2i{1, 1}) - left_min};
        draw(&result);
        return result;
    }
    return *this;
}
