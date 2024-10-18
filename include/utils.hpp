#ifndef UTILS_HPP
#define UTILS_HPP


#include <map>
#include <stdint.h>
#include <string>


template<typename T>
struct Vector2
{
    Vector2()
        : Vector2(0, 0) {}
    explicit
    Vector2(T x)
        : Vector2(x, x) {}
    Vector2(T x, T y)
        : x(x), y(y) {}
    template<typename Ty>
    Vector2(Vector2<Ty> oth)
        : Vector2(oth.x, oth.y) {}

    T x, y;
};


using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned>;
using Vector2f = Vector2<float>;


#define DECLARE_VECTOR2_OPERATOR(op) \
template<typename T> \
inline \
Vector2<T> operator op(Vector2<T> left, Vector2<T> right) { \
    return Vector2<T>{ left.x op right.x, left.y op right.y }; \
}

#define DECLARE_VECTOR2_LOGIC_OPERATOR(op) \
template<typename T> \
inline \
bool operator op(Vector2<T> left, Vector2<T> right) { \
    return left.x op right.x and left.y op right.y ; \
}

DECLARE_VECTOR2_OPERATOR(+)
DECLARE_VECTOR2_OPERATOR(-)
DECLARE_VECTOR2_OPERATOR(/)
DECLARE_VECTOR2_OPERATOR(*)

DECLARE_VECTOR2_LOGIC_OPERATOR(<)
DECLARE_VECTOR2_LOGIC_OPERATOR(>)
DECLARE_VECTOR2_LOGIC_OPERATOR(<=)
DECLARE_VECTOR2_LOGIC_OPERATOR(>=)
DECLARE_VECTOR2_LOGIC_OPERATOR(==)
DECLARE_VECTOR2_LOGIC_OPERATOR(!=)



inline std::string escape_string(std::string_view sv)
{
    std::string out;
    static const std::map<char, const char*> escapes = {
            {'\n', "\\n"},
            {'\r', "\\r"},
            {'\t', "\\t"},
            {'\e', "\\e"},
            {'\b', "\\b"},
            {'\a', "\\a"},
            {'\f', "\\f"}
    };
    for (auto c : sv) {
        if ((uint8_t)c < ' ' and escapes.count(c))
            out += escapes.at(c);
        else if (c < '\x7F')
            out += c;
        else {
            out += "\\x";
            out += "0123456789ABCDEF"[c & 0xF];
            out += "0123456789ABCDEF"[c >> 4];
        }
    }
    return out;
}


#endif // UTILS_HPP
