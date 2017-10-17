#ifndef GODOT_VIM_H
#define GODOT_VIM_H

#include "reference.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/script_editor_plugin.h"
#include "motion.h"

class GodotVimPlugin;

struct Registry {
    bool linewise;
    Vector<String> lines;
};

class GodotVim : public Node {
public:
    enum VimMode {
        NORMAL,
        INSERT,
        VISUAL,
        NONE
    };

    enum VisualType {
        SELECT_CHARS,
        SELECT_LINES,
        SELECT_BLOCK
    };

private:
    OBJ_TYPE(GodotVim, Node);

    typedef void (GodotVim::* cmdFunction) ();
    typedef bool (* matchFunction) (int, String);

    VimMode vim_mode;

    VisualType visual_type;

    enum CommandType {
        MOTION,
        OPERATOR,
        ACTION,
        SEARCH,
        EX
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
        String input_char;
    } input_state;

    Registry registry;
    Motion::Range selection_range;

    Vector<String> command_bindings;
    Map<String, Command*> command_map;
    Map<String, Command*> normal_command_map;
    Map<String, Command*> visual_command_map;

    Vector2 visual_start;
    int virtual_column;

    GodotVimPlugin *plugin;
    EditorNode *editor_node;
    TextEdit *text_edit;
    CodeTextEditor *editor;
    LineEdit *command_line;

    void _setup_editor();
    void _clear_state();
    void _editor_focus_enter();
    void _editor_input(const InputEvent &p_event);
    void _command_input(const String &p_input);

    // Helper methods
    void _parse_command_input(const InputEventKey &p_event);
    void _run_normal_command(Command *p_cmd);
    void _run_visual_command(Command *p_cmd);

    Command * _find_command(String binding);

    void _setup_commands();

    bool _is_command(const String &binding);

    bool _is_normal_command(const String &input);
    bool _is_visual_command(const String &input);

    bool _is_char_needed_command(const String &input);

    bool _map_contains_key(const String &input, Map<String, Command*> map);

    void _run_command(Command *cmd);

    String _get_character(const InputEventKey &p_event);

    void _enter_insert_mode();
    void _enter_insert_mode_append();

    void _toggle_visual_mode();
    void _toggle_visual_mode_line();

    void _open_line_above();
    void _open_line_below();

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

    void _set_vim_mode(VimMode mode);
    VimMode _get_vim_mode();
    void _set_visual_type(VisualType vtype);
    VisualType _get_visual_type();

    void _update_visual_selection();

    void _undo();
    void _redo();

    String _get_current_line();
    String _get_line(int line);
    int _get_current_line_length();
    int _get_line_count();

    void _cursor_set(int line, int col);
    void _cursor_set_line(int line);
    void _cursor_set_column(int column);
    int _cursor_get_line();
    int _cursor_get_column();

    Motion::Range get_selection();

    void select_range(Motion::Range range);
    void yank_range(Motion::Range range);
    Registry get_registry();

    void set_line(int line, String text);

    void save_script();

    const InputState get_input_state();

    TextEdit *get_text_edit();
    LineEdit *get_command_line();

    GodotVim(EditorNode *editor_node);
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

    EditorNode *editor_node;

    bool plugged;

protected:
    static void _bind_methods();

public:
    void _notification(int p_what);
    void _tree_changed();
    void _node_removed(Node *p_node);
    void _tabs_changed(int current);

    Node *_get_node_by_type(Node *node, String type);

    GodotVimPlugin(EditorNode *p_node);
    ~GodotVimPlugin();
};

#endif // GODOT_VIM_H
