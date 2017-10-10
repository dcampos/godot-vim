#include "godot_vim.h"
#include "map.h"
#include "os/keyboard.h"
#include "os/input_event.h"
#include "editor/editor_node.h"
#include "motion.h"
#include "action.h"
#include "operation.h"

void GodotVim::_clear_state() {
    input_state.current_command = NULL;
    input_state.operator_command = NULL;
    input_state.input_string.clear();
    input_state.repeat_count = 0;
    input_state.operator_count = 0;
}

void GodotVim::_editor_input(const InputEvent &p_event) {
    if (p_event.type == InputEvent::KEY) {
        const InputEventKey &ke = p_event.key;

        if (!ke.pressed) return;

        if (ke.scancode == KEY_ESCAPE) {
            if (vim_mode == INSERT) {
                _cursor_set_column(_cursor_get_column() - 1);
            } else if (vim_mode == NORMAL) {
                input_state.repeat_count = 0;
                input_state.input_string.clear();
            }
            _set_vim_mode(NORMAL);
        } else {
            print_line("parsing command...");
            _parse_command_input(ke);
        }
    }
}

void GodotVim::_parse_command_input(const InputEventKey &p_event) {
    String character = _get_character(p_event);
    input_state.input_string += character;

    if (vim_mode == NORMAL && _is_normal_command(input_state.input_string)) {
        //print_line("running normal");

        Command *cmd = _find_command(input_state.input_string);

        _run_normal_command(cmd);

    } else if (vim_mode == VISUAL && _is_visual_command(input_state.input_string)) {
        //print_line("running visual");

        Command *cmd = _find_command(input_state.input_string);

        _run_visual_command(cmd);

    } else if (input_state.input_string.is_numeric()) {
        //print_line("adding to repeat count");
        int n = input_state.input_string.to_int();
        input_state.repeat_count = input_state.repeat_count * 10 + n;
        input_state.input_string.clear();
    } else {
        //print_line("checking if it exists...");

        if (!_map_contains_key(input_state.input_string, command_map))
            _clear_state();
    }
}

void GodotVim::_run_normal_command(Command *p_cmd) {
    if (p_cmd->is_operation() && input_state.operator_command) {

        if (input_state.operator_command == p_cmd) {
            print_line("same operator");
        }
        _clear_state();

    } else if (p_cmd->is_operation()) {

        //print_line("adding operation => " + p_cmd->get_type());
        input_state.operator_command = p_cmd;
        input_state.operator_count = input_state.repeat_count;
        input_state.repeat_count = 0;
        input_state.input_string.clear();

    } else if (p_cmd->is_motion() && input_state.operator_command) {
        //print_line("running motion/operator command!");

        Command *op_cmd = input_state.operator_command;

        if (op_cmd->is_operation()) {

            int cl = _cursor_get_line();
            int cc = _cursor_get_column();

            _run_command(p_cmd);

            text_edit->select(cl, cc, _cursor_get_line(), _cursor_get_column()+1);

            _run_command(op_cmd);

            text_edit->deselect();
        }
        _clear_state();
    } else {
        //print_line("running simple command!");
        _run_command(p_cmd);
        _clear_state();
    }
}

void GodotVim::_run_visual_command(Command *p_cmd) {
    int sl = text_edit->get_selection_from_line();
    int sc = text_edit->get_selection_from_column();

    if (sl < 0) sl = _cursor_get_line();
    if (sc < 0) sc = _cursor_get_column();

    print_line("running visual command!");

    _run_command(p_cmd);

    if (p_cmd->is_operation())
        _set_vim_mode(NORMAL);

    if (vim_mode == VISUAL) {
        _update_visual_selection();
    } else {
        text_edit->deselect();
    }
    _clear_state();
}

