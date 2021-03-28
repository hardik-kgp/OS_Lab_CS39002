#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
#include "list.h"

static void syscall_handler (struct intr_frame *);
void check_address(const void*);
void exit_handler(int);
int exec_handler(char *);

extern bool running;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  // printf ("system call!\n");
  
  int * p = f->esp;
  check_address(p);

  int system_call = * p;
  switch(system_call){

    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      check_address(p+1);
      exit_handler(*(p+1));
      break;

    case SYS_EXEC:
      check_address(p+1);
      check_address(*(p+1));
      f->eax = exec_handler(*(p+1));
      break;

    case SYS_WRITE:
      // printf("fd : %d | Length : %d\n",*(p+5),*(p+7));
      // printf("buffer: %szz\n",*(p+6));
      if(*(p + 5) == 1){
        putbuf(*(p + 6), *(p + 7));  
      }
      break;

    case SYS_WAIT:
      check_address(p+1);
      f->eax = process_wait(*(p+1));
      break;
    
    default:
      exit_handler(-1);
  }
}

void check_address(const void *addr)
{
  struct thread *t = thread_current();

  if(!is_user_vaddr(addr) || !pagedir_get_page(t->pagedir, addr)){
    exit_handler(-1);
  }
}

void exit_handler(int status)
{
  /*Function to exit the process, receive the status, and remove parent from semaphore queue*/
  struct list_elem *e;

  for (e = list_begin (&thread_current()->parent->child_processes);
    e != list_end (&thread_current()->parent->child_processes);
    e = list_next (e)) {

    struct child *f = list_entry (e, struct child, elem);
    if(f->tid == thread_current()->tid) {
      f->done = true;
      f->exit_error = status;
    }
  }

	thread_current()->exit_error = status;
	if(thread_current()->parent->waiting_child == thread_current()->tid)
		sema_up(&thread_current()->parent->child_lock);

	thread_exit();
}

int exec_handler(char *fname)
{
  /*Function to execute commands in file fname*/
	char * fn_cp = malloc (strlen(fname)+1);
  strlcpy(fn_cp, fname, strlen(fname)+1);
  
  char * save_ptr;
  fn_cp = strtok_r(fn_cp," ",&save_ptr);

  struct file* f = filesys_open (fn_cp);

  if(f==NULL) { // File not found
    return -1;
  }
  else {
    file_close(f); // Execute commands in fname
    return process_execute(fname);
  }
}