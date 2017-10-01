#ifndef COMMAND_H
#define COMMAND_H

#include "reference.h"

class GodotVim;
class Motion;
class Action;
class Operation;

class Command : public Reference {

protected:
    GodotVim* vim;

public:

    virtual Motion *get_motion() { return NULL; }

    virtual Action *get_action() { return NULL; }

    virtual Operation *get_operation() { return NULL; }

    virtual bool is_motion() { return false; }

    virtual bool is_operation() { return false; }

    virtual bool is_action() { return false; }

    virtual void run() = 0;

    Command(GodotVim *vim);

};

#endif // COMMAND_H
