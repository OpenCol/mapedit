/* btype.c -- numbers in text: binary, decimal and hexadecimal.
 *
 * stoi reads the numbers in the text files: "0x" introduces a hexadecimal number (xtoi), "0b"
 * a binary one (btoi), "0d" or no prefix a decimal one (atoi). The prefix's letter may be
 * either case. All three give 16-bit values and wrap without a check; each stops at the first
 * character that is not a digit of its base.
 */
#include "msc.h"
#include "btype.h"
#include "font.h"
#include "loader.h"
#include "popup.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */

/* Reads hexadecimal digits (either case) from p, which follows the "0x". */
i16 xtoi(char *p)
{
    i16 result = 0;
    i16 ch;

    while (*p != 0) {
        ch = *p;
        p++;
        ch = toupper(ch);
        if (isdigit(ch)) {
            result = result << 4;
            result += ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            result = result << 4;
            result += ch - 0x37;
        } else {
            while (*p != 0)
                p++;
        }
    }
    return result;
}

/* Reads binary digits from p, which follows the "0b". */
i16 btoi(char *p)
{
    i16 value = 0;
    i16 c;

    while (*p != 0) {
        c = *p;
        p++;
        c = toupper(c);
        if (c == '0' || c == '1') {
            value = value * 2;
            value += c - '0';
        } else {
            while (*p != 0)
                p++;
        }
    }
    return value;
}

/* The number in s: hexadecimal after "0x", binary after "0b", otherwise decimal (a "0d"
 * prefix is skipped). The test on s[1] upper-cases a lower-case letter. */
i16 stoi(char *s)
{
    char c;

    if (s[0] == '0') {
        c = s[1];
        if ((_ctype + 1)[(u8)c] & 2)
            c -= 0x20;
        if (c == 'X')
            return xtoi(s + 2);
        if (c == 'B')
            return btoi(s + 2);
        if (c == 'D')
            return msc_atoi(s + 2);
    }
    return msc_atoi(s);
}
