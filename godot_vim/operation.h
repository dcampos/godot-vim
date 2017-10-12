#ifndef OPERATION_H
#define OPERATION_H

#include "command.h"

class Operation : public Command {
public:
    typedef void (Operation::* operationFunction) ();

private:
    enum CaseOperation {
        TO_LOWER,
        TO_UPPER,
        INVERT
    };

    int flags;
    operationFunction fcn;

    Operation(GodotVim *vim, int flags, operationFunction fcn);

    void _change_case(CaseOperation option);

public:

    void delete_char();
    void delete_previous_char();
    void delete_text();
    void delete_lines();

    void yank();

    void to_upper();
    void to_lower();
    void toggle_case();

    void change_text();
    void change_lines();

    void indent_text();
    void unindent_text();
    void reindent_text();

    void apply_with_motion(Motion *motion);

    void run();

    bool is_operation() { return true; }

    Operation *get_operation() { return this; }

    static Operation *create_operation(GodotVim *vim, int flags, operationFunction fcn);
};

#endif // OPERATION_H
