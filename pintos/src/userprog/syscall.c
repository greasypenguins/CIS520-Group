#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/malloc.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "filesys/file.c"
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
  struct thread * t = thread_current(); //set thread t to current thread
  t->exit_status = status; //set kernel thread t's status to passed status
  printf("Thread: %s, exit(%d)\n",t->name, status); //print full thread t name
  thread_exit(); //exit the thread
}

pid_t
exec (const char *cmd_line) {
  if(*cmd_line == NULL) {
    return -1;
  }
  lock_acquire(&sys_lock); // acquire lock before returning child PID
  pid_t new_id = (pid_t)process_execute(cmd_line);
  lock_release(&sys_lock);
  return new_id;
}

int
wait (pid_t pid) {
  process_wait(pid);
/*============================================================================================

==============================================================================================
ATTENTION: What are we supposed to return here???
==============================================================================================

============================================================================================*/
}

bool
create (const char *file, unsigned initial_size) {
  lock_acquire(&sys_lock); //added lock
  bool create_file = filesys_create(file, initial_size);
  lock_release(&sys_lock);
  return create_file;
}

bool
remove (const char *file) {
  lock_acquire(&sys_lock); //added lock
  bool removed = filesys_remove(file);
  lock_release(&sys_lock);
  return removed;
}

int
open (const char *file) {
  struct thread * t = thread_current();
  struct file * f = filesys_open(file);
  if (f == NULL) {
    return -1;
  }

  //If open_files is empty, assign this file's fd to be 2
  if(list_empty(&(t->open_files)))
  {
    f->fd =(int) 2;
  }
  //Else assign this file's fd to be 1 + the fd of the last file in open_files
  else
  {
    struct list_elem * last_open_file_elem = list_back(&(t->open_files));
    struct file * last_open_file = list_entry(last_open_file_elem, struct file, file_elem);
    f->fd = last_open_file->fd + 1;
  }

  return f->fd;
}

int
filesize (int fd) {
  return (int)file_length(thread_get_open_file(fd));
}

int
read (int fd, void *buffer, unsigned size) {
  int data = file_read(thread_get_open_file(fd), buffer, size);
  if (data == 0) {
    return -1;
  }
  return data;
}

int
write (int fd, const void *buffer, unsigned size) {
  if (fd == 1) {
    lock_acquire(&sys_lock);
    putbuf(buffer,size); //implement correct call using this fucntion
    lock_release(&sys_lock);
    return size;
  }
  else {
    return file_write(thread_get_open_file(fd), buffer, size);
  }

}

void
seek (int fd, unsigned position) {
  lock_acquire(&sys_lock); //added lock

  if (list_empty(&thread_current()->open_files)) //immediately return if no open files
  {
    lock_release(&sys_lock);
    return;
  }

  file_seek(thread_get_open_file(fd), position); //otherwise use thread_get_open_file() to return current file as arguement for file_seek()
  lock_release(&sys_lock);
  return;
}

unsigned
tell (int fd) {
  //Get a reference to the file
  struct file * open_file = thread_get_open_file(fd);

  //Return the file's position
  return open_file->pos;
}

void
close (int fd) {
  file_close(thread_get_open_file(fd));
}
