/* (c) 2018 ukrkyi */
#ifndef DISPLAY_H
#define DISPLAY_H

extern void display_init();
extern void display_clear();
extern void display_set_write_position(unsigned line, unsigned offset);
extern int display_putletter(unsigned char letter);

#endif // DISPLAY_H
