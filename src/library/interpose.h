#ifndef LIBTAS_INTERPOSE_H_INCLUDED
#define LIBTAS_INTERPOSE_H_INCLUDED

#include "hook.h"
#if defined(__APPLE__) && defined (__MACH__)
# include <mach-o/dyld-interposing.h>
#endif

/* Explanation for these macros:
 *
 * The issue with hooking dlopen, dlsym etc is that we can't use dlsym itself
 * to get a pointer to the original dlsym function (because that would lead to
 * infinite recursion). On Linux we use the internal `_dl_sym` function as a
 * workaround. On macOS we *could* use `dyld_lookup_func`, but this is a little
 * more complicated than we'd like (due to `dyld_lookup_func` being essentially(
 * deprecated. However, it turns out that macOS has its own "interpose" system
 * specifically designed for hooking functions like this, and if we use this
 * system, we don't have to find the original functions manually. The problem
 * is that macOS's "interpose" system and the Linux LD_PRELOAD trick work
 * somewhat differently, so we define these macros that allow us to write code
 * once that works on both macOS and Linux.
 *
 * In particular,
 * - on Linux, we define a function *with the same name as the
 *   original*, while on macOS the function must have a *different* name
 * - on Linux, we define an "orig pointer" that we don't need on macOS
 * - on macOS, we have to "register" the interposition while we don't need to
 *   do so on Linux
 * - we also want a way to unambiguously refer to the original function
 *
 * In practice, for other code, this means you must use `CUSTOM(name)` instead
 * of `name` when calling functions hooked using this method.
 */
#if defined(__APPLE__) && defined(__MACH__)
# define CUSTOM(name) my_##name
# define DEFINE_ORIG_POINTER_MAYBE(name)
# define DYLD_INTERPOSE_MAYBE(name) DYLD_INTERPOSE(CUSTOM(name), name)
# define ORIG(name) name
#else
# define CUSTOM(name) name
# define DEFINE_ORIG_POINTER_MAYBE(name) DEFINE_ORIG_POINTER(name)
# define DYLD_INTERPOSE_MAYBE(name)
# define ORIG(name) orig::name
#endif

#endif