void GodotVim::_update_visual_selection() {
    int l1 = _cursor_get_line();
    int c1 = _cursor_get_column();

    if (l1 < visual_start.y || (l1 == visual_start.y && c1 <= visual_start.x)) {
        if (visual_type == SELECT_LINES) {
            text_edit->select(l1, 0, visual_start.y, _get_line(visual_start.y).length());
        } else {
            text_edit->select(l1, c1, visual_start.y, visual_start.x + 1);
        }
    } else {
        if (visual_type == SELECT_LINES) {
            text_edit->select(visual_start.y, 0, l1, _get_line(l1).length());
        } else {
            text_edit->select(visual_start.y, visual_start.x, l1, c1 + 1);
        }
    }
}

const GodotVim::InputState GodotVim::get_input_state() {
    return input_state;
}

LineEdit *GodotVim::get_command_line() {
    return command_line;
}

bool GodotVim::_map_contains_key(const String &input, Map<String, Command*> map) {

    for (Map<String, Command*>::Element *e = map.front(); e; e = e->next()) {
        String k = e->key();
        if (k.begins_with(input)) {
            return true;
        }
    }

    return false;
}

Command * GodotVim::_find_command(String binding) {
    if (vim_mode == NORMAL && normal_command_map.has(binding)) {
        return normal_command_map.find(binding)->get();
    } else if (vim_mode == VISUAL && visual_command_map.has(binding)) {
        return visual_command_map.find(binding)->get();
    }
    return command_map.find(binding)->get();
}

bool GodotVim::_is_normal_command(const String &input) {
    return command_map.has(input) || normal_command_map.has(input);
}

bool GodotVim::_is_visual_command(const String &input) {
    return command_map.has(input) || visual_command_map.has(input);
}

void GodotVim::_run_command(Command *cmd) {
    print_line("_run_command");
    //if (input_state.repeat_count == 0) input_state.repeat_count++;
    cmd->run();
}

