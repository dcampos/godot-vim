#ifndef GODOT_VIM_H
#define GODOT_VIM_H

#include "reference.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/script_editor_plugin.h"

class GodotVim : public Reference {
    OBJ_TYPE(GodotVim, Reference);

    typedef void (GodotVim::* cmdFunction) ();
    typedef bool (* matchFunction) (int, String);

    enum VimMode {
        COMMAND,
        INSERT,
        VISUAL,
        VISUAL_BLOCK,
        VISUAL_LINE,
        ALL_MODES
    };

    enum Command {
        FIND_FORWARD,
        FIND_BACKWARD
    };

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
    };

    static Vector<String> command_bindings;
    static Map<String, VimCommand> command_map;

    VimMode vim_mode;
    int virtual_column;
    int repeat_count;
    int motion_count;

    String current_command;

    String input_string;

    int count;

    TextEdit *text_edit;
    CodeTextEditor *editor;

    void _setup_editor();
    void _clear_state();
    void _editor_focus_enter();
    void _editor_input(const InputEvent &p_event);

    // Helper methods
    void _set_vim_mode(VimMode mode);
    void _parse_command_input(const InputEventKey &p_event);
    String _get_character(const InputEventKey &p_event);

    String _get_current_line();
    String _get_line(int line);
    int _get_current_line_length();
    int _get_line_count();

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

    void _cursor_set(int line, int col);
    void _cursor_set_line(int line);
    void _cursor_set_column(int column);
    int _cursor_get_line();
    int _cursor_get_column();

    static void _setup_command_map();
    static void _create_command(String binding, cmdFunction function, CommandType type = MOTION, VimMode context = ALL_MODES);

protected:
    static void _bind_methods();
    void disconnect_all();

public:
    void set_editor(CodeTextEditor *editor);

    GodotVim();
    ~GodotVim();
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
