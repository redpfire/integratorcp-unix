
#include <serial.h>

void serial_putc(char c)
{
  while(*(volatile uint32_t*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));
  *(volatile uint32_t*) SERIAL_BASE = c;
}

void serial_puts(const char *s)
{
  for(int i = 0 ; s[i] != '\0'; ++i)
  {
    if(s[i] == '\n')
      serial_putc('\r');
    serial_putc(s[i]);
  }
}

char serial_getc()
{
  while(*(volatile uint32_t *)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & SERIAL_RX_EMPTY);
  uint32_t t;
  t = *(volatile uint32_t *) SERIAL_BASE;
  return t & 0xFF;
}
