// .c
// OS Dynamic Code Execution Function
// by Kyle Furey

#include "exec.h"
#include "lib.h"
#include "assembly.h"

/**
 * Dynamically executes the given string as assembled machine code.
 * This allocates a page of memory for the process to enable virtual addressing.
 * Returns a handle for the executing process.
 */
phandle_t exec(string_t code) {
    assert(code != NULL, "exec() - code was NULL!");
    // TODO
    return 0;
}

/**
 * Dynamically executes the given string as a shell-like command script.
 * Returns an error string or NULL if the script completed successfully.
 */
string_t cmd(string_t script) {
    assert(script != NULL, "cmd() - script was NULL!");
    // TODO
    return NULL;
}
