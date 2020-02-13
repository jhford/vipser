#ifdef __linux__

#include <stddef.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <linux/signal.h>
#include <sys/ptrace.h>

#include "sandbox.h"

int init_sandbox() {
    return prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
}

#else // __linux__

int init_sandbox() {
    return 0;
}

#endif // __linux__
