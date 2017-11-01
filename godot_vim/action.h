#ifndef ACTION_H
#define ACTION_H

#include "command.h"

class Action : public Command {
public:
    typedef void (Action::* actionFunction) ();

private:
    int flags;
    actionFunction fcn;

    Action(GodotVim *vim, int flags, actionFunction fcn, int cmd_flags);

    void _open_line(bool above);

public:

    void enter_insert_mode();
    void enter_insert_mode_append();

    void enter_insert_mode_after_eol();
    void enter_insert_mode_after_selection();
    void enter_insert_mode_first_non_blank();
    void enter_insert_mode_before_selection();

    void enter_ex();

    void open_line_above();
    void open_line_below();

    void undo();
    void redo();

    void paste();
    void paste_before();

    void toggle_visual_mode_line();
    void toggle_visual_mode();

    void next_tab();
    void previous_tab();

    void replace_char();

    void delete_char();
    void delete_previous_char();

    void delete_to_eol();
    void change_to_eol();

    void scroll_to_center();
    void scroll_to_center_first_non_blank();

    void run();

    bool is_action() { return true; }

    Action *get_action() { return this; }

    static Action *create_action(GodotVim *vim, int flags, actionFunction fcn, int cmd_flags = 0);
};

#endif // ACTION_H
