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
#include "lib/syscall-nr.h"

int get_value_from_stack(struct intr_frame *, int);
bool user_pointer_is_valid(const void *);
static void syscall_handler (struct intr_frame *);

struct lock sys_lock; //lock to guarantee only one process is altering the file at a time

int get_value_from_stack(struct intr_frame * f, int position)
{
  return *((int *)(f->esp) + position);
}

/* Returns true if the pointer is valid */
bool user_pointer_is_valid(const void * ptr)
{
  /* Null pointers and pointers to outside user memory are invalid */
  if((ptr == NULL        )
  || (!is_user_vaddr(ptr)))
  {
    return false;
  }

  uint32_t * active_page_table_directory = active_pd(); //Can also get with thread_current()->pagedir

  /* Pointer is valid only if it points to a page in the active page table directory */
  if( lookup_page(active_page_table_directory, ptr, false) == NULL) //Can also use pagedir_get_page
  {
    return false;
  }
  else
  {
    return true;
  }
}

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&sys_lock); //initiate the lock for syscall
}

static void
syscall_handler (struct intr_frame *f)
{
  if(!user_pointer_is_valid((void *)f))
  {
    return;
  }

  int fd;
  void * buffer;
  int status;
  int ret;
  pid_t pret;
  const char *cmd_line;

  int syscall_number = get_value_from_stack(f, 0);
  switch(syscall_number)
  {
    /* Projects 2 and later. */
    case SYS_HALT:      /* Halt the operating system. */
      printf("syscall number matches SYS_HALT\n");
      halt();
      break;
    case SYS_EXIT:      /* Terminate this process. */
      printf("syscall number matches SYS_EXIT\n");
		  status = (int)get_value_from_stack(f, 1);
		  exit(status);
      //get status from stack
      //exit(int status);
      break;
    case SYS_EXEC:      /* Start another process. */
      printf("syscall number matches SYS_EXEC\n");
		  cmd_line = (const char *)get_value_from_stack(f, 1);
      //hex_dump((uintptr_t)(f->esp), f->esp, 8, true);
		  pret = exec(cmd_line);
	    f->eax = (uint32_t)pret;
      //get cmd_line from the stack
      //pid_t ret = exec(const char *cmd_line);
      //return ret on the stack
      break;
    case SYS_WAIT:      /* Wait for a child process to die. */
      printf("syscall number matches SYS_WAIT\n");
	    pret = (pid_t)get_value_from_stack(f, 1);
	  
	    ret = wait(pret);
	    f->eax = (uint32_t)ret;
      //get pid from the stack
      //int ret = wait(pid_t pid);
      //return ret on the stack
      break;
    case SYS_CREATE:    /* Create a file. */
      printf("syscall number matches SYS_CREATE\n");
	    cmd_line = (const char*)get_value_from_stack(f, 1);
	    unsigned initial_size = (unsigned int)get_value_from_stack(f, 2);
	    bool ret = create(cmd_line, initial_size);
	    f->eax = (uint32_t)ret;
      //get file from the stack
      //get initial_size from the stack
      //bool ret = create(const char *file, unsigned initial_size);
      //return ret on the stack
      break;
    case SYS_REMOVE:    /* Delete a file. */
      printf("syscall number matches SYS_REMOVE\n");
      f->eax = (uint32_t)remove((const char *)get_value_from_stack(f, 1));
      break;
    case SYS_OPEN:      /* Open a file. */
      printf("syscall number matches SYS_OPEN\n");
	    f->eax = (uint32_t)open((const char *)get_value_from_stack(f, 1));
      break;
    case SYS_FILESIZE:  /* Obtain a file's size. */
      printf("syscall number matches SYS_FILESIZE\n");
	    f->eax = (uint32_t)filesize((int)get_value_from_stack(f, 1));
      break;
    case SYS_READ:      /* Read from a file. */
      printf("syscall number matches SYS_READ\n");
	    fd = (int)get_value_from_stack(f, 1);
	    buffer = (void *)get_value_from_stack(f, 2);
	    unsigned size = (unsigned)get_value_from_stack(f, 3);
	    f->eax = (uint32_t)read(fd, buffer, size);
      break;
    case SYS_WRITE:     /* Write to a file. */
      printf("syscall number matches SYS_WRITE\n");
      fd = (int)get_value_from_stack(f, 1);
      buffer = (void *)get_value_from_stack(f, 2);
      unsigned int isize = (unsigned int)get_value_from_stack(f, 3);
      ret = write(fd, buffer, isize);
      f->eax = (uint32_t)ret;
      break;
    case SYS_SEEK:      /* Change position in a file. */
      printf("syscall number matches SYS_SEEK\n");
      fd = (int)get_value_from_stack(f, 1);
      unsigned int position = (unsigned int)get_value_from_stack(f, 2);
      seek(fd, position);
      break;
    case SYS_TELL:      /* Report current position in a file. */
      printf("syscall number matches SYS_TELL\n");
      fd = (int)get_value_from_stack(f, 1);
      unsigned int uret = tell(fd);
      f->eax = (uint32_t)uret;
      break;
    case SYS_CLOSE:     /* Close a file. */
      printf("syscall number matches SYS_CLOSE\n");
      fd = (int)get_value_from_stack(f, 1);
      close(fd);
      break;
  }
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
  if(!user_pointer_is_valid((void *)cmd_line))
  {
    return -1;
  }

  pid_t new_id = (pid_t)process_execute(cmd_line);

  return new_id;
}

