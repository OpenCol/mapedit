/* btype.h -- numbers in text: hexadecimal, binary and decimal.
 *
 * Defined in sys/btype.c. stoi reads a number in any of the three bases: "0x..." is
 * hexadecimal (xtoi), "0b..." binary (btoi), "0d..." or anything else decimal (atoi). In xtoi
 * and btoi a character that is not a digit of the base ends the number and the rest of the
 * text is ignored.
 */
#ifndef MAPEDIT_BTYPE_H
#define MAPEDIT_BTYPE_H

#include "msc.h"

i16 xtoi(char *p);
i16 btoi(char *p);
i16 stoi(char *s);

#endif
