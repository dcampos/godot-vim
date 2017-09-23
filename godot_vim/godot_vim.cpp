#include "godot_vim.h"
#include "os/keyboard.h"
#include "os/input_event.h"
#include "editor/plugins/script_editor_plugin.h"

// Some utility functions copied over from text_edit.cpp

static const int a = 0;

static bool _is_text_char(CharType c) {

    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static bool _is_symbol(CharType c) {

    return c != '_' && ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~'));
}

static bool _is_char(CharType c) {

    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool _is_number(CharType c) {
    return (c >= '0' && c <= '9');
}

static bool _is_hex_symbol(CharType c) {
    return ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static bool _is_pair_right_symbol(CharType c) {
    return c == '"' ||
           c == '\'' ||
           c == ')' ||
           c == ']' ||
           c == '}';
}

static bool _is_pair_left_symbol(CharType c) {
    return c == '"' ||
           c == '\'' ||
           c == '(' ||
           c == '[' ||
           c == '{';
}

static bool _is_pair_symbol(CharType c) {
    return _is_pair_left_symbol(c) || _is_pair_right_symbol(c);
}

static CharType _get_right_pair_symbol(CharType c) {
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

static CharType _get_left_pair_symbol(CharType c) {
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

// End of copied functions

static bool _is_space(CharType c) {
    return (c == ' ' || c == '\t');
}

void GodotVim::_clear_state() {
    repeat_count = 0;
    input_string.clear();
    current_command = "";
}

void GodotVim::_editor_input(const InputEvent &p_event) {
    if (p_event.type == InputEvent::KEY) {
        const InputEventKey &kk = p_event.key;

        if (!kk.pressed) return;

        if (kk.scancode == KEY_ESCAPE) {
            if (vim_mode == INSERT) {
                _move_left();
            } else if (vim_mode == COMMAND) {
                repeat_count = 0;
                input_string.clear();
            }
            _set_vim_mode(COMMAND);
        } else if (vim_mode == COMMAND) {
            _parse_command_input(kk);
        }
    }
}

void GodotVim::_parse_command_input(const InputEventKey &p_event) {
    String character = _get_character(p_event);
    input_string += character;
    print_line(input_string);

    if (command_map.has(input_string)) {
        VimCommand cmd = command_map.find(input_string)->get();

        if (cmd.type == OPERATOR) {
            _clear_state();
            current_command = cmd.binding;
        } else if (cmd.type == MOTION && current_command != "") {
            VimCommand op_cmd = command_map.find(current_command)->get();
            if (op_cmd.type == OPERATOR) {
                int cl = _cursor_get_line();
                int cc = _cursor_get_column();
                cmdFunction func = cmd.function;
                if (repeat_count == 0) repeat_count++;
                (this->*func)();
                text_edit->select(cl, cc, _cursor_get_line(), _cursor_get_column()+1);
                cmdFunction op_func = op_cmd.function;
                (this->*op_func)();
                text_edit->deselect();
            }
            _clear_state();
        } else {
            print_line("running simple command!");
            cmdFunction func = cmd.function;
            if (repeat_count == 0) repeat_count++;
            (this->*func)();
            _clear_state();
        }
    } else if (input_string.is_numeric()) {
        int n = input_string.to_int();
        repeat_count = repeat_count * 10 + n;
        input_string.clear();
        print_line(itos(repeat_count));
    } else {

        bool contains = false;
        for (int i = 0; i < command_bindings.size(); ++i) {
            String binding = command_bindings.get(i);
            if (binding.begins_with(input_string)) contains = true;
        }

        if (!contains) input_string.clear();
    }
}

void GodotVim::_open_line(int line) {
    _move_by_lines(line);
    _cursor_set_column(_get_current_line_length());
    text_edit->insert_text_at_cursor("\n");
}

int GodotVim::_find_forward(const String &p_string) {
    int n = _get_current_line().find(p_string, _cursor_get_column());
    if (n >= 0) {
        _cursor_set_column(n);
    }
}

int GodotVim::_find_backward(const String &p_string) {
    int n = _get_current_line().substr(0, _cursor_get_column()).find(p_string);
    if (n >= 0) {
        _cursor_set_column(n);
    }
}

String GodotVim::_get_character(const InputEventKey &p_event) {
    String character = String::chr(wchar_t(p_event.unicode));
    return character;
}

void GodotVim::_editor_focus_enter() {
    _setup_editor();
}

void GodotVim::_cursor_set(int line, int col) {
    text_edit->cursor_set_line(line);
    text_edit->cursor_set_column(col);
}

void GodotVim::_cursor_set_line(int line) {
    text_edit->cursor_set_line(line);
}

void GodotVim::_cursor_set_column(int column) {
    text_edit->cursor_set_column(column);
}

int GodotVim::_cursor_get_line() {
    text_edit->cursor_get_line();
}

int GodotVim::_cursor_get_column() {
    text_edit->cursor_get_column();
}

int GodotVim::_get_current_line_length() {
    return text_edit->get_line(text_edit->cursor_get_line()).length();
}

String GodotVim::_get_current_line() {
    return _get_line(_cursor_get_line());
}

String GodotVim::_get_line(int line) {
    return text_edit->get_line(line);
}

int GodotVim::_get_line_count() {
    return text_edit->get_line_count();
}

void GodotVim::_move_left() {
    _move_by_columns(-repeat_count);
}

void GodotVim::_move_right() {
    _move_by_columns(repeat_count);
}

void GodotVim::_move_down() {
    _move_by_lines(repeat_count);
}

void GodotVim::_move_up() {
    _move_by_lines(-repeat_count);
}

void GodotVim::_move_to_line_start() {
    _cursor_set_column(0);
}

void GodotVim::_move_to_line_end() {
    _cursor_set_column(_get_current_line_length() - 1);
}

// Some util functions for word movement

static bool _is_same_word(CharType a, CharType b) {
    return ((_is_text_char(a) && _is_text_char(b))
                || (_is_symbol(a) && b == a));
}

static bool _is_end_of_word(int col, String line) {
    if (line.length() == 0) return false;

    CharType char1 = line[col];

    if ((!_is_space(char1)) && col >= line.length() - 1) return true;

    CharType char2 = line[col + 1];

    return ((_is_text_char(char1) && !_is_text_char(char2)) || (_is_symbol(char1) && char1 != char2));
}

static bool _is_end_of_big_word(int col, String line) {
    if (line.length() == 0) return false;

    CharType char1 = line[col];

    if ((!_is_space(char1)) && col >= line.length() - 1) return true;

    CharType char2 = line[col + 1];

    return ((_is_text_char(char1) || _is_symbol(char1)) && _is_space(char2));
}


static bool _is_beginning_of_word(int col, String line) {
    if (line.length() == 0) return true;
    if (_is_space(line[col])) return false;
    if (col == 0) return true;

    CharType char1 = line[col - 1];
    CharType char2 = line[col];

    return (!_is_text_char(char1) && _is_text_char(char2)) || (_is_symbol(char2) && char2 != char1);
}

static bool _is_beginning_of_big_word(int col, String line) {
    if (line.length() == 0) return false;
    if (_is_space(line[col])) return false;
    if (col == 0) return true;

    CharType char1 = line[col - 1];
    CharType char2 = line[col];

    return _is_space(char1) && (_is_text_char(char2) || _is_symbol(char2));
}

void GodotVim::_move_forward(GodotVim::matchFunction function) {
    int next_cc = _cursor_get_column() + 1;
    int next_cl = _cursor_get_line();

    String line_text = _get_line(next_cl);

    for (int i = next_cl; i < text_edit->get_line_count(); i++) {
        line_text = _get_line(i);

        if (line_text.length() == 0 || next_cc >= line_text.length()) {
            next_cc = 0;
            continue;
        }

        for (int j = next_cc; j < line_text.length(); j++) {
            if ((function)(j, line_text)) {
                print_line("setting cursor to " + itos(j) + ", " + itos(i));
                _cursor_set(i, j);
                return;
            }
        }
    }
}

void GodotVim::_move_backward(GodotVim::matchFunction function) {
    int next_cc = _cursor_get_column() - 1;
    int next_cl = _cursor_get_line();

    String line_text = _get_line(next_cl);

    bool needs_cc = false;

    for (int i = next_cl; i >= 0; i--) {
        line_text = _get_line(i);

        if (needs_cc) next_cc = line_text.length() - 1;

        if (line_text.length() == 0) {
            needs_cc = true;
            continue;
        }

        for (int j = next_cc; j >= 0; j--) {
            if ((function)(j, line_text)) {
                print_line("setting cursor to " + itos(j) + ", " + itos(i));
                _cursor_set_line(i);
                _cursor_set_column(j);
                return;
            }
        }

        needs_cc = true;
    }
}

void GodotVim::_move_to_first_non_blank(int line) {
    String line_text = _get_line(line);
    if (_cursor_get_line() != line)
        _cursor_set_line(line);
    for (int i = 0; i < line_text.length(); i++) {
        CharType c = line_text[i];
        if (!_is_space(c)) {
                _cursor_set_column(i);
                return;
        }
    }
}

void GodotVim::_move_word_right() {
    for (int i = 0; i < repeat_count; i++) {
        _move_forward(&_is_beginning_of_word);
    }
}

void GodotVim::_move_word_right_big() {
    for (int i = 0; i < repeat_count; i++) {
        _move_forward(&_is_beginning_of_big_word);
    }
}

void GodotVim::_move_word_end() {
    for (int i = 0; i < repeat_count; i++) {
        _move_forward(&_is_end_of_word);
    }
}

void GodotVim::_move_word_end_big() {
    for (int i = 0; i < repeat_count; i++) {
        _move_forward(&_is_end_of_big_word);
    }
}

void GodotVim::_move_word_end_backward() {
    for (int i = 0; i < repeat_count; i++) {
        _move_backward(&_is_end_of_word);
    }
}

void GodotVim::_move_word_end_big_backward() {
    for (int i = 0; i < repeat_count; i++) {
        _move_backward(&_is_end_of_big_word);
    }
}

void GodotVim::_move_word_beginning() {
    for (int i = 0; i < repeat_count; i++) {
        _move_backward(&_is_beginning_of_word);
    }
}

void GodotVim::_move_word_beginning_big() {
    for (int i = 0; i < repeat_count; i++) {
        _move_backward(&_is_beginning_of_big_word);
    }
}

void GodotVim::_move_paragraph_up() {
    int next_cl = _cursor_get_line();

    bool has_text = false;

    for (int i = next_cl; i >= 0; i--) {
        String line_text = _get_line(i);
        if (line_text.size() == 0) {
            if (has_text) {
                _cursor_set(i, 0);
                return;
            }
        } else {
            has_text = true;
        }
    }
}

void GodotVim::_move_paragraph_down() {
    int next_cl = _cursor_get_line();

    bool has_text = false;

    for (int i = next_cl; i < _get_line_count(); i++) {
        String line_text = _get_line(i);
        if (line_text.size() == 0) {
            if (has_text) {
                _cursor_set(i, 0);
                return;
            }
        } else {
            has_text = true;
        }
    }
}

void GodotVim::_move_to_matching_pair() {
    String line_text = _get_line(_cursor_get_line());
    int col = _cursor_get_column();
    CharType c = line_text[col];
    if (_is_pair_left_symbol(c)) {
        CharType m = _get_right_pair_symbol(c);

        col++;
        int count = 1;

        for (int i = _cursor_get_line(); i < _get_line_count(); i++) {
            line_text = _get_line(i);
            for (int j = col; j < line_text.length(); j++) {
                CharType cur = line_text[j];
                if (cur == c) {
                    count++;
                } else if (cur == m) {
                    count--;
                    if (count == 0) {
                        _cursor_set(i, j);
                        return;
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

        for (int i = _cursor_get_line(); i >= 0; i--) {
            line_text = _get_line(i);

            if (needs_col) col = line_text.length();

            for (int j = col; j >= 0; j--) {
                CharType cur = line_text[j];
                if (cur == c) {
                    count++;
                } else if (cur == m) {
                    count--;
                    if (count == 0) {
                        _cursor_set(i, j);
                        return;
                    }
                }
            }
            needs_col = true;
        }
    }
}

void GodotVim::_move_to_first_non_blank() {
    _move_to_first_non_blank(_cursor_get_line());
}

void GodotVim::_move_to_beginning_of_last_line() {
    _move_to_first_non_blank(_get_line_count() - 1);
}

void GodotVim::_move_to_beginning_of_first_line() {
    _move_to_first_non_blank(0);
}

void GodotVim::_move_to_beginning_of_previous_line() {
    if (_cursor_get_line() > 0) {
        _move_to_first_non_blank(_cursor_get_line() - 1);
    }
}

void GodotVim::_move_to_beginning_of_next_line() {
    if (_cursor_get_line() < _get_line_count() - 1) {
        _move_to_first_non_blank(_cursor_get_line() + 1);
    }
}

void GodotVim::_move_to_last_searched_char() {

}

void GodotVim::_move_to_last_searched_char_backward() {

}

void GodotVim::_move_to_column() {
    String line_text = _get_current_line();
    if (repeat_count - 1 < line_text.length()) {
        _cursor_set_column(repeat_count - 1);
    }
}

void GodotVim::_enter_insert_mode() {
    _set_vim_mode(INSERT);
}

void GodotVim::_enter_insert_mode_append() {
    _set_vim_mode(INSERT);
    _cursor_set_column(_cursor_get_column() + 1);
}

void GodotVim::_open_line_below() {
    _open_line(0);
    _set_vim_mode(INSERT);
}

void GodotVim::_open_line_above() {
    _open_line(-1);
    _set_vim_mode(INSERT);
}


void GodotVim::_delete_text() {
    text_edit->cut();
}

void GodotVim::_change_text() {
    text_edit->cut();
    _enter_insert_mode();
}

void GodotVim::_change_case(CaseOperation option) {
    int ssl = text_edit->get_selection_from_line();
    int sel = text_edit->get_selection_to_line();
    int ssc = text_edit->get_selection_from_column();
    int sec = text_edit->get_selection_to_column();

    text_edit->begin_complex_operation();
    for (int i = ssl; i <= sel; i++) {
        String line_text = _get_line(i);
        for (int j = (i == ssl ? ssc : 0); j < (i == sel ? sec : line_text.length()); j++) {
            switch (option) {

            case TO_LOWER:
                line_text[j] = String::char_lowercase(line_text[j]);
                break;

            case TO_UPPER:
                line_text[j] = String::char_uppercase(line_text[j]);
                break;

            default:
                if (line_text.substr(j, 1).match("[a-z]"))
                    line_text[j] =  String::char_uppercase(line_text[j]);
                else
                    line_text[j] =  String::char_lowercase(line_text[j]);
                break;
            }
        }
        text_edit->set_line(i, line_text);
    }
    text_edit->end_complex_operation();
}

void GodotVim::_change_case() {
    _change_case(INVERT);
}

void GodotVim::_change_to_lower_case() {
    _change_case(TO_LOWER);
}

void GodotVim::_change_to_upper_case() {
    _change_case(TO_UPPER);
}

void GodotVim::_indent_text() {
    int line = _cursor_get_line();
    String text_line = _get_line(line);
    text_line = '\t' + text_line;
    text_edit->set_line(line, text_line);
}

void GodotVim::_unindent_text() {
    int line = _cursor_get_line();
    String text_line = _get_line(line);
    if (text_line.begins_with("\t")) {
        text_line.erase(0, 1);
        text_edit->set_line(line, text_line);
    }
}

void GodotVim::_undo() {
    text_edit->undo();
}

void GodotVim::_redo() {
    text_edit->redo();
}

void GodotVim::_goto_next_tab() {
    TabContainer *tab_container = text_edit->get_parent()->get_parent()->cast_to<TabContainer>();
    tab_container->set_current_tab(tab_container->get_current_tab()+1);
}

void GodotVim::_goto_previous_tab() {

}

void GodotVim::_move_by_columns(int cols) {
    int col = CLAMP(text_edit->cursor_get_column() + cols, 0, _get_current_line_length() - 1);
    text_edit->cursor_set_column(col);
    virtual_column = col;
}

void GodotVim::_move_by_lines(int lines) {
    int line = CLAMP(text_edit->cursor_get_line() + lines, 0, text_edit->get_line_count() - 1);
    text_edit->cursor_set_line(line);
    //_check_virtual_column();
}

void GodotVim::_check_virtual_column() {
    int line_length = _get_current_line_length();
    if (text_edit->cursor_get_column() >= line_length) {
        text_edit->cursor_set_column(line_length - 1);
    } else if (virtual_column < line_length) {
        text_edit->cursor_set_column(virtual_column);
    }
}

void GodotVim::_set_vim_mode(VimMode mode) {
    vim_mode = mode;

    switch (mode) {

    case COMMAND:
        text_edit->cursor_set_block_mode(true);
        text_edit->set_readonly(true);
        break;

    case INSERT:
        text_edit->cursor_set_block_mode(false);
        text_edit->set_readonly(false);
        break;

    default:
        break;
    }
}

void GodotVim::_setup_editor() {
    if (vim_mode == COMMAND) {
        text_edit->cursor_set_block_mode(true);
        text_edit->set_readonly(true);
    }
}

Map<String, GodotVim::VimCommand> GodotVim::command_map;
Vector<String> GodotVim::command_bindings;

void GodotVim::_setup_command_map() {

    if (command_map.size() > 0) return;

    _create_command("l", &GodotVim::_move_right);
    _create_command("h", &GodotVim::_move_left);
    _create_command("j", &GodotVim::_move_down);
    _create_command("k", &GodotVim::_move_up);
    _create_command("0", &GodotVim::_move_to_line_start);
    _create_command("$", &GodotVim::_move_to_line_end);

    _create_command("^", &GodotVim::_move_to_first_non_blank);
    _create_command("G", &GodotVim::_move_to_beginning_of_last_line);
    _create_command("gg", &GodotVim::_move_to_beginning_of_first_line);
    _create_command("-", &GodotVim::_move_to_beginning_of_previous_line);
    _create_command("+", &GodotVim::_move_to_beginning_of_next_line);
    _create_command(";", &GodotVim::_move_to_last_searched_char);
    _create_command(",", &GodotVim::_move_to_last_searched_char_backward);
    _create_command("|", &GodotVim::_move_to_column);

    _create_command("w", &GodotVim::_move_word_right);
    _create_command("W", &GodotVim::_move_word_right_big);
    _create_command("e", &GodotVim::_move_word_end);
    _create_command("E", &GodotVim::_move_word_end_big);
    _create_command("ge", &GodotVim::_move_word_end_backward);
    _create_command("gE", &GodotVim::_move_word_end_big_backward);
    _create_command("b", &GodotVim::_move_word_beginning);
    _create_command("B", &GodotVim::_move_word_beginning_big);
    _create_command("}", &GodotVim::_move_paragraph_down);
   _create_command("{", &GodotVim::_move_paragraph_up);
   _create_command("%", &GodotVim::_move_to_matching_pair);
   _create_command("o", &GodotVim::_open_line_below);
   _create_command("O", &GodotVim::_open_line_above);
   _create_command("i", &GodotVim::_enter_insert_mode, ACTION);
   _create_command("a", &GodotVim::_enter_insert_mode_append, ACTION);
   //_create_command("A", &GodotVim::_enter_insert_mode_after_eol, ACTION);
   //_create_command("A", &GodotVim::_enter_insert_mode_after_selection, ACTION, VISUAL);
   //_create_command("I", &GodotVim::_enter_insert_mode_first_non_blank, ACTION);
   //_create_command("I", &GodotVim::_enter_insert_mode_before_selection, ACTION, VISUAL);
   //_create_command("v", &GodotVim::_toggle_visual_mode, ACTION);
   //_create_command("V", &GodotVim::_toggle_visual_line_mode, ACTION);
   //_create_command("<C-v>", &GodotVim::_toggle_visual_block_mode, ACTION);

   _create_command("d", &GodotVim::_delete_text, OPERATOR);
   //_create_command("y", &GodotVim::_yank_text, OPERATOR);
   _create_command("c", &GodotVim::_change_text, OPERATOR);
   _create_command(">", &GodotVim::_indent_text, OPERATOR);
   _create_command("<", &GodotVim::_unindent_text, OPERATOR);
   _create_command("g~", &GodotVim::_change_case, OPERATOR);
   _create_command("gu", &GodotVim::_change_to_lower_case, OPERATOR);
   _create_command("gU", &GodotVim::_change_to_upper_case, OPERATOR);
   //_create_command("=", &GodotVim::_indent_text, OPERATOR);
   //_create_command("n", &GodotVim::_find_next, ACTION);
   //_create_command("N", &GodotVim::_find_previous, ACTION);
   //_create_command("x", &GodotVim::_delete_char, ACTION);
   //_create_command("X", &GodotVim::_delete_previous_char, ACTION);
   //_create_command("D", &GodotVim::_delete_to_eol, ACTION, COMMAND);
   //_create_command("D", &GodotVim::_delete_text, ACTION, VISUAL);
   // ~
   // ~
   //_create_command("gt", &GodotVim::_goto_next_tab, ACTION);
   //_create_command("gT", &GodotVim::_goto_previous_tab, ACTION);

   //_create_command("J", &GodotVim::_join_lines, ACTION);
   //_create_command("p", &GodotVim::_paste, ACTION);
   //_create_command("P", &GodotVim::_paste_before, ACTION);
   //_create_command("r", &GodotVim::_replace_char, ACTION);

   _create_command("u", &GodotVim::_undo, ACTION);
   //_create_command("u", &GodotVim::_change_to_lowercase, OPERATOR, VISUAL);
   //_create_command("<C-r>", &GodotVim::_redo, ACTION);
   //_create_command("U", &GodotVim::_change_to_uppercase, OPERATOR, VISUAL);

   //_create_command("zz", &GodotVim::_scroll_to_center, ACTION);
   //_create_command("z.", &GodotVim::_scroll_to_center_first_non_blank, ACTION);
   //_create_command("zt", &GodotVim::_scroll_to_top, ACTION);
   //_create_command("z<CR>", &GodotVim::_scroll_to_top_fist_non_blank, ACTION);
   //_create_command("z-", &GodotVim::_scroll_to_bottom_, ACTION);
   //_create_command("zb", &GodotVim::_scroll_to_bottom_first_non_blank, ACTION);
   //_create_command(".", &GodotVim::_repeat_last_edit, ACTION);
   // <C-a>
   // <C-x>
   // <C-x>

   //_create_command("a<char>", &GodotVim::_text_object, MOTION);
   //_create_command("i<char>", &GodotVim::_text_object_inner, MOTION);

   //
   //_create_command("/", &GodotVim::_start_search, SEARCH);
   //_create_command("?", &GodotVim::_start_search_backward, SEARCH);
   //_create_command("*", &GodotVim::_search_word_under_cursor, SEARCH);
   //_create_command("#", &GodotVim::_search_word_under_cursor_backward, SEARCH);
   // g*
   // g#

   //_create_command(":", &GodotVim::_enter_ex, EX);

}

void GodotVim::_create_command(String binding, cmdFunction function, CommandType type, VimMode context) {
    VimCommand cmd = {binding, function, type, context};
    command_map.insert(binding, cmd);
    command_bindings.push_back(binding);
}

void GodotVim::disconnect_all() {
    text_edit->disconnect("input_event", this, "_editor_input");
    text_edit->disconnect("focus_enter", this, "_editor_focus_enter");
}

void GodotVim::_bind_methods() {
    ObjectTypeDB::bind_method("_editor_input", &GodotVim::_editor_input);
    ObjectTypeDB::bind_method("_editor_focus_enter", &GodotVim::_editor_focus_enter);
}

void GodotVim::set_editor(CodeTextEditor *editor) {
    this->editor = editor;
    text_edit = editor->get_text_edit();
    text_edit->connect("input_event", this, "_editor_input");
    text_edit->connect("focus_enter", this, "_editor_focus_enter");
    _setup_editor();
}

GodotVim::GodotVim() {
    vim_mode = COMMAND;
    virtual_column = 0;
    _setup_command_map();
}

GodotVim::~GodotVim() {
    disconnect_all();
}

void GodotVimPlugin::_node_removed(Node * p_node) {
    if (p_node == NULL) return;

    if (editor_registry.has(p_node->get_path())) {
        print_line("node " + p_node->get_path() + " removed");
        GodotVim *godot_vim = editor_registry.find(p_node->get_path())->get();
        //godot_vim->disconnect_all();
        memdelete(godot_vim);
        editor_registry.erase(p_node->get_path());
    }
}

void GodotVimPlugin::_tree_changed() {
    print_line("tree changed");

    Node *text_edit = get_editor_viewport()->find_node("TextEdit", true, false);

    if (text_edit == NULL) return;

    if (text_edit->is_type("TextEdit")) {

        Node * parent = text_edit->get_parent();

        if (parent->is_type("CodeTextEditor") && !parent->is_in_group(vim_plugged_group)) {

            Node * parent_parent = parent->get_parent();

            if (parent_parent->is_type("TabContainer")) {

                tab_container = parent_parent->cast_to<TabContainer>();
                tab_container->connect("tab_changed", this, "_tabs_changed");

                for (int i = 0; i < tab_container->get_tab_count(); i++) {
                    CodeTextEditor *code_editor = tab_container->get_tab_control(i)->cast_to<CodeTextEditor>();
                    code_editor->add_to_group(vim_plugged_group);
                    _plug_editor(code_editor);
                }
            }

        }
    }
}

void GodotVimPlugin::_plug_editor(CodeTextEditor *editor) {
    GodotVim *godot_vim = memnew(GodotVim);
    godot_vim->set_editor(editor);
}

void GodotVimPlugin::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        print_line("entered tree");
        get_tree()->connect("tree_changed", this, "_tree_changed");
        get_tree()->connect("node_removed", this, "_node_removed");
    } else if (p_what == NOTIFICATION_EXIT_TREE) {
        print_line("exiting tree");
        get_tree()->disconnect("node_removed", this, "_node_removed");
        get_tree()->disconnect("tree_changed", this, "_tree_changed");
    }
}

void GodotVimPlugin::_tabs_changed(int current) {
    print_line("tabs changed");
    CodeTextEditor *code_editor = tab_container->get_tab_control(current)->cast_to<CodeTextEditor>();
    if (!code_editor->is_in_group(vim_plugged_group))
        _plug_editor(code_editor);
}

void GodotVimPlugin::_bind_methods() {
    ObjectTypeDB::bind_method("_tree_changed", &GodotVimPlugin::_tree_changed);
    ObjectTypeDB::bind_method("_node_removed", &GodotVimPlugin::_node_removed);
    ObjectTypeDB::bind_method("_tabs_changed", &GodotVimPlugin::_tabs_changed);
}

GodotVimPlugin::GodotVimPlugin(EditorNode *p_node) {
    print_line("plugin created");
    vim_plugged_group = "vim_editor_plugged";
}

GodotVimPlugin::~GodotVimPlugin() {

    if (tab_container != NULL)
        tab_container->disconnect("tab_changed", this, "_tabs_changed");

}
