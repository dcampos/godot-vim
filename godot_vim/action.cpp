#include "action.h"
#include "util.h"
#include "godot_vim.h"

void Action::enter_insert_mode() {
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::enter_insert_mode_append() {
    vim->_set_vim_mode(GodotVim::INSERT);
    vim->_cursor_set_column(vim->_cursor_get_column() + 1);
}

void Action::enter_insert_mode_after_eol() {
    vim->_set_vim_mode(GodotVim::INSERT);
    vim->_cursor_set_column(vim->_get_current_line_length());
}

void Action::enter_insert_mode_after_selection() {
    vim->_cursor_set_column(vim->get_selection().to.col);
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::enter_insert_mode_first_non_blank() {
    vim->_set_vim_mode(GodotVim::INSERT);
    vim->_cursor_set_column(_find_first_non_blank(vim->_get_current_line()));
}

void Action::enter_insert_mode_before_selection() {
    vim->_cursor_set_column(0);
    vim->_cursor_set_line(vim->get_selection().from.row);
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::enter_ex() {
    vim->get_command_line()->show();
    vim->get_command_line()->set_text(":");
    vim->get_command_line()->set_cursor_pos(1);
    vim->get_command_line()->grab_focus();
}

void Action::undo() {
    vim->get_text_edit()->undo();
}

void Action::redo() {
    vim->get_text_edit()->redo();
}

void Action::toggle_visual_mode() {
    if (vim->_get_vim_mode() == GodotVim::VISUAL) {
        vim->_set_vim_mode(GodotVim::NORMAL);
    } else {
        vim->_set_visual_type(GodotVim::SELECT_CHARS);
        vim->_set_vim_mode(GodotVim::VISUAL);
    }
}

void Action::open_line_below() {
    vim->_cursor_set_column(vim->_get_current_line_length());
    vim->get_text_edit()->insert_text_at_cursor("\n");
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::open_line_above() {
    vim->_cursor_set_column(0);
    vim->get_text_edit()->insert_text_at_cursor("\n");
    vim->_cursor_set_line(vim->_cursor_get_line() - 1);
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::toggle_visual_mode_line() {
    if (vim->_get_vim_mode() == GodotVim::VISUAL && vim->_get_visual_type() == GodotVim::SELECT_LINES) {
        vim->_set_vim_mode(GodotVim::NORMAL);
    } else {
        vim->_set_visual_type(GodotVim::SELECT_LINES);
        vim->_set_vim_mode(GodotVim::VISUAL);
    }
}

void Action::delete_to_eol() {
    int sc = vim->_cursor_get_column();
    int sl = vim->_cursor_get_line();
    int ec = vim->_get_current_line_length();
    int el = sl;

    vim->get_text_edit()->select(sl, sc, el, ec);
    vim->get_text_edit()->cut();
}

void Action::change_to_eol() {
    int sc = vim->_cursor_get_column();
    int sl = vim->_cursor_get_line();
    int ec = vim->_get_current_line_length();
    int el = sl;

    vim->get_text_edit()->select(sl, sc, el, ec);
    vim->get_text_edit()->cut();
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Action::scroll_to_center() {
    vim->get_text_edit()->center_viewport_to_cursor();
}

void Action::run() {
    (this->*fcn)();
}

void Action::_open_line(bool above) {
    if (above) vim->_cursor_set_line(vim->_cursor_get_line() - 1);
    vim->_cursor_set_column(vim->_get_current_line_length());
    vim->get_text_edit()->insert_text_at_cursor("\n");
}

Action::Action(GodotVim *vim, int flags, Action::actionFunction fcn) : Command(vim), flags(flags), fcn(fcn) {
}

Action * Action::create_action(GodotVim *vim, int flags, Action::actionFunction fcn) {
    return memnew(Action(vim, flags, fcn));
}
