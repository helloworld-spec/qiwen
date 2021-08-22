#include <stdio.h>
#include "hal.h"

__attribute__((used))
int fgetc(FILE *stream)
{
	signed char c;

	while (!hal_console_getc(&c)) { }

	if (c == '\r')
		c = '\n';

	//fputc(c, stream);

	return c;
}

__attribute__((used))
char *fgets(char *s, int size, FILE *stream)
{
	char *p = s;

	int i = 0;

	for (i = 0; i < size - 1; i++){

		int c = fgetc(stream);

		if(c == '\n'){

			*p++ = '\n';
			break;
		}
		else if(c == '\0'){

			break;
		}
		else if(c < 0){

			return (void*)0;
		}
		else{
			*p++ = c;
		}
	}

	*p = '\0';

	return s;
}

__attribute__((used))
int getc(FILE *stream)
{
	return fgetc(stream);
}

__attribute__((used))
int getchar(void)
{
	return fgetc((void*)0);
}

__attribute__((used))
int ungetc(int c, FILE *stream);
