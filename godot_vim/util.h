#ifndef UTIL_H
#define UTIL_H

#include "ustring.h"

bool _is_space(CharType c);

bool _is_text_char(CharType c);

bool _is_symbol(CharType c);

bool _is_char(CharType c);

bool _is_number(CharType c);

bool _is_hex_symbol(CharType c);

bool _is_pair_right_symbol(CharType c);

bool _is_pair_left_symbol(CharType c);

bool _is_pair_symbol(CharType c);

CharType _get_right_pair_symbol(CharType c);

CharType _get_left_pair_symbol(CharType c);

bool _is_same_word(CharType a, CharType b);

bool _is_end_of_word(int col, String line);

bool _is_end_of_big_word(int col, String line);

bool _is_beginning_of_word(int col, String line);

bool _is_beginning_of_big_word(int col, String line);

int _find_first_non_blank(String line);

String _join_lines(Vector<String> lines);

#endif // UTIL_H
