/* w16types.h -- the Windows 3.1 structures whose layout the API fixes.
 *
 * Member names and types follow WINDOWS.H (tagRECT, tagPOINT, tagPAINTSTRUCT,
 * tagPALETTEENTRY, tagSIZE, tagLOGFONT). The Windows version of the game uses them; records.h
 * needs RECT. The editor itself uses none of them.
 */
#ifndef W16TYPES_H
#define W16TYPES_H

typedef struct { i16 x, y; } POINT;

typedef struct { i16 left, top, right, bottom; } RECT;

typedef struct {
    u16 hdc;
    i16 fErase;
    RECT rcPaint;
    i16 fRestore, fIncUpdate;
    u8 rgbReserved[16];
} PAINTSTRUCT;

typedef struct { u8 peRed, peGreen, peBlue, peFlags; } PALETTEENTRY;

typedef struct { i16 cx, cy; } SIZE;

/* LF_FACESIZE is 32. */
typedef struct {
    i16 lfHeight, lfWidth, lfEscapement, lfOrientation, lfWeight;
    u8 lfItalic, lfUnderline, lfStrikeOut, lfCharSet,
                  lfOutPrecision, lfClipPrecision, lfQuality, lfPitchAndFamily;
    char lfFaceName[32];
} LOGFONT;

/* The high byte of the word w as it lies in memory (little-endian), read as a byte. WINDOWS.H's
 * HIBYTE shifts a value instead. */
#define HI_BYTE(w)  (((u8 *)&(w))[1])

#endif
