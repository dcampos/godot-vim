#include "operation.h"
#include "godot_vim.h"

void Operation::_change_case(CaseOperation option) {
    Motion::Range sel = vim->get_selection();

    vim->get_text_edit()->begin_complex_operation();
    for (int i = sel.from.row; i <= sel.to.row; i++) {
        String line_text = vim->_get_line(i);
        for (int j = (i == sel.from.row ? sel.from.col : 0); j < (i == sel.from.row ? sel.to.col : line_text.length()); j++) {
            switch (option) {

            case TO_LOWER:
                line_text[j] = String::char_lowercase(line_text[j]);
                break;

            case TO_UPPER:
                line_text[j] = String::char_uppercase(line_text[j]);
                break;

            default:
                if (line_text.substr(j, 1).match("[a-z]"))
                    line_text[j] =  String::char_uppercase(line_text[j]);
                else
                    line_text[j] =  String::char_lowercase(line_text[j]);
                break;
            }
        }
        vim->set_line(i, line_text);
    }
    vim->get_text_edit()->end_complex_operation();
}

void Operation::delete_char() {

}

void Operation::delete_previous_char() {

}

void Operation::delete_text() {
    vim->get_text_edit()->cut();
}

void Operation::delete_lines() {

}

void Operation::yank() {

}

void Operation::change_text() {
    vim->get_text_edit()->cut();
    vim->_set_vim_mode(GodotVim::INSERT);
}

void Operation::change_lines() {

}

void Operation::toggle_case() {
    _change_case(INVERT);
}

void Operation::to_lower() {
    _change_case(TO_LOWER);
}

void Operation::to_upper() {
    _change_case(TO_UPPER);
}

void Operation::indent_text() {
    int line = vim->_cursor_get_line();
    String text_line = vim->_get_line(line);
    text_line = '\t' + text_line;
    vim->set_line(line, text_line);
}

void Operation::unindent_text() {
    int line = vim->_cursor_get_line();
    String text_line = vim->_get_line(line);
    if (text_line.begins_with("\t")) {
        text_line.erase(0, 1);
        vim->set_line(line, text_line);
    }
}

void Operation::reindent_text() {

}

void Operation::run() {
    (this->*fcn)();
}

Operation::Operation(GodotVim *vim, int flags, Operation::operationFunction fcn) : Command(vim), flags(flags), fcn(fcn) {
}

Operation * Operation::create_operation(GodotVim *vim, int flags, Operation::operationFunction fcn) {
    return memnew(Operation(vim, flags, fcn));
}
