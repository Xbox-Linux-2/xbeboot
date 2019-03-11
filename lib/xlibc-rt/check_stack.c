#include <debug.h>
#include "xboxkrnl/xboxkrnl.h"
#include "check_stack.h"

#ifdef DEBUG_CONSOLE
    #define _print DbgPrint
#else
    #define _print debugPrint
#endif

void _cdecl _xlibc_check_stack(DWORD requested_size, DWORD stack_ptr)
{
    PKTHREAD current_thread = KeGetCurrentThread();

    if (requested_size >= stack_ptr || stack_ptr - requested_size < (DWORD)current_thread->StackLimit)
    {
        _print("\nStack overflow caught!\n"
               "stack pointer: 0x%x\n"
               "request size:  0x%x\n"
               "stack limit:   0x%x)\n\n",
               stack_ptr, requested_size, (DWORD)current_thread->StackLimit);
        // TODO: halt execution
    }
}
