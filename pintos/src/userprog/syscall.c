#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/init.h"
#include "devices/shutdown.h" 
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "threads/malloc.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);

//Returns a value from the stack at the position given.
int get_value_from_stack(struct intr_frame * f, int position)
{
	return *((int *)(f->esp) + position);
}

//file struct for use in methods
struct thread_file
{
    struct list_elem file_elem;
    struct file *file_addr;
    int file_descriptor;
};
//lock for synchronization
struct lock sys_lock;

void
syscall_init (void) {

  lock_init(&sys_lock);

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*Handles system calls, searches stack for arguments and applies arguments to their respective states.*/
static void
syscall_handler (struct intr_frame *f UNUSED)
{
    check_valid_addr((const void *) f->esp);

    int values[3];
    void * page;

		switch(*(int *) f->esp)
		{
			//Halts the system.
			case SYS_HALT:
				halt();
				break;
			//Exits the program
			case SYS_EXIT:
				values[0]=get_value_from_stack(f, 1);
				exit(values[0]);
				break;
			//Executes the program.
			case SYS_EXEC:
				values[0] = get_value_from_stack(f, 1);
				
				page = (void *) pagedir_get_page(thread_current()->pagedir, (const void *) values[0]);
				if (page == NULL)
				{
					exit(-1);
				}
				values[0] = (int)page;
				f->eax = exec((const char *) values[0]);
				break;
			//Forces the system to wait for process to finish running.
			case SYS_WAIT:
				values[0] = get_value_from_stack(f, 1);

				f->eax = wait((pid_t) values[0]);
				break;
			//Creates a new page.
			case SYS_CREATE:
				values[0] = get_value_from_stack(f, 1);
				values[1] = get_value_from_stack(f, 2);
				check_buffer((void *)values[0], values[1]);

				page = pagedir_get_page(thread_current()->pagedir, (const void *) values[0]);
				if (page == NULL)
				{
				  exit(-1);
				}
				values[0] = (int)page;

				f->eax = create((const char *) values[0], (unsigned) values[1]);
				break;
			//Removes a page.
			case SYS_REMOVE:
				values[0] = get_value_from_stack(f, 1);

				page = pagedir_get_page(thread_current()->pagedir, (const void *) values[0]);
				if (page == NULL)
				{
				  exit(-1);
				}
				values[0] = (int)page;

				f->eax = remove((const char *) values[0]);
				break;
			//Opens a file if it is in memory.
			case SYS_OPEN:
				values[0] = get_value_from_stack(f, 1);

				page = pagedir_get_page(thread_current()->pagedir, (const void *) values[0]);
				if (page == NULL)
				{
				  exit(-1);
				}
				values[0] = (int)page;

				f->eax = open((const char *) values[0]);

				break;
			//Returns the file size of a file.
			case SYS_FILESIZE:
				values[0] = get_value_from_stack(f, 1);

				f->eax = filesize(values[0]);
				break;
			//Reads a given amount of bytes from a file.
			case SYS_READ:
				values[0] = get_value_from_stack(f, 1); 
				values[1] = get_value_from_stack(f, 2);
				values[2] = get_value_from_stack(f, 3);
				check_buffer((void *)values[1], values[2]);

				page = pagedir_get_page(thread_current()->pagedir, (const void *) values[1]);
				if (page == NULL)
				{
				  exit(-1);
				}
				values[1] = (int)page;

				f->eax = read(values[0], (void *) values[1], (unsigned) values[2]);
				break;
			//Writes a given amount of bytes to a file.
			case SYS_WRITE:
				values[0] = get_value_from_stack(f, 1);
				values[1] = get_value_from_stack(f, 2);
				values[2] = get_value_from_stack(f, 3);

				check_buffer((void *)values[1], values[2]);

				page = pagedir_get_page(thread_current()->pagedir, (const void *) values[1]);
				if (page == NULL)
				{
				  exit(-1);
				}
				values[1] = (int)page;

				f->eax = write(values[0], (const void *) values[1], (unsigned) values[2]);
				break;
			//Moves the cursor to a specified position in a file.
			case SYS_SEEK:
				values[0] = get_value_from_stack(f, 1);
				values[1] = get_value_from_stack(f, 2);

				seek(values[0], (unsigned) values[1]);
				break;
			//Returns cursor location.
			case SYS_TELL:
				values[0] = get_value_from_stack(f, 1);

				f->eax = tell(values[0]);
				break;
			//Closes a file.
			case SYS_CLOSE:
				values[0] = get_value_from_stack(f, 1); 

				close(values[0]);
				break;
			//Throws an exception.
			default:
				exit(-1);
				break;
		}
}
//Halts the system.
void halt (void)
{
	shutdown_power_off();
}

//Exits the current process, post the thread number and exit code.
void exit (int status)
{
	thread_current()->exit_status = status;
	printf("Thread: %s, exit(%d)\n",thread_current()->name, status);
    thread_exit ();
}

//Writes data to a given file from buffer, with length 'length'
int write (int fd, const void *buffer, unsigned length)
{
	struct list_elem *temp;


	if(fd == 1)
	{
		lock_acquire(&sys_lock);
		putbuf(buffer, length);
		lock_release(&sys_lock);
		return length;
	}
	if (fd == 0 || list_empty(&thread_current()->open_files))
	{
		return 0;
    }

	for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
	{
		struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
		if (t->file_descriptor == fd)
		{
			lock_acquire(&sys_lock);
			int bytes_written = (int) file_write(t->file_addr, buffer, length);
			lock_release(&sys_lock);
			return bytes_written;
		}
	}
	return 0;
}

//Executes a process with given name, returns the process ID if successful.
pid_t exec (const char * cmd_line)
{
	if(!cmd_line)
	{
		return -1;
	}
	lock_acquire(&sys_lock);
	pid_t child_tid = process_execute(cmd_line);
	lock_release(&sys_lock);
	return child_tid;
}

//Waits on a process to die.
int wait (pid_t pid)
{
  return process_wait(pid);
}

//Creates a new filesystem, returns true if successful.
bool create (const char *file, unsigned initial_size)
{
  lock_acquire(&sys_lock);
  bool file_status = filesys_create(file, initial_size);
  lock_release(&sys_lock);
  return file_status;
}

//Removes a filesystem, returns true if successful.
bool remove (const char *file)
{
  lock_acquire(&sys_lock);
  bool was_removed = filesys_remove(file);
  lock_release(&sys_lock);
  return was_removed;
}

//Opens a file with given name, adds it to the threads open files list and returns the file's designator.
int open (const char *file)
{
  struct file* f = filesys_open(file);

  if(f == NULL)
  {
    return -1;
  }

  lock_acquire(&sys_lock);
  struct thread_file *new_file = malloc(sizeof(struct thread_file));
  new_file->file_addr = f;
  int fd = thread_current ()->cur_fd;
  thread_current ()->cur_fd++;
  new_file->file_descriptor = fd;
  list_push_front(&thread_current ()->open_files, &new_file->file_elem);
  lock_release(&sys_lock);
  return fd;
}

//Returns the filesize of a given filename.
int filesize (int fd)
{
  struct list_elem *temp;

  if (list_empty(&thread_current()->open_files))
  {
    return -1;
  }
  lock_acquire(&sys_lock);
  for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
  {
      struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
      if (t->file_descriptor == fd)
      {
        lock_release(&sys_lock);
        return (int) file_length(t->file_addr);
      }
  }

  lock_release(&sys_lock);

  return -1;
}

//Reads a given file for length bytes.
int read (int fd, void *buffer, unsigned length)
{
  struct list_elem *temp;


  if (fd == 0)
  {
    return (int) input_getc();
  }

  if (fd == 1 || list_empty(&thread_current()->open_files))
  {
    return 0;
  }

  lock_acquire(&sys_lock);
  for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
  {
      struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
      if (t->file_descriptor == fd)
      {
        lock_release(&sys_lock);
        int bytes = (int) file_read(t->file_addr, buffer, length);
        return bytes;
      }
  }

  lock_release(&sys_lock);

  return -1;
}

//Sets file fd's cursor position
void seek (int fd, unsigned position)
{
  struct list_elem *temp;


  if (list_empty(&thread_current()->open_files))
  {
    return;
  }

  lock_acquire(&sys_lock);
  for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
  {
      struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
      if (t->file_descriptor == fd)
      {
        file_seek(t->file_addr, position);
        lock_release(&sys_lock);
        return;
      }
  }

  lock_release(&sys_lock);

  return;
}

//Returns file fd's current cursor position.
unsigned tell (int fd)
{
  struct list_elem *temp;


  if (list_empty(&thread_current()->open_files))
  {
    return -1;
  }

  lock_acquire(&sys_lock);
  for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
  {
      struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
      if (t->file_descriptor == fd)
      {
        unsigned position = (unsigned) file_tell(t->file_addr);
        lock_release(&sys_lock);
        return position;
      }
  }

  lock_release(&sys_lock);

  return -1;
}

//Closes given file, returns if file doesn't exist or is closed.
void close (int fd)
{
  struct list_elem *temp;


  if (list_empty(&thread_current()->open_files))
  {
    return;
  }

  lock_acquire(&sys_lock);
  for (temp = list_front(&thread_current()->open_files); temp != NULL; temp = temp->next)
  {
      struct thread_file *t = list_entry (temp, struct thread_file, file_elem);
      if (t->file_descriptor == fd)
      {
        file_close(t->file_addr);
        list_remove(&t->file_elem);
        lock_release(&sys_lock);
        return;
      }
  }

  lock_release(&sys_lock);

  return;
}

//Checks if a given ptr is valid, throws an exception if not.
void check_valid_addr (const void *ptr_to_check)
{
  if(!is_user_vaddr(ptr_to_check) || ptr_to_check == NULL || ptr_to_check < (void *) 0x08048000)
	{
		exit(-1);
	}
}

//Checks if a buffer is valid for a given amount of memory.
void check_buffer (void *buff_to_check, unsigned size)
{
  unsigned i;
  char *ptr  = (char * )buff_to_check;
  for (i = 0; i < size; i++)
    {
      check_valid_addr((const void *) ptr);
      ptr++;
    }
}
