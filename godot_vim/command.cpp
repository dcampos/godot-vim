#include "command.h"

bool Command::is_edit() {
    return cmd_flags & IS_EDIT;
}

bool Command::needs_char() {
    return cmd_flags & NEEDS_CHAR;
}

void Command::set_context(Context context) {
    this->context = context;
}

Command::Command(GodotVim * vim, int cmd_flags, Context context) : vim(vim), cmd_flags(cmd_flags), context(context) {

}
