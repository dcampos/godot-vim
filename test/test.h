#ifndef TEST_H
#define TEST_H

#include "reference.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/script_editor_plugin.h"

class TestProxy : public ScriptTextEditorProxy {
    OBJ_TYPE(TestProxy, ScriptTextEditorProxy);

    typedef void (TestProxy::* cmdFunction) ();

    enum VimMode {
        COMMAND,
        INSERT,
        VISUAL,
        VISUAL_BLOCK,
        VISUAL_LINE
    };

    enum Command {
        FIND_FORWARD,
        FIND_BACKWARD,
        NONE
    };

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
    };

    static Map<String, VimCommand> command_map;

    VimMode vim_mode;
    int virtual_column;
    int repeat_count;

    Command current_command;

    int count;
    TextEdit *text_edit;
    ScriptTextEditor *editor;

    void _setup_editor();
    void editor_focus_enter();

    // Helper methods
    void _set_vim_mode(VimMode mode);
    void _parse_command_input(const InputEventKey &p_event);
    String _get_character(const InputEventKey &p_event);

    String _get_current_line();
    String _get_line(int line);
    int _get_current_line_length();

    int _find_forward(const String &p_string);
    int _find_backward(const String &p_string);

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
    void _move_paragraph_up();
    void _move_paragraph_down();
    void _enter_insert_mode();
    void _enter_insert_mode_append();
    void _open_line_above();
    void _open_line_below();

    // Utility functions
    void _move_columns(int cols);
    void _move_lines(int lines);
    void _open_line(int line);
    Point2 _find_next_word();

    void _check_virtual_column();

    void _cursor_set_line(int line);
    void _cursor_set_column(int column);
    int _cursor_get_line();
    int _cursor_get_column();

    static void _setup_command_map();
    static void _create_command(String binding, cmdFunction function, CommandType type = MOTION);

protected:
    static void _bind_methods();

public:
    void set_editor(ScriptTextEditor *editor) override;
    void editor_input(const InputEvent &p_event) override;

    TestProxy();
};

class Test : public EditorPlugin {
    OBJ_TYPE(Test, EditorPlugin);

protected:
    static void _bind_methods();

public:
    Test(EditorNode *p_node);
    ~Test();
};

#endif // TEST_H
