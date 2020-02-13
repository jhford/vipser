#ifndef VIPSER_SANDBOX_H
#define VIPSER_SANDBOX_H

// Initialise a sandbox for this process.
// On linux, this function will disable all system calls
// other than write(), read() on already open file descriptors
// as well as _exit() and sigreturn().
// check out man 2 seccomp for more details
int init_sandbox();

#endif //VIPSER_SANDBOX_H
