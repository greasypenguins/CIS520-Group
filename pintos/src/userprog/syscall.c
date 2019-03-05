#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

struct lock sys_lock; //lock to guarantee only one process is altering the file at a time

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&sys_lock); //initiate the lock for syscall
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  printf ("system call!\n");
  thread_exit ();
}

void
halt(void) {
  shutdown_power_off();
}

void
exit (int status) {
  struct thread t = thread_current(); //set thread t to current thread
  t->status = status; //set kernel thread t's status to passed status
  printf("Thread: %s, exit(%d)\n",t->name, status); //print full thread t name
  thread_exit(); //exit the thread
}

pid_t
exec (const char *cmd_line) {
  if(*cmd_line == NULL) {
    return -1;
  }
  lock_acquire(&sys_lock); // acquire lock before returning child PID
  pid_t new_id = process_execute(*cmd_line);
  lock_release(&sys_lock);
  return new_id;
}

int
wait (pid_t pid) {
  process_wait(pid);
}

bool
create (const char *file, unsigned initial_size) {
  lock_acquire(&sys_lock); //added lock
  bool file_status = filesys_create(file, initial_size);
  lock_release(&sys_lock);
  return filesys_create(file, initial_size);
}

bool
remove (const char *file) {
  lock_acquire(&sys_lock); //added lock
  bool removed = filesys_remove(file);
  lock_release(&sys_lock);
  return filesys_remove(file);
}

int
open (const char *file) {
  struct file f = filesys_open(file);
  if (f == NULL) {
    return -1;
  }
  int out = current_thread()->fd;
  current_thread()->fd++;
  return out;
}

int
filesize (int fd) {
  return (int)file_length(fd);
}

int
read (int fd, void *buffer, unsigned size) {
  int data = file_read(fd, buffer, size);
  if (data == 0) {
    return -1;
  }
  return data;
}

int
write (int fd, const void *buffer, unsigned size) {
  if (fd == 1) {
    return putbuf(); //implement correct call using this fucntion
  else {
    return file_write(fd, buffer, size);
  }

}

void
seek (int fd, unsigned position) {
  //set current file location marker to this value?
}

unsigned
tell (int fd) {
  //implemented alongside seek
}

void
close (int fd) {
  file_close(fd);
}