int
wait (pid_t pid) {
  return process_wait(pid);
}

bool
create (const char *file, unsigned initial_size) {
  lock_acquire(&filesys_lock);
  bool create_file = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return create_file;
}

bool
remove (const char *file) {
  lock_acquire(&filesys_lock);
  bool removed = filesys_remove(file);
  lock_release(&filesys_lock);
  return removed;
}

int
open (const char *file) {
  if(!user_pointer_is_valid((void *)file))
  {
    return -1;
  }

  struct thread * t = thread_current();
  lock_acquire(&filesys_lock);
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

  int fd = f->fd;
  lock_release(&filesys_lock);

  return fd;
}

int
filesize (int fd) {
  lock_acquire(&filesys_lock);
  int fs = (int)file_length(thread_get_open_file(fd));
  lock_release(&filesys_lock);
  return fs;
}

int
read (int fd, void *buffer, unsigned size) {
  if(!user_pointer_is_valid(buffer))
  {
    return -1;
  }

  lock_acquire(&filesys_lock);
  int data = file_read(thread_get_open_file(fd), buffer, size);
  lock_release(&filesys_lock);

  if (data == 0) {
    return -1;
  }
  return data;
}

int
write (int fd, const void *buffer, unsigned int size) {
  if(!user_pointer_is_valid(buffer))
  {
    return -1;
  }

  if (fd == 1) {
    putbuf(buffer,size); //implement correct call using this function
    return size;
  }
  else {
    lock_acquire(&filesys_lock);
    int num_bytes_written = file_write(thread_get_open_file(fd), buffer, size);
    lock_release(&filesys_lock);
    return num_bytes_written;
  }

}

void
seek (int fd, unsigned int position) {
  lock_acquire(&filesys_lock);

  if (list_empty(&thread_current()->open_files)) //immediately return if no open files
  {
    lock_release(&filesys_lock);
    return;
  }

  file_seek(thread_get_open_file(fd), position); //otherwise use thread_get_open_file() to return current file as arguement for file_seek()
  lock_release(&filesys_lock);
  return;
}

unsigned int
tell (int fd) {
  lock_acquire(&filesys_lock);
  //Get a reference to the file
  struct file * open_file = thread_get_open_file(fd);
  off_t pos = open_file->pos;
  lock_release(&filesys_lock);
  //Return the file's position
  return pos;
}

void
close (int fd) {
  lock_acquire(&filesys_lock);
  file_close(thread_get_open_file(fd));
  lock_release(&filesys_lock);
}
