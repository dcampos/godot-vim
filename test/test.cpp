#include "test.h"
#include "os/keyboard.h"
#include "os/input_event.h"
#include "editor/plugins/script_editor_plugin.h"

// Some utility functions copied over from text_edit.cpp

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

// End of copied functions

void TestProxy::editor_input(const InputEvent &p_event) {
    if (p_event.type == InputEvent::KEY) {
        const InputEventKey &kk = p_event.key;

        if (!kk.pressed) return;

        if (kk.scancode == KEY_ESCAPE) {
            if (vim_mode == INSERT) {
                _move_left();
            } else if (vim_mode == COMMAND) {
                repeat_count = 0;
            }
            _set_vim_mode(COMMAND);
        } else if (vim_mode == COMMAND) {
            _parse_command_input(kk);
        }
    }
}

void TestProxy::_parse_command_input(const InputEventKey &p_event) {
    String character = _get_character(p_event);

    if (command_map.has(character)) {
        VimCommand cmd = command_map.find(character)->get();
        cmdFunction func = cmd.function;
        (this->*func)();
    } else if (current_command == FIND_FORWARD) {
        _find_forward(character);
        current_command = NONE;
    } else if (current_command == FIND_BACKWARD) {
        _find_backward(character);
        current_command = NONE;
    } else if (character.is_numeric() && current_command == NONE) {
        int n = char(p_event.scancode) - '0';
        repeat_count = repeat_count * 10 + n;
        print_line("repeat_count = " + itos(repeat_count));
    } else if (character == "s") {
        _set_vim_mode(INSERT);
        text_edit->select(_cursor_get_line(), _cursor_get_column(), _cursor_get_line(), _cursor_get_column() + 1);
        text_edit->cut();
        //_move_right(1);
    } else if (character == "f") {
        current_command = FIND_FORWARD;
    } else if (character == "F") {
        current_command = FIND_BACKWARD;
    } else if (character == "u") {
        text_edit->undo();
    } else if (character == "r" && p_event.mod.control) {
        text_edit->redo();
    }
}

void TestProxy::_open_line(int line) {
    _move_lines(line);
    _cursor_set_column(_get_current_line_length());
    text_edit->insert_text_at_cursor("\n");
}

int TestProxy::_find_forward(const String &p_string) {
    int n = _get_current_line().find(p_string, _cursor_get_column());
    print_line(itos(n));
    if (n >= 0) {
        _cursor_set_column(n);
    }
}

int TestProxy::_find_backward(const String &p_string) {
    int n = _get_current_line().substr(0, _cursor_get_column()).find(p_string);
    if (n >= 0) {
        _cursor_set_column(n);
    }
}

String TestProxy::_get_character(const InputEventKey &p_event) {
    String character = String::chr(wchar_t(p_event.unicode));
    return character;
}

void TestProxy::editor_focus_enter() {
    _setup_editor();
}

void TestProxy::_cursor_set_line(int line) {
    text_edit->cursor_set_line(line);
}

void TestProxy::_cursor_set_column(int column) {
    text_edit->cursor_set_column(column);
}

int TestProxy::_cursor_get_line() {
    text_edit->cursor_get_line();
}

int TestProxy::_cursor_get_column() {
    text_edit->cursor_get_column();
}

int TestProxy::_get_current_line_length() {
    return text_edit->get_line(text_edit->cursor_get_line()).length();
}

String TestProxy::_get_current_line() {
    return _get_line(_cursor_get_line());
}

String TestProxy::_get_line(int line) {
    return text_edit->get_line(line);
}

void TestProxy::_move_left() {
    _move_columns(-1);
}

void TestProxy::_move_right() {
    _move_columns(1);
}

void TestProxy::_move_down() {
    _move_lines(1);
}

void TestProxy::_move_up() {
    _move_lines(-1);
}

void TestProxy::_move_to_line_start() {
    _cursor_set_column(0);
}

void TestProxy::_move_to_line_end() {
    _cursor_set_column(_get_current_line_length() - 1);
}

Point2 TestProxy::_find_next_word() {
    int next_cc = _cursor_get_column();
    int next_cl = _cursor_get_line();

    String line_text = _get_current_line();
    CharType first_char;

    while (line_text.size() < 1) {
        if (next_cl + 1 >= text_edit->get_line_count())
            return Vector2(next_cc, next_cl);

        line_text = _get_line(++next_cl);
        next_cc = 0;
    }

    first_char = line_text[next_cc];

    bool broken_word = false;

    // TODO: skip multiple tabs
    while (next_cc < line_text.length()) {
        CharType cur_char = line_text[next_cc];

        if (cur_char == ' ' || cur_char == '\t') {
            next_cc++;
            broken_word = true;
        } else if (!broken_word && (_is_text_char(first_char) && _is_text_char(cur_char))
                || (_is_symbol(first_char) && cur_char == first_char)) {
            next_cc++;
        } else {
            print_line("breaking");
            break;
        }

        while (next_cc >= line_text.length()) {
            if (next_cl + 1 > text_edit->get_line_count())
                break;

            line_text = _get_line(++next_cl);
            next_cc = 0;
            broken_word = true;
        }
    }

    return Vector2(next_cc, next_cl);
}

static bool _is_same_word(CharType a, CharType b) {
    return ((_is_text_char(a) && _is_text_char(b))
                || (_is_symbol(a) && b == a));
}

