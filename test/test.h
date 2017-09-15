#ifndef TEST_H
#define TEST_H

#include "reference.h"
#include "editor/editor_plugin.h"
#include "editor/plugins/script_editor_plugin.h"

class TestProxy : public ScriptTextEditorProxy {
    OBJ_TYPE(TestProxy, ScriptTextEditorProxy);

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

    VimMode vim_mode;
    int virtual_column;

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
    int _get_current_line_length();

    int _find_forward(const String &p_string);
    int _find_backward(const String &p_string);

    void _open_line(int line);

    void _move_left(int cols);
    void _move_right(int cols);
    void _move_down(int lines);
    void _move_up(int lines);
    void _move_columns(int cols);
    void _move_lines(int lines);

    void _check_virtual_column();

    void _cursor_set_line(int line);
    void _cursor_set_column(int column);
    int _cursor_get_line();
    int _cursor_get_column();

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
