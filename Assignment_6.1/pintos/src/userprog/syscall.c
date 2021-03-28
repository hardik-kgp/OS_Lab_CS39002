#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
static void syscall_handler (struct intr_frame *);
void check_address(const void*);
void process_exit(int);
int process_exec(char *);

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
      process_exit(*(p+1));
      break;

    case SYS_EXEC:
      check_addr(p+1);
      check_addr(*(p+1));
      f->eax = process_exec(*(p+1));
      break;

    case SYS_WRITE:
      // printf("fd : %d | Length : %d\n",*(p+5),*(p+7));
      // printf("buffer: %szz\n",*(p+6));
      if(*(p + 5) == 1){
        putbuf(*(p + 6), *(p + 7));  
      }
      break;
    
    default:
      printf("Default: %d", system_call);
  }
}

void check_address(const void *addr)
{
  struct thread *t = thread_current();

  if(!is_user_vaddr(addr) || !pagedir_get_page(t->pagedir, vaddr)){
    process_exit(FAILURE);
  }
}

void process_exit(int status)
{
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

int process_exec(char *fname)
{
	acquire_filesys_lock();
	char * fn_cp = malloc (strlen(fname)+1);
  strlcpy(fn_cp, fname, strlen(fname)+1);
  
  char * save_ptr;
  fn_cp = strtok_r(fn_cp," ",&save_ptr);

  struct file* f = filesys_open (fn_cp);

  if(f==NULL)
  {
    release_filesys_lock();
    return -1;
  }
  else
  {
    file_close(f);
    release_filesys_lock();
    return process_execute(fname);
  }
}