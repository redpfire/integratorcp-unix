
#include <stdio.h>
#include <guarded.h>
#include <serial.h>
#include <mem.h>

void _puts(char *s)
{
    resources_t *r = (resources_t *)GUARDED_POINTER;
    access_resource(&r->resources[G_SERIAL]);
    ((void(*)())r->resources[G_SERIAL].pointer)(s);
    release_resource(&r->resources[G_SERIAL]);
}

void _printf(const char *fmt, __builtin_va_list *va)
{
    uint32_t pc = 0;
    uint32_t sp = 0;
    asm volatile("mov %0, r7" : "=r"(pc));
    asm volatile("mov %0, r8" : "=r"(sp));
    const char *p;
    char buf[1024] = {0};
    int x, y;
    int i;
    char *s;
    
    x = 0;
    __builtin_va_list argp;
    argp = *(va);

    for(p = fmt; *p != '\0'; ++p)
    {
        if(*p =='\\')
        {
            switch(*++p)
            {
                case 'n':
                    buf[x] = '\n';
                    ++x;
                    break;
                case 'r':
                    buf[x] = '\r';
                    ++x;
                    break;
                default:
                    break;
            }
            continue;
        }

        if(*p != '%')
        {
            buf[x] = *p;
            ++x;
            continue;
        }
        switch(*++p)
        {
            case 'c':
                i = __builtin_va_arg(argp, int);
                buf[x] = i;
                ++x;
                break;
            case 's':
                s = __builtin_va_arg(argp, char *);
                for(y = 0; s[y]; ++y)
                {
                    buf[x] = s[y];
                    ++x;
                }
                break;
            case '%':
                buf[x] = '%';
                ++x;
                break;
        }
    }

    __builtin_va_end(argp);
    buf[x] = 0;
    _puts(buf);
    asm volatile("mov pc, %0" :: "r"(pc));
}

void printf(const char *fmt, ...)
{
    __builtin_va_list argp;
    __builtin_va_start(argp, fmt);
    asm("push {lr}");
    asm("mov r8, #0x300");
    asm("stm r8, {%0, %1, sp}" :: "r"(fmt),"r"(&argp));
    asm("swi #1");
    asm("mov r8, #0x308");
    asm("ldm r8, {sp}");
    asm("pop {lr}");
}
