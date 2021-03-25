#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int i;
  // printf("inside echo.. argc: %d\n", argc);
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");
	// printf("hello world\n" );

  return EXIT_SUCCESS;
}
