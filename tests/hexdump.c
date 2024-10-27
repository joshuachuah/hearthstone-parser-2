
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

enum
{
	e_success = 0,
	e_systemcall_failed,
};

static const char printable[] = ""
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"`1234567890-="
	"~!@#$%^&*()_+"
	"{}|"
	"[]\\"
	";',./"
	":\"<>?"
	" ";

int main(int n, const char** args)
{
	int error = 0;
	const char* path;
	const char* arg0;
	
	int fd, i;
	uint8_t read_buffer[16];
	char out_buffer[(2 + 1) * 16 + 2 + 1 + 16 + 1 + 1], *m;
	ssize_t read_retval;
	off_t offset = 0;
	
	for (arg0 = *args++; (path = *args++);)
	{
		if (!strcmp(path, "-"))
			fd = 0;
		else if ((fd = open(path, O_RDONLY)) < 0)
			fprintf(stderr, "%s: cannot open(\"%s\"): %m!\n", arg0, path),
			error = e_systemcall_failed;
		
		while (!error && ({
			printf("%08lx  ", offset);
			(read_retval = read(fd, &read_buffer, sizeof(read_buffer))) > 0;
		}))
		{
			for (i = 0, m = out_buffer; i < 16; i++)
			{
				if (i < read_retval)
					m += sprintf(m, "%02x ", read_buffer[i]);
				else
					m += sprintf(m, "   ");
				
				if (i == 7)
					*m++ = ' ';
			}
			
			*m++ = ' ';
			*m++ = '|';
			
			for (i = 0; i < read_retval; i++)
				if (memchr(printable, read_buffer[i], sizeof(printable) - 1))
					*m++ = read_buffer[i];
				else
					*m++ = '.';
			
			*m++ = '|';
			
			*m = '\0', puts(out_buffer);
			
			offset += read_retval;
		}
		
		putchar('\n');
		
		if (!error && read_retval < 0)
			perror("read"),
			error = e_systemcall_failed;
		
		if (fd > 0) close(fd);
	}
	
	return error;
}























