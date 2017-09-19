#include "register_types.h"
#include "object_type_db.h"
#include "godot_vim.h"

void register_godot_vim_types() {
    EditorPlugins::add_by_type<GodotVimPlugin>();
//    ObjectTypeDB::register_type<Test>();
}

void unregister_godot_vim_types() {
   //nothing to do here
}
