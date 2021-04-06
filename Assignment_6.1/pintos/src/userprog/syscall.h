#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void close_handler(struct list*, int);
void all_close_handler(struct list* );

#endif /* userprog/syscall.h */
