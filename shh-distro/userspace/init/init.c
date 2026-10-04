  #include <stdio.h>
  #include <unistd.h>

  int main(void)
  {
      setvbuf(stdout, NULL, _IONBF, 0);
 
      printf("\r\nHello, world from SHH init!\r\n");
 
      unsigned int count = 0;
      for (;;)
      {
      	count ++;
      	sleep(1);
      	printf("%d\n\r", count);
      }
  }