String GodotVim::_get_character(const InputEventKey &p_event) {
    String character = String::chr(wchar_t(p_event.unicode));
    if (p_event.mod.control)
        character = "<C-" + character.to_upper() + ">";
    return character;
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

Motion::Range GodotVim::get_selection() {
    return Motion::Range(
        text_edit->get_selection_from_line(),
        text_edit->get_selection_from_column(),
        text_edit->get_selection_to_line(),
        text_edit->get_selection_to_column()
    );
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

void GodotVim::set_line(int line, String text) {
    text_edit->set_line(line, text);
}


TextEdit * GodotVim::get_text_edit() {
    return text_edit;
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

GodotVim::VimMode GodotVim::_get_vim_mode() {
    return vim_mode;
}

void GodotVim::_set_vim_mode(VimMode mode) {
    switch (mode) {

    case NORMAL:
        text_edit->cursor_set_block_mode(true);
        text_edit->set_readonly(true);
        text_edit->deselect();
        break;

    case INSERT:
        text_edit->cursor_set_block_mode(false);
        text_edit->set_readonly(false);
        text_edit->deselect();
        break;

    case VISUAL:
        if (vim_mode != VISUAL) {
            visual_start.x = _cursor_get_column();
            visual_start.y = _cursor_get_line();
        }
        _update_visual_selection();
        break;

    default:
        break;
    }

    vim_mode = mode;
}

GodotVim::VisualType GodotVim::_get_visual_type() {
    return visual_type;
}

void GodotVim::_set_visual_type(GodotVim::VisualType vtype) {
    visual_type = vtype;
}

void GodotVim::_setup_editor() {
    if (vim_mode == NORMAL) {
        text_edit->cursor_set_block_mode(true);
        text_edit->set_readonly(true);
    }

    _clear_state();
}

void GodotVim::_setup_commands() {
    if (!command_map.empty()) return;

    _create_command("h", Motion::create_motion(this, 0, &Motion::_move_left));
    _create_command("j", Motion::create_motion(this, 0, &Motion::_move_down));
    _create_command("k", Motion::create_motion(this, 0, &Motion::_move_up));
    _create_command("l", Motion::create_motion(this, 0, &Motion::_move_right));
    _create_command("0", Motion::create_motion(this, 0, &Motion::_move_to_line_start));
    _create_command("$", Motion::create_motion(this, 0, &Motion::_move_to_line_end));
    _create_command("^", Motion::create_motion(this, 0, &Motion::_move_to_first_non_blank));
    _create_command("gg", Motion::create_motion(this, 0, &Motion::_move_to_beginning_of_first_line));
    _create_command("G", Motion::create_motion(this, 0, &Motion::_move_to_beginning_of_last_line));
    _create_command("-", Motion::create_motion(this, 0, &Motion::_move_to_beginning_of_previous_line));
    _create_command("+", Motion::create_motion(this, 0, &Motion::_move_to_beginning_of_next_line));
    _create_command(";", Motion::create_motion(this, 0, &Motion::_move_to_last_searched_char));
    _create_command(",", Motion::create_motion(this, 0, &Motion::_move_to_last_searched_char_backward));
    _create_command("|", Motion::create_motion(this, 0, &Motion::_move_to_column));
    _create_command("w", Motion::create_motion(this, 0, &Motion::_move_word_right));
    _create_command("W", Motion::create_motion(this, 0, &Motion::_move_word_right_big));
    _create_command("e", Motion::create_motion(this, 0, &Motion::_move_word_end));
    _create_command("E", Motion::create_motion(this, 0, &Motion::_move_word_end_big));
    _create_command("ge", Motion::create_motion(this, 0, &Motion::_move_word_end_backward));
    _create_command("gE", Motion::create_motion(this, 0, &Motion::_move_word_end_big_backward));
    _create_command("b", Motion::create_motion(this, 0, &Motion::_move_word_beginning));
    _create_command("B", Motion::create_motion(this, 0, &Motion::_move_word_beginning_big));
    _create_command("}", Motion::create_motion(this, 0, &Motion::_move_paragraph_down));
    _create_command("{", Motion::create_motion(this, 0, &Motion::_move_paragraph_up));
    _create_command("%", Motion::create_motion(this, 0, &Motion::_move_to_matching_pair));
    _create_command("n", Motion::create_motion(this, 0, &Motion::find_next));
    _create_command("N", Motion::create_motion(this, 0, &Motion::find_previous));

    // ACTIONS
    _create_command("i", Action::create_action(this, 0, &Action::enter_insert_mode));
    _create_command("a", Action::create_action(this, 0, &Action::enter_insert_mode_append));
    _create_command("A", Action::create_action(this, 0, &Action::enter_insert_mode_after_eol));
    _create_command("A", Action::create_action(this, 0, &Action::enter_insert_mode_after_selection), VISUAL);
    _create_command("I", Action::create_action(this, 0, &Action::enter_insert_mode_first_non_blank));
    _create_command("I", Action::create_action(this, 0, &Action::enter_insert_mode_before_selection), VISUAL);
    _create_command("u", Action::create_action(this, 0, &Action::undo));
    _create_command("<C-R>", Action::create_action(this, 0, &Action::redo));
    _create_command("v", Action::create_action(this, 0, &Action::toggle_visual_mode));
    _create_command("V", Action::create_action(this, 0, &Action::toggle_visual_mode_line));
    _create_command("o", Action::create_action(this, 0, &Action::open_line_below));
    _create_command("O", Action::create_action(this, 0, &Action::open_line_above));

    _create_command("D", Action::create_action(this, 0, &Action::delete_to_eol), NORMAL);
    _create_command("C", Action::create_action(this, 0, &Action::change_to_eol), NORMAL);
    _create_command("zz", Action::create_action(this, 0, &Action::scroll_to_center));

    _create_command(":", Action::create_action(this, 0, &Action::enter_ex));

    // SEARCH
    _create_command("*", Motion::create_motion(this, 0, &Motion::search_word_under_cursor));
    _create_command("#", Motion::create_motion(this, 0, &Motion::search_word_under_cursor_backward));

    // OPERATIONS
    _create_command("x", Operation::create_operation(this, 0, &Operation::delete_char));
    _create_command("X", Operation::create_operation(this, 0, &Operation::delete_previous_char));
    _create_command("d", Operation::create_operation(this, 0, &Operation::delete_text));
    _create_command("D", Operation::create_operation(this, 0, &Operation::delete_text), VISUAL);

    _create_command("y", Operation::create_operation(this, 0, &Operation::yank));
    _create_command("c", Operation::create_operation(this, 0, &Operation::change_text));
    _create_command("C", Operation::create_operation(this, 0, &Operation::change_lines), VISUAL);

    _create_command(">", Operation::create_operation(this, 0, &Operation::indent_text));
    _create_command("<", Operation::create_operation(this, 0, &Operation::unindent_text));
    _create_command("=", Operation::create_operation(this, 0, &Operation::reindent_text));
    _create_command("g~", Operation::create_operation(this, 0, &Operation::toggle_case));
    _create_command("gu", Operation::create_operation(this, 0, &Operation::to_lower));
    _create_command("u", Operation::create_operation(this, 0, &Operation::to_lower), VISUAL);
    _create_command("gU", Operation::create_operation(this, 0, &Operation::to_upper));
    _create_command("U", Operation::create_operation(this, 0, &Operation::to_upper), VISUAL);
}

void GodotVim::_setup_command_map() {

/*
    //_create_command("<C-v>", &GodotVim::_toggle_visual_mode_block, ACTION);

    // ~
    //_create_command("gt", &GodotVim::_goto_next_tab, ACTION);
    //_create_command("gT", &GodotVim::_goto_previous_tab, ACTION);

    //_create_command("J", &GodotVim::_join_lines, ACTION);
    //_create_command("p", &GodotVim::_paste, ACTION);
    //_create_command("P", &GodotVim::_paste_before, ACTION);
    //_create_command("r", &GodotVim::_replace_char, ACTION);

    //_create_command("z.", &GodotVim::_scroll_to_center_first_non_blank, ACTION);
    //_create_command("zt", &GodotVim::_scroll_to_top, ACTION);
    //_create_command("z<CR>", &GodotVim::_scroll_to_top_first_non_blank, ACTION);
    //_create_command("z-", &GodotVim::_scroll_to_bottom_, ACTION);
    //_create_command("zb", &GodotVim::_scroll_to_bottom_first_non_blank, ACTION);
    //_create_command(".", &GodotVim::_repeat_last_edit, ACTION);
    // <C-a>
    // <C-x>
    // <C-x>

    //_create_command("a<char>", &GodotVim::_text_object, MOTION);
    //_create_command("i<char>", &GodotVim::_text_object_inner, MOTION);

    //_create_command("/", &GodotVim::_start_search, SEARCH);
    //_create_command("?", &GodotVim::_start_search_backward, SEARCH);
    // g*
    // g#

    */

}

void GodotVim::_create_command(String binding, Command *cmd, VimMode context) {
    if (context == NONE) {
        command_map.insert(binding, cmd);
    } else if (context == NORMAL) {
        normal_command_map.insert(binding, cmd);
    } else if (context == VISUAL) {
        visual_command_map.insert(binding, cmd);
    }
}

void GodotVim::disconnect_all() {
    text_edit->disconnect("input_event", this, "_editor_input");
    text_edit->disconnect("focus_enter", this, "_editor_focus_enter");
}

void GodotVim::_editor_focus_enter() {
    _setup_editor();
}

void GodotVim::_command_input(const String &p_input) {
    command_line->hide();
    text_edit->grab_focus();
    if (p_input == ":w") {
        save_script();
    }
}

void GodotVim::_bind_methods() {
    ObjectTypeDB::bind_method("_editor_input", &GodotVim::_editor_input);
    ObjectTypeDB::bind_method("_editor_focus_enter", &GodotVim::_editor_focus_enter);
    ObjectTypeDB::bind_method("_command_input", &GodotVim::_command_input);
}

void GodotVim::save_script() {
    print_line("Saving resource...");
    editor_node->save_resource(editor->cast_to<ScriptTextEditor>()->get_edited_script());
}

void GodotVim::set_editor(CodeTextEditor *editor) {
    this->editor = editor;
    this->text_edit = editor->get_text_edit();

    text_edit->connect("input_event", this, "_editor_input");
    text_edit->connect("focus_enter", this, "_editor_focus_enter");

    // TEST

    command_line = memnew(LineEdit);
    editor->add_child(command_line);
    editor->move_child(command_line, text_edit->get_index() + 1);
    command_line->connect("text_entered", this, "_command_input");
    command_line->hide();

    // END

    _setup_editor();
}

GodotVim::GodotVim(EditorNode *p_editor_node) {
    editor_node = p_editor_node;
    vim_mode = NORMAL;
    virtual_column = 0;
    visual_start = Vector2();
    _setup_commands();
}

GodotVim::~GodotVim() {
    //disconnect_all();
}

void GodotVimPlugin::_node_removed(Node * p_node) {
}

void GodotVimPlugin::_tree_changed() {
    print_line("tree changed");

    if (plugged) return;

    Node *editor = get_editor_viewport();

    if (editor == NULL) return;

    Node *script_editor = _get_node_by_type(editor, "ScriptEditor");

    if (!script_editor) return;

    Node *split = _get_node_by_type(script_editor, "HSplitContainer");

    if (!split) return;

    Node *tabs = _get_node_by_type(split, "TabContainer");

    Node *script_list = _get_node_by_type(split, "ItemList");

    if (!tabs || !script_list) return;

    plugged = true;

    tab_container = tabs->cast_to<TabContainer>();

    tab_container->connect("tab_changed", this, "_tabs_changed");

    for (int i = 0; i < tab_container->get_tab_count(); i++) {
        if (!tab_container->get_tab_control(i)->is_type("CodeTextEditor"))
            continue;

        CodeTextEditor *code_editor = tab_container->get_tab_control(i)->cast_to<CodeTextEditor>();

        if (code_editor->has_node(NodePath("GodotVim"))) {
            continue;
        }

        _plug_editor(code_editor);
    }

}

Node *GodotVimPlugin::_get_node_by_type(Node *node, String type) {
    for (int i=0; i < node->get_child_count(); i++) {
        Node * child = node->get_child(i);
        if (child->is_type(type)) {
            return child;
        }
    }

    return NULL;
}

void GodotVimPlugin::_plug_editor(CodeTextEditor *editor) {
    print_line("plugging...");
    GodotVim *godot_vim = memnew(GodotVim(editor_node));
    godot_vim->set_name("GodotVim");
    godot_vim->set_editor(editor);
    editor->add_child(godot_vim);
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
    if (!code_editor->has_node(NodePath("GodotVim"))) {
        _plug_editor(code_editor);
    }
}

void GodotVimPlugin::_bind_methods() {
    ObjectTypeDB::bind_method("_tree_changed", &GodotVimPlugin::_tree_changed);
    ObjectTypeDB::bind_method("_node_removed", &GodotVimPlugin::_node_removed);
    ObjectTypeDB::bind_method("_tabs_changed", &GodotVimPlugin::_tabs_changed);
}

GodotVimPlugin::GodotVimPlugin(EditorNode *p_node) {
    print_line("plugin created");
    editor_node = p_node;
    plugged = false;
}

GodotVimPlugin::~GodotVimPlugin() {

    //if (tab_container)
        //tab_container->disconnect("tab_changed", this, "_tabs_changed");
}
