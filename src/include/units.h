/* units.h -- unit type ids, standing orders, and how the game reads its Unit record.
 *
 * The game's own constants, shared with the editor, which uses none of them. A Unit
 * (records.h) is one unit on the map; its type is an index into the unit type table, whose
 * names are the game's UNIT0..UNIT23 texts.
 */
#ifndef UNITS_H
#define UNITS_H

#define UNIT_COLONISTS              0
#define UNIT_SOLDIERS               1
#define UNIT_PIONEERS               2
#define UNIT_MISSIONARIES           3
#define UNIT_DRAGOONS               4
#define UNIT_SCOUTS                 5
#define UNIT_REGULARS               6
#define UNIT_CONTINENTAL_CAVALRY    7
#define UNIT_CAVALRY                8
#define UNIT_CONTINENTAL_ARMY       9
#define UNIT_TREASURE_TRAIN         10
#define UNIT_ARTILLERY              11
#define UNIT_WAGON_TRAIN            12
#define UNIT_CARAVEL                13
#define UNIT_MERCHANTMAN            14
#define UNIT_GALLEON                15
#define UNIT_PRIVATEER              16
#define UNIT_FRIGATE                17
#define UNIT_MAN_O_WAR              18
#define UNIT_BRAVES                 19
#define UNIT_ARMED_BRAVES           20
#define UNIT_MOUNTED_BRAVES         21
#define UNIT_MOUNTED_WARRIORS       22
/* The scratch type: the game rewrites its table entry for each throwaway unit it makes -- a
 * fort's guns, the AI's one-turn work unit -- so it describes whatever unit was made last,
 * not a unit of the game. */
#define UNIT_TEMPORARY              23

#define UNIT_SHIP_FIRST             UNIT_CARAVEL
#define UNIT_SHIP_LAST              UNIT_MAN_O_WAR

/* ---- Unit.orders: the standing order ---------------------------------------------------- */
#define ORDERS_NONE                 0
#define ORDERS_SENTRY               1       /* "Sentry / Board ship" */
#define ORDERS_TRADE_ROUTE          2       /* following a trade route */
#define ORDERS_FORTIFY              5       /* fortifying */
#define ORDERS_FORTIFIED            6       /* fortified, which counts in defence */
#define ORDERS_BUILD_COLONY         7       /* founding a colony */
#define ORDERS_PLOW                 8       /* clearing or plowing */
#define ORDERS_BUILD_ROAD           9       /* building a road */
#define ORDERS_MISSION              11      /* on a mission: see Unit.mission below */

/* Orders 3, 10 and 12 occur and are not named:
 *   3   set on arrival together with a destination (dest_x, dest_y), like a mission without a
 *       mission code; the game's order dispatcher has no case for it.
 *  10   moves the unit one step a turn toward its destination and clears the order there;
 *       no code that sets it is known.
 *  12   set by the AI with a target in dest_x, dest_y when it picks a direction; the move is
 *       not shown, and arriving keeps this order (and 2) where it clears the others. Probably
 *       "the AI is moving this unit".
 */

/* ---- the unit type table, one row of the UNITS text table per type ---------------------
 * The table's columns are name, icon, moves, attack, defence, holds, ...; the record keeps
 * defence before attack.
 *   name      the text id
 *   moves     in thirds of a square: the table's value times 3. A ship of a nation with
 *             Ferdinand Magellan gets another 3.
 *   defence   also a damaged ship's repair time (see Unit.counter)
 *   holds     cargo slots: 0 for artillery, 2 for a caravel, 6 for a man-o-war
 *   size      the holds this unit takes up when something carries it; 99 means it cannot be
 *             carried (every ship and the wagon train). A treasure train's is 6, which is why
 *             it needs a galleon.
 */

/* ---- Unit.counter: one counter for whatever the unit is waiting out --------------------
 *   a pioneer        turns of work left on a clearing, a road or plowing
 *   a ship at sea    turns until it reaches Europe
 *   a damaged ship   the repair time: it is repaired when its type's defence - counter
 *                    reaches 0, and the icon shows that difference
 *   a convert        turns before its faith lapses
 *   a native         turns idle before the tribe acts with it
 */

/* ---- Unit.spec: what it holds depends on the unit --------------------------------------
 *   a colonist   its specialty (a job number, 0..28)
 *   a treasure   its value in hundreds of gold, so a treasure is at most 25,500
 *   a carrier    its trade route: the route in the low nibble, the destination in the high
 * No unit has two of these: only land units without cargo holds get a specialty, and those
 * carry neither a route nor treasure.
 */

/* ---- Unit.mission: a second standing-order byte ----------------------------------------
 * With ORDERS_MISSION the mission code, and dest_x, dest_y the destination. The AI uses codes
 * 0x21..0x57; nothing in the game's data lists them, and they are not named.
 */

/* A unit whose x is UNIT_EUROPE_X + its nation is in that nation's Europe. */
#define UNIT_EUROPE_X               0xec

/* ---- Unit.flags ------------------------------------------------------------------------
 * The low nibble is the owning nation. The high nibble is who has seen the unit, one bit per
 * European nation, laid out like the map's sight plane (terrain.h's MAP3_NATION_BIT): the
 * game draws a unit only for a viewer whose bit is set, or with fog of war off.
 */
#define UNIT_NATION_MASK            0x0f
/* The same bit as MAP3_NATION_BIT, written the way the game writes it for a unit's flags. */
#define UNIT_SEEN_BY(n)             (0x10 << (n))

#define UNIT_RECORD_SIZE            0x1c    /* the size of a Unit record */
#define UNIT_NONE                   (-1)

#endif
