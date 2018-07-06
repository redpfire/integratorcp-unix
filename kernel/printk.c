
#include <kernel.h>
#include <serial.h>

static char* itoh(int i, char *buf)
{
	const char 	*itoh_map = "0123456789ABCDEF";
	int			n;
	int			b;
	int			z;
	int			s;
 
	if (sizeof(void*) == 4)
		s = 8;
	if (sizeof(void*) == 8)
		s = 16;
 
	for (z = 0, n = 7; n > -1; --n)
	{
		b = (i >> (n * 4)) & 0xf;
		buf[z] = itoh_map[b];
		++z;
	}
	buf[z] = 0;
	return buf;
}

void printk(const char *fmt, ...)
{
    const char *p;
    char buf[1024] = {0};
    int x, y;
    int i;
    char *s;
    char fmtbuf[256];

    x = 0;
    __builtin_va_list argp;
    __builtin_va_start(argp, fmt);

    for(p = fmt; *p != '\0'; ++p)
    {
        if(*p =='\\')
        {
            switch(*++p)
            {
                case 'n':
                    buf[x++] = '\n';
                    break;
                case 'r':
                    buf[x++] = '\r';
                    break;
                default:
                    break;
            }
            continue;
        }

        if(*p != '%')
        {
            buf[x++] = *p;
            continue;
        }
        switch(*++p)
        {
            case 'c':
                i = __builtin_va_arg(argp, int);
                buf[x++] = i;
                break;
            case 's':
                s = __builtin_va_arg(argp, char *);
                for(y = 0; s[y]; ++y)
                {
                    buf[x++] = s[y];
                }
                break;
            case 'x':
                i = __builtin_va_arg(argp, int);
                s = itoh(i, fmtbuf);
                for(y = 0; s[y]; ++y)
                {
                    buf[x++] = s[y];
                }
                break;
            case '%':
                buf[x++] = '%';
                break;
        }
    }

    __builtin_va_end(argp);
    buf[x] = 0;
    serial_puts(buf);
}