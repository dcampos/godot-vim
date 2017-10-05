#include "util.h"
#include "ustring.h"

bool _is_space(CharType c) {
    return (c == ' ' || c == '\t');
}

bool _is_text_char(CharType c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

bool _is_symbol(CharType c) {
    return c != '_' && ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~'));
}

bool _is_char(CharType c) {

    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool _is_number(CharType c) {
    return (c >= '0' && c <= '9');
}

bool _is_hex_symbol(CharType c) {
    return ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

bool _is_pair_right_symbol(CharType c) {
    return c == '"' ||
           c == '\'' ||
           c == ')' ||
           c == ']' ||
           c == '}';
}

bool _is_pair_left_symbol(CharType c) {
    return c == '"' ||
           c == '\'' ||
           c == '(' ||
           c == '[' ||
           c == '{';
}

bool _is_pair_symbol(CharType c) {
    return _is_pair_left_symbol(c) || _is_pair_right_symbol(c);
}

CharType _get_right_pair_symbol(CharType c) {
    if (c == '"')
        return '"';
    if (c == '\'')
        return '\'';
    if (c == '(')
        return ')';
    if (c == '[')
        return ']';
    if (c == '{')
        return '}';
    return 0;
}

CharType _get_left_pair_symbol(CharType c) {
    if (c == '"')
        return '"';
    if (c == '\'')
        return '\'';
    if (c == ')')
        return '(';
    if (c == ']')
        return '[';
    if (c == '}')
        return '{';
    return 0;
}

bool _is_same_word(CharType a, CharType b) {
    return ((_is_text_char(a) && _is_text_char(b))
                || (_is_symbol(a) && b == a));
}

bool _is_end_of_word(int col, String line) {
    if (line.length() == 0) return false;

    CharType char1 = line[col];

    if ((!_is_space(char1)) && col >= line.length() - 1) return true;

    CharType char2 = line[col + 1];

    return ((_is_text_char(char1) && !_is_text_char(char2)) || (_is_symbol(char1) && char1 != char2));
}

bool _is_end_of_big_word(int col, String line) {
    if (line.length() == 0) return false;

    CharType char1 = line[col];

    if ((!_is_space(char1)) && col >= line.length() - 1) return true;

    CharType char2 = line[col + 1];

    return ((_is_text_char(char1) || _is_symbol(char1)) && _is_space(char2));
}


bool _is_beginning_of_word(int col, String line) {
    if (line.length() == 0) return true;
    if (_is_space(line[col])) return false;
    if (col == 0) return true;

    CharType char1 = line[col - 1];
    CharType char2 = line[col];

    return (!_is_text_char(char1) && _is_text_char(char2)) || (_is_symbol(char2) && char2 != char1);
}

bool _is_beginning_of_big_word(int col, String line) {
    if (line.length() == 0) return false;
    if (_is_space(line[col])) return false;
    if (col == 0) return true;

    CharType char1 = line[col - 1];
    CharType char2 = line[col];

    return _is_space(char1) && (_is_text_char(char2) || _is_symbol(char2));
}

int _find_first_non_blank(String line) {

    for (int i = 0; i < line.length(); i++) {
        CharType c = line[i];
        if (!_is_space(c)) {
                return i;
        }
    }

    return 0;
}
