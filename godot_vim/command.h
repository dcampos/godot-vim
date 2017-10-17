#ifndef COMMAND_H
#define COMMAND_H

#include "reference.h"

class GodotVim;
class Motion;
class Action;
class Operation;

enum Context {
    CONTEXT_NORMAL,
    CONTEXT_VISUAL,
    CONTEXT_OPERATION,
    CONTEXT_NONE
};

class Command : public Reference {

    int cmd_flags;
    Context context;

public:

    enum {
        NEEDS_CHAR = 1,
        IS_EDIT = 1 << 1,
    };

protected:
    GodotVim* vim;

public:

    bool is_edit();

    bool needs_char();

    void set_context(Context context);

    virtual Motion *get_motion() { return NULL; }

    virtual Action *get_action() { return NULL; }

    virtual Operation *get_operation() { return NULL; }

    virtual bool is_motion() { return false; }

    virtual bool is_operation() { return false; }

    virtual bool is_action() { return false; }

    virtual void run() = 0;

    Command(GodotVim *vim, int cmd_flags = 0, Context context = CONTEXT_NONE);

};

#endif // COMMAND_H
