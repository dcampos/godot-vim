#ifndef GODOT_VIM_H
#define GODOT_VIM_H

#include "reference.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/script_editor_plugin.h"
#include "motion.h"

class GodotVim : public Reference {
    OBJ_TYPE(GodotVim, Reference);

    typedef void (GodotVim::* cmdFunction) ();
    typedef bool (* matchFunction) (int, String);

    enum VimMode {
        NORMAL,
        INSERT,
        VISUAL,
        NONE
    } vim_mode;

    enum VisualType {
        SELECT_CHARS,
        SELECT_LINES,
        SELECT_BLOCK
    } visual_type;

    enum CommandType {
        MOTION,
        OPERATOR,
        ACTION,
        SEARCH,
        EX
    };

    enum CaseOperation {
        TO_LOWER,
        TO_UPPER,
        INVERT
    };

    struct VimCommand {
        String binding;
        cmdFunction function;
        CommandType type;
        VimMode context;
        VimCommand(String binding, cmdFunction function, CommandType type, VimMode context)
            : binding(binding), function(function), type(type), context(context) {}
        VimCommand() {}
    };

    struct MotionCommand : VimCommand {
        bool linewise;
        MotionCommand() : linewise(true) {}
    };

    struct InputState {
        Command *operator_command;
        Command *current_command;
        int operator_count;
        int repeat_count;
        String input_string;
    } input_state;

    struct MotionState {
        Vector2 start;
        Vector2 end;
        bool linewise;
        bool inclusive;
    } motion_state;

    Vector<String> command_bindings;
    Map<String, Command*> command_map;
    Map<String, Command*> normal_command_map;
    Map<String, Command*> visual_command_map;

    Motion *motion;

    Vector2 visual_start;
    int virtual_column;

    TextEdit *text_edit;
    CodeTextEditor *editor;

    void _setup_editor();
    void _clear_state();
    void _editor_focus_enter();
    void _editor_input(const InputEvent &p_event);

    // Helper methods
    void _set_vim_mode(VimMode mode);
    void _parse_command_input(const InputEventKey &p_event);
    void _run_normal_command(Command *p_cmd);
    void _run_visual_command(Command *p_cmd);

    Command * _find_command(String binding);

    void _setup_commands();

    bool _is_normal_command(const String &input);
    bool _is_visual_command(const String &input);

    bool _map_contains_key(const String &input, Map<String, Command*> map);

    void _run_command(Command *cmd);

    String _get_character(const InputEventKey &p_event);

    // Action commands
    void _move_left();
    void _move_right();
    void _move_down();
    void _move_up();
    void _move_to_line_end();
    void _move_to_line_start();
    void _move_word_right();
    void _move_word_right_big();
    void _move_word_beginning();
    void _move_word_beginning_big();
    void _move_word_end();
    void _move_word_end_big();
    void _move_word_end_backward();
    void _move_word_end_big_backward();
    void _move_paragraph_up();
    void _move_paragraph_down();
    void _move_to_matching_pair();
    void _move_to_first_non_blank();
    void _move_to_beginning_of_last_line();
    void _move_to_beginning_of_first_line();
    void _move_to_beginning_of_previous_line();
    void _move_to_beginning_of_next_line();
    void _move_to_last_searched_char();
    void _move_to_last_searched_char_backward();
    void _move_to_column();

    void _enter_insert_mode();
    void _enter_insert_mode_append();

    void _toggle_visual_mode();
    void _toggle_visual_mode_line();

    void _open_line_above();
    void _open_line_below();

    void _delete_text();
    void _change_text();
    void _change_case();
    void _change_to_lower_case();
    void _change_to_upper_case();
    void _indent_text();
    void _unindent_text();

    void _undo();
    void _redo();

    void _goto_next_tab();
    void _goto_previous_tab();

    // Utility functions
    int _find_forward(const String &p_string);
    int _find_backward(const String &p_string);

    void _move_backward(matchFunction function);
    void _move_forward(matchFunction function);
    void _move_to_first_non_blank(int line);
    void _move_by_columns(int cols);
    void _move_by_lines(int lines);
    void _open_line(int line);
    void _change_case(CaseOperation option);
    Point2 _find_next_word();

    void _check_virtual_column();

    static void _setup_command_map();
    static void _add_command(VimCommand *command);

protected:
    static void _bind_methods();
    void disconnect_all();

public:
    void set_editor(CodeTextEditor *editor);

    void _create_command(String binding, Command *cmd, VimMode context = NONE);

    String _get_current_line();
    String _get_line(int line);
    int _get_current_line_length();
    int _get_line_count();

    void _cursor_set(int line, int col);
    void _cursor_set_line(int line);
    void _cursor_set_column(int column);
    int _cursor_get_line();
    int _cursor_get_column();

    int _selection_get_line_from();
    int _selection_get_column_from();
    int _selection_get_line_to();
    int _selection_get_column_to();
    void _update_visual_selection();

    const InputState get_input_state();

    GodotVim();
    ~GodotVim();

    static bool _is_text_char(CharType c);
    static bool _is_symbol(CharType c);
    static bool _is_char(CharType c);
    static bool _is_number(CharType c);
    static bool _is_hex_symbol(CharType c);
    static bool _is_pair_right_symbol(CharType c);
    static bool _is_pair_left_symbol(CharType c);
    static bool _is_pair_symbol(CharType c);
    static CharType _get_right_pair_symbol(CharType c);
    static CharType _get_left_pair_symbol(CharType c);
    static bool _is_space(CharType c);
};

class GodotVimPlugin : public EditorPlugin {
    OBJ_TYPE(GodotVimPlugin, EditorPlugin);

    static Map<String, GodotVim*> editor_registry;

    TabContainer *tab_container;

    void _plug_editor(CodeTextEditor *editor);

    String vim_plugged_group;

protected:
    static void _bind_methods();

public:
    void _notification(int p_what);
    void _tree_changed();
    void _node_removed(Node *p_node);
    void _tabs_changed(int current);

    GodotVimPlugin(EditorNode *p_node);
    ~GodotVimPlugin();
};

#endif // GODOT_VIM_H
