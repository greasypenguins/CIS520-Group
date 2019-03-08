#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>

typedef int pid_t;

void syscall_init (void);

//static void syscall_handler (struct intr_frame *f UNUSED); //double check
void halt(void);
void exit (int);
pid_t exec (const char *cmd_line);
int wait (pid_t);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int);
int read (int, void *buffer, unsigned size);
int write (int, const void *buffer, unsigned size);
void seek (int, unsigned position);
unsigned tell (int);
void close (int);

#endif /* userprog/syscall.h */
