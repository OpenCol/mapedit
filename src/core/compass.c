/* compass.c -- the directions as x and y steps, for the map view's neighbour scans.
 *
 * Two pairs of tables, x steps and y steps (y grows downwards):
 *   card_x/y    the four orthogonal neighbours: north, east, south, west
 *   move_x/y    the eight neighbours, clockwise from north
 * Each table ends with two zero entries, as in the original. The map view walks them when it
 * draws coasts and seams.
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

