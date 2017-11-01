#include "motion.h"
#include "util.h"
#include "godot_vim.h"

Motion::Pos Motion::_move_forward(Motion::matchFunction function, bool accept_empty) {
    int next_cc = vim->_cursor_get_column() + 1;
    int next_cl = vim->_cursor_get_line();

    String line_text = vim->_get_line(next_cl);

    for (int i = next_cl; i < vim->_get_line_count(); i++) {
        line_text = vim->_get_line(i);

        if (line_text.length() == 0 || next_cc >= line_text.length()) {
            if (line_text.length() == 0 && accept_empty && i > vim->_cursor_get_line()) return Pos(i, 0);
            next_cc = 0;
            continue;
        }

        for (int j = next_cc; j < line_text.length(); j++) {
            if ((function)(j, line_text)) {
                print_line("setting cursor to " + itos(j) + ", " + itos(i));
                return Pos(i, j);
            }
        }
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::_move_backward(Motion::matchFunction function, bool accept_empty) {
    int next_cc = vim->_cursor_get_column() - 1;
    int next_cl = vim->_cursor_get_line();

    String line_text = vim->_get_line(next_cl);

    bool needs_cc = false;

    for (int i = next_cl; i >= 0; i--) {
        line_text = vim->_get_line(i);

        if (needs_cc) next_cc = line_text.length() - 1;

        if (line_text.length() == 0) {
            if (accept_empty && i < vim->_cursor_get_line()) return Pos(i, 0);

            needs_cc = true;
            continue;
        }

        for (int j = next_cc; j >= 0; j--) {
            if ((function)(j, line_text)) {
                print_line("setting cursor to " + itos(j) + ", " + itos(i));
                vim->_cursor_set_line(i);
                vim->_cursor_set_column(j);
                return Pos(i, j);
            }
        }

        needs_cc = true;
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::_move_to_first_non_blank(int line) {
    String line_text = vim->_get_line(line);
    if (vim->_cursor_get_line() != line)
        vim->_cursor_set_line(line);
    for (int i = 0; i < line_text.length(); i++) {
        CharType c = line_text[i];
        if (!_is_space(c)) {
                vim->_cursor_set_column(i);
                return Pos(line, i);
        }
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::_move_by_columns(int cols) {
    int col = CLAMP(vim->_cursor_get_column() + cols, 0, vim->_get_current_line_length() - 1);
    return Pos(vim->_cursor_get_line(), col);
}

Motion::Pos Motion::_move_by_lines(int lines) {
    int line = CLAMP(vim->_cursor_get_line() + lines, 0, vim->_get_line_count() - 1);
    return Pos(line, vim->_cursor_get_column());
}

Motion::Pos Motion::_move_left() {
    return _move_by_columns(-1);
}

Motion::Pos Motion::_move_right() {
    return _move_by_columns(1);
}

Motion::Pos Motion::_move_down() {
    return _move_by_lines(1);
}

Motion::Pos Motion::_move_up() {
    return _move_by_lines(-1);
}

Motion::Pos Motion::_move_to_line_start() {
    return Pos(vim->_cursor_get_line(), 0);
}

Motion::Pos Motion::_move_to_line_end() {
    return Pos(vim->_cursor_get_line(), vim->_get_current_line_length() - 1);
}

Motion::Pos Motion::_move_word_right() {
    return _move_forward(&_is_beginning_of_word, true);
}

Motion::Pos Motion::_move_word_right_big() {
    return _move_forward(&_is_beginning_of_big_word, true);
}

Motion::Pos Motion::_move_word_end() {
    return _move_forward(&_is_end_of_word);
}

Motion::Pos Motion::_move_word_end_big() {
    return _move_forward(&_is_end_of_big_word);
}

Motion::Pos Motion::_move_word_end_backward() {
    return _move_backward(&_is_end_of_word);
}

Motion::Pos Motion::_move_word_end_big_backward() {
    return _move_backward(&_is_end_of_big_word);
}

Motion::Pos Motion::_move_word_beginning() {
    return _move_backward(&_is_beginning_of_word, true);
}

Motion::Pos Motion::_move_word_beginning_big() {
    return _move_backward(&_is_beginning_of_big_word, true);
}

Motion::Pos Motion::_move_paragraph_up() {
    int next_cl = vim->_cursor_get_line();

    bool has_text = false;

    for (int i = next_cl; i >= 0; i--) {
        String line_text = vim->_get_line(i);
        if (line_text.size() == 0) {
            if (has_text) {
                return Pos(i, 0);
            }
        } else {
            has_text = true;
        }
    }

    return Pos(0, 0);
}

Motion::Pos Motion::_move_paragraph_down() {
    int next_cl = vim->_cursor_get_line();

    bool has_text = false;

    for (int i = next_cl; i < vim->_get_line_count(); i++) {
        String line_text = vim->_get_line(i);
        if (line_text.size() == 0) {
            if (has_text) {
                return Pos(i, 0);
            }
        } else {
            has_text = true;
        }
    }

    int last_line = vim->_get_line_count() - 1;
    return Pos(last_line, vim->_get_line(last_line).length()  - 1);
}

Motion::Pos Motion::_move_to_matching_pair() {
    String line_text = vim->_get_line(vim->_cursor_get_line());
    int col = vim->_cursor_get_column();
    CharType c = line_text[col];
    if (_is_pair_left_symbol(c)) {
        CharType m = _get_right_pair_symbol(c);

        col++;
        int count = 1;

        for (int i = vim->_cursor_get_line(); i < vim->_get_line_count(); i++) {
            line_text = vim->_get_line(i);
            for (int j = col; j < line_text.length(); j++) {
                CharType cur = line_text[j];
                if (cur == c) {
                    count++;
                } else if (cur == m) {
                    count--;
                    if (count == 0) {
                        return Pos(i, j);
                    }
                }
            }
            col = 0;
        }
    } else if (_is_pair_right_symbol(c)) {
        CharType m = _get_left_pair_symbol(c);

        col--;
        int count = 1;
        bool needs_col = false;

        for (int i = vim->_cursor_get_line(); i >= 0; i--) {
            line_text = vim->_get_line(i);

            if (needs_col) col = line_text.length();

            for (int j = col; j >= 0; j--) {
                CharType cur = line_text[j];
                if (cur == c) {
                    count++;
                } else if (cur == m) {
                    count--;
                    if (count == 0) {
                        return Pos(i, j);
                    }
                }
            }
            needs_col = true;
        }
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::_move_to_first_non_blank() {
    return _move_to_first_non_blank(vim->_cursor_get_line());
}


Motion::Pos Motion::_move_to_beginning_of_last_line() {
    return _move_to_first_non_blank(vim->_get_line_count() - 1);
}

Motion::Pos Motion::_move_to_beginning_of_first_line() {
    return _move_to_first_non_blank(0);
}

Motion::Pos Motion::_move_to_beginning_of_previous_line() {
    if (vim->_cursor_get_line() > 0) {
        return _move_to_first_non_blank(vim->_cursor_get_line() - 1);
    }
    return _move_to_first_non_blank();
}

Motion::Pos Motion::_move_to_beginning_of_next_line() {
    if (vim->_cursor_get_line() < vim->_get_line_count() - 1) {
        _move_to_first_non_blank(vim->_cursor_get_line() + 1);
    }
    return _move_to_first_non_blank();
}

Motion::Pos Motion::_move_to_last_searched_char() {

}

Motion::Pos Motion::_move_to_last_searched_char_backward() {

}

Motion::Pos Motion::_move_to_column() {
    String line_text = vim->_get_current_line();
    if (vim->get_input_state().repeat_count - 1 < line_text.length()) {
        vim->_cursor_set_column(vim->get_input_state().repeat_count - 1);
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::search_word_under_cursor() {
    String word = vim->get_text_edit()->get_word_under_cursor();
    int r_l = vim->_cursor_get_line();
    int r_c = vim->_cursor_get_column();

    if (vim->get_text_edit()->search(word, 0, vim->_cursor_get_line(), vim->_cursor_get_column(), r_l, r_c)) {
        print_line("word found");
        return Pos(r_l, r_c);
    }
    return Pos(r_l, r_c);
}

Motion::Pos Motion::search_word_under_cursor_backward() {

}

Motion::Pos Motion::find_next() {
}

Motion::Pos Motion::find_previous() {
}

Motion::Pos Motion::find_char() {
    int n = find_in_line(vim->_get_current_line(), vim->get_input_state().input_char, vim->_cursor_get_column() + 1);

    if (n >= 0) {
        return Pos(vim->_cursor_get_line(), n);
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

Motion::Pos Motion::find_char_backward() {
    int n = find_in_line(vim->_get_current_line().substr(0, vim->_cursor_get_column()), vim->get_input_state().input_char);

    if (n >= 0) {
        return Pos(vim->_cursor_get_line(), n);
    }

    return Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
}

bool Motion::is_inclusive() {
    return flags & INCLUSIVE;
}

bool Motion::is_linewise() {
    return flags & LINEWISE;
}

bool Motion::is_text_object() {
    return flags & TEXT_OBJECT;
}

/**
 * Applies a motion taking repeat count into consideration.
 *
 * @brief Motion::apply
 * @return
 */
Motion::Pos Motion::apply() {
    Pos pos = Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
    int repeat = vim->get_input_state().repeat_count;
    if (repeat < 1) repeat = 1;
    for (int i = 0; i < repeat; i++) {
        vim->_cursor_set_line(pos.row);
        vim->_cursor_set_column(pos.col);
        pos = (this->*fcn)();
    }
    return pos;
}

Motion::Range Motion::get_range() {
    Pos cur_pos = Pos(vim->_cursor_get_line(), vim->_cursor_get_column());
    Pos pos = apply();
    Range range = pos < cur_pos ? Range(pos, cur_pos) : Range(cur_pos, pos);
    range.inclusive = is_inclusive();
    range.linewise = is_linewise();
    range.text_object = is_text_object();
    return range;
}

void Motion::run() {
    Pos pos = apply();
    vim->_cursor_set_line(pos.row);
    vim->_cursor_set_column(pos.col);
}

Motion::Motion(GodotVim *vim, int flags, motionFcn fcn, int cmd_flags) : Command(vim, cmd_flags), flags(flags), fcn(fcn) {
}

Motion * Motion::create_motion(GodotVim *vim, int flags, Motion::motionFcn fcn, int cmd_flags) {
    Motion *motion = memnew(Motion(vim, flags, fcn, cmd_flags));
    return motion;
}
