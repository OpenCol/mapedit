/* compass.c -- the directions as x and y steps, for the map view's neighbour scans.
 *
 * Three pairs of tables, x steps and y steps (y grows downwards):
 *   card_x/y    the four orthogonal neighbours: north, east, south, west
 *   move_x/y    the eight neighbours, clockwise from north
 *   radius_x/y  the 20 squares within two steps, nearest first: the four orthogonal, the four
 *               diagonal, the four two away in a straight line, then the eight knight's moves
 * Each table ends with two zero entries, as in the original. The map view walks card and
 * move when it draws coasts and seams; nothing in the editor uses radius.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "loader.h"
#include "map.h"

i8 card_x[6] = { 0, 1, 0, -1, 0, 0 };
i8 card_y[6] = { -1, 0, 1, 0, 0, 0 };
i8 move_x[10] = { 0, 1, 1, 1, 0, -1, -1, -1, 0, 0 };
i8 move_y[10] = { -1, -1, 0, 1, 1, 1, 0, -1, 0, 0 };
i8 radius_x[22] = { 0, 1, 0, -1, -1, 1, 1, -1, 0, 2, 0, -2, -1, 1, -1, 1, -2, -2, 2, 2, 0, 0 };
i8 radius_y[22] = { -1, 0, 1, 0, -1, -1, 1, 1, -2, 0, 2, 0, -2, -2, 2, 2, -1, 1, -1, 1, 0, 0 };

