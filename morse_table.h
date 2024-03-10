#ifndef MORSE_TABLE_H
#define MORSE_TABLE_H

#include "QtCore/qmap.h"
typedef struct {
    char letter;
    int duration;   // duration in dits
} morse_table_t;

// Based upon a 50 dot duration standard word such as PARIS, the time for one dit
//   duration or one unit can be computed by the formula:
//   T = 1200 / Speed(wpm) where T is the dit duration.
//         11   3  5  3  7  3  3   3  5   7  = 50
//   PARIS  .__.    ._    ._.    ..    ...

QMap<char, int> morseTimingMap {
    {'A', 8},     // ._  1 dit, 1 intercharacter, 3 dah, 3 after char (inter char spacing)
    {'B', 12},    // _...
    {'C', 14},    //_._.
    {'D', 10},    // _..
    {'E', 4},     // .
    {'F', 12},    // .._.
    {'G', 12},    // __.
    {'H', 10},    // ....
    {'I', 6},     // ..
    {'J', 16},    // .___
    {'K', 12},    // _._
    {'L', 12},    // ._..
    {'M', 10},    // __
    {'N', 8},     // _.
    {'O', 14},    // ___
    {'P', 14},    // .__.
    {'Q', 16},    // __._
    {'R', 10},    // ._.
    {'S', 9},     // ...
    {'T', 6},     // _
    {'U', 10},    // .._
    {'V', 12},    // ..._
    {'W', 12},    // .__
    {'X', 14},    // .__.
    {'Y', 16},    // _.__
    {'Z', 14},    // __..
    {'1', 20},    // .____
    {'2', 18},    // ..___
    {'3', 16},    // ...__
    {'4', 14},    // ...._
    {'5', 12},    // .....
    {'6', 14},    // _....
    {'7', 16},    // __...
    {'8', 18},    // ___..
    {'9', 20},    // ____.
    {'0', 22},    // _____
    {'?', 18},    // ..__..
    {'.', 20},    // ._._._
    {' ', 7},     // space is 7 elements
    {'=', 16},    // _..._  Prosign BT
    {'+', 16},    // ._._.  Prosign AR
    {'%', 14},    // ._...  Prosign AS
    {'*', 18},    // ..._._  Prosign SK
    {',', 22},    // __..__
    {'/', 16},    // _.._.
    {'\n', 0}     // Enter key - need to treat it as valid and counted to keep tx_position in sync
};

#endif // MORSE_TABLE_H
