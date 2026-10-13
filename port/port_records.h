/* port_records.h -- the records several modules share, one definition each.
 *
 * In 1994 each module declared the records it used, and the declarations agreed in size
 * but not always in shape: a work buffer's DosPort was { h, w, bits } in seventeen modules,
 * { h, w, void far *bits } in fourteen and { h, w, off, seg } in two, all eight bytes. In
 * the port a pointer is not four bytes, and a DosPort passed by value between modules must
 * have one layout, so the import drops the modules' own definitions of these names and every
 * module sees this one (tools/import.py, CANONICAL).
 */
#ifndef MAPEDIT_PORT_RECORDS_H
#define MAPEDIT_PORT_RECORDS_H

/* a work buffer: h rows of w pixels (palette indices), row-major */
typedef struct {
    i16 h;
    i16 w;
    char *bits;
} DosPort;

#endif
