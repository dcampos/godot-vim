#ifndef COMMAND_H
#define COMMAND_H

#include "reference.h"

class Motion;

class Command : public Reference {

public:

    virtual Motion *get_motion() { return NULL; }

    virtual bool is_motion() { return false; }

    virtual bool is_operation() { return false; }

    virtual bool is_action() { return false; }

    virtual void run() = 0;

    Command();

};

#endif // COMMAND_H