static bool _is_end_of_word(int col, String line) {
    if (line.length() == 0) return false;
    if (col >= line.length() - 1) return true;

    CharType char1 = line[col];
    CharType char2 = line[col + 1];

    return (_is_text_char(char1) && !_is_text_char(char2)) || (_is_symbol(char1) && char1 != char2);
}

static bool _is_beginning_of_word(int col, String line) {
    if (line.length() == 0) return false;
    if (col == 0) return true;

    CharType char1 = line[col - 1];
    CharType char2 = line[col];

    return (!_is_text_char(char1) && _is_text_char(char2)) || (_is_symbol(char2) && char2 != char1);
}

void TestProxy::_move_word_right() {
    Point2 next_word_p2 = _find_next_word();

    _cursor_set_column(next_word_p2.x);
    _cursor_set_line(next_word_p2.y);
}

void TestProxy::_move_word_right_big() {}

void TestProxy::_move_word_end() {
    int next_cc = _cursor_get_column();
    int next_cl = _cursor_get_line();

    String line_text = _get_line(next_cl);

    CharType first_char;

    if (_is_end_of_word(next_cc, line_text)) {
        Point2 next_word_p2 = _find_next_word();
        next_cc = next_word_p2.x;
        next_cl = next_word_p2.y;
    }

    if (line_text.size() == 0) return;

    first_char = line_text[next_cc];

    while (next_cc < line_text.length()) {
        CharType cur_char = line_text[next_cc];

        if (_is_same_word(first_char, cur_char)) {
            next_cc++;
        } else {
            break;
        }
    }

    _cursor_set_column(--next_cc);
    _cursor_set_line(next_cl);
}

void TestProxy::_move_word_end_big() {}

void TestProxy::_move_word_beginning() {}

void TestProxy::_move_word_beginning_big() {}

void TestProxy::_move_paragraph_up() {}

void TestProxy::_move_paragraph_down() {}

void TestProxy::_enter_insert_mode() {
    _set_vim_mode(INSERT);
}

void TestProxy::_enter_insert_mode_append() {
    _set_vim_mode(INSERT);
    _cursor_set_column(_cursor_get_column() + 1);
}

void TestProxy::_open_line_below() {
    _open_line(0);
    _set_vim_mode(INSERT);
}

void TestProxy::_open_line_above() {
    _open_line(-1);
    _set_vim_mode(INSERT);
}

void TestProxy::_move_columns(int cols) {
    int col = CLAMP(text_edit->cursor_get_column() + cols, 0, _get_current_line_length() - 1);
    text_edit->cursor_set_column(col);
    virtual_column = col;
}

void TestProxy::_move_lines(int lines) {
    int line = CLAMP(text_edit->cursor_get_line() + lines, 0, text_edit->get_line_count() - 1);
    text_edit->cursor_set_line(line);
    //_check_virtual_column();
}

void TestProxy::_check_virtual_column() {
    int line_length = _get_current_line_length();
    if (text_edit->cursor_get_column() >= line_length) {
        text_edit->cursor_set_column(line_length - 1);
    } else if (virtual_column < line_length) {
        text_edit->cursor_set_column(virtual_column);
    }
}

void TestProxy::_set_vim_mode(VimMode mode) {
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

void TestProxy::_setup_editor() {
    print_line("setup editor");
    if (vim_mode == COMMAND) {
        text_edit->cursor_set_block_mode(true);
        text_edit->set_readonly(true);
    }
}

Map<String, TestProxy::VimCommand> TestProxy::command_map;

void TestProxy::_setup_command_map() {

    if (command_map.size() > 0) return;

   _create_command("l", &TestProxy::_move_right);
   _create_command("h", &TestProxy::_move_left);
   _create_command("j", &TestProxy::_move_down);
   _create_command("k", &TestProxy::_move_up);
   _create_command("0", &TestProxy::_move_to_line_start);
   _create_command("$", &TestProxy::_move_to_line_end);
   _create_command("w", &TestProxy::_move_word_right);
   _create_command("W", &TestProxy::_move_word_right_big);
   _create_command("e", &TestProxy::_move_word_end);
   _create_command("E", &TestProxy::_move_word_end_big);
   _create_command("b", &TestProxy::_move_word_beginning);
   _create_command("B", &TestProxy::_move_word_beginning_big);
   _create_command("o", &TestProxy::_open_line_below);
   _create_command("O", &TestProxy::_open_line_above);
   _create_command("i", &TestProxy::_enter_insert_mode, ACTION);
   _create_command("a", &TestProxy::_enter_insert_mode_append, ACTION);
}

void TestProxy::_create_command(String binding, cmdFunction function, CommandType type) {
    VimCommand cmd = {binding, function, type};
    command_map.insert(binding, cmd);
}

void TestProxy::_bind_methods() {
    ObjectTypeDB::bind_method("editor_input", &TestProxy::editor_input);
    ObjectTypeDB::bind_method("editor_focus_enter", &TestProxy::editor_focus_enter);
}

void TestProxy::set_editor(ScriptTextEditor *editor) {
    this->editor = editor;
    text_edit = editor->get_text_edit();
    text_edit->connect("input_event", this, "editor_input");
    text_edit->connect("focus_enter", this, "editor_focus_enter");
    _setup_editor();
}

TestProxy::TestProxy() {
    vim_mode = COMMAND;
    virtual_column = 0;
    current_command = NONE;
    _setup_command_map();
}

void Test::_bind_methods() {
    // ObjectTypeDB::bind_method("editor_input", &Test::editor_input);
}

Test::Test(EditorNode *p_node) {
    ScriptTextEditor::add_proxy(memnew(TestProxy));
}

Test::~Test() {}
