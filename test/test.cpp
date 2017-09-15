#include "test.h"
#include "os/keyboard.h"
#include "os/input_event.h"
#include "editor/plugins/script_editor_plugin.h"

void TestProxy::editor_input(const InputEvent &p_event) {
    if (p_event.type == InputEvent::KEY) {
        const InputEventKey &kk = p_event.key;

        if (!kk.pressed) return;

        print_line(_get_character(kk));

        if (kk.scancode == KEY_ESCAPE) {
            if (vim_mode == INSERT) {
                _move_left(1);
            }
            _set_vim_mode(COMMAND);
        } else if (vim_mode == COMMAND) {
            _parse_command_input(kk);
        }
    }
}

void TestProxy::_parse_command_input(const InputEventKey &p_event) {
    String character = _get_character(p_event);
    if (current_command == FIND_FORWARD) {
        _find_forward(character);
        current_command = NONE;
    } else if (current_command == FIND_BACKWARD) {
        _find_backward(character);
        current_command = NONE;
    } else if (character == "i") {
        _set_vim_mode(INSERT);
    } else if (character == "a") {
        _set_vim_mode(INSERT);
        _move_right(1);
    } else if (character == "s") {
        _set_vim_mode(INSERT);
        text_edit->select(_cursor_get_line(), _cursor_get_column(), _cursor_get_line(), _cursor_get_column() + 1);
        text_edit->cut();
        //_move_right(1);
    } else if (character == "j") {
        _move_down(1);
    } else if (character == "k") {
        _move_up(1);
    } else if (character == "l") {
        _move_right(1);
    } else if (character == "h") {
        _move_left(1);
    } else if (character == "0") {
        text_edit->cursor_set_column(0);
    } else if (character == "$") {
        _cursor_set_column(_get_current_line_length() - 1);
    } else if (character == "f") {
        current_command = FIND_FORWARD;
    } else if (character == "F") {
        current_command = FIND_BACKWARD;
    } else if (character == "o") {
        _open_line(0);
        _set_vim_mode(INSERT);
    } else if (character == "O") {
        _open_line(-1);
        _set_vim_mode(INSERT);
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
    print_line(itos(n));
    if (n >= 0) {
        _cursor_set_column(n);
    }
}

String TestProxy::_get_character(const InputEventKey &p_event) {
    String character = keycode_get_string(p_event.scancode);
    return p_event.mod.shift ? character : character.to_lower();
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
    return text_edit->get_line(text_edit->cursor_get_line());
}

void TestProxy::_move_left(int cols) {
    _move_columns(-cols);
}

void TestProxy::_move_right(int cols) {
    _move_columns(cols);
}

void TestProxy::_move_down(int lines) {
    _move_lines(lines);
}

void TestProxy::_move_up(int lines) {
    _move_lines(-lines);
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
}

void Test::_bind_methods() {
    // ObjectTypeDB::bind_method("editor_input", &Test::editor_input);
}

Test::Test(EditorNode *p_node) {
    ScriptTextEditor::add_proxy(memnew(TestProxy));
}

Test::~Test() {}
