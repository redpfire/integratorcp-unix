
#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

#include <inttype.h>

void serial_putc(char);
void serial_puts(const char*);
