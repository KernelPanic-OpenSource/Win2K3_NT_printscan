/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    faxtable.h

Abstract:

    Tables used for Group3 fax compression

Environment:

	Windows XP Fax driver, kernel mode

Revision History:

	01/23/96 -davidx-
		Created it.

	dd-mm-yy -author-
		description

--*/

#ifndef _FAXTABLE_H_
#define _FAXTABLE_H_

//
// Data structure for representing code table entries
//

typedef struct {

    WORD    length;     // code length
    WORD    code;       // code word itself

} CODETABLE;

typedef const CODETABLE *PCODETABLE;

//
// Code word for end-of-line (EOL)
//  000000000001
//

#define EOL_CODE        1
#define EOL_LENGTH      12

//
// Code word for 2D encoding - pass mode
//  0001
//

#define PASSCODE        1
#define PASSCODE_LENGTH 4

//
// Code word for 2D encoding - horizontal mode prefix
//  001
//

#define HORZCODE        1
#define HORZCODE_LENGTH 3

//
// Code word for 2D encoding - vertical mode
//

static const CODETABLE VertCodes[] = {

    {  7, 0x02 },       // 0000010         VL3
    {  6, 0x02 },       // 000010          VL2
    {  3, 0x02 },       // 010             VL1
    {  1, 0x01 },       // 1                V0
    {  3, 0x03 },       // 011             VR1
    {  6, 0x03 },       // 000011          VR2
    {  7, 0x03 },       // 0000011         VR3
};

//
// Code table for white runs
//

static const CODETABLE WhiteRunCodes[] = {
    
    {  8, 0x35 },       // 00110101          0
    {  6, 0x07 },       // 000111            1
    {  4, 0x07 },       // 0111              2
    {  4, 0x08 },       // 1000              3
    {  4, 0x0b },       // 1011              4
    {  4, 0x0c },       // 1100              5
    {  4, 0x0e },       // 1110              6
    {  4, 0x0f },       // 1111              7
    {  5, 0x13 },       // 10011             8
    {  5, 0x14 },       // 10100             9
    {  5, 0x07 },       // 00111            10
    {  5, 0x08 },       // 01000            11
    {  6, 0x08 },       // 001000           12
    {  6, 0x03 },       // 000011           13
    {  6, 0x34 },       // 110100           14
    {  6, 0x35 },       // 110101           15
    {  6, 0x2a },       // 101010           16
    {  6, 0x2b },       // 101011           17
    {  7, 0x27 },       // 0100111          18
    {  7, 0x0c },       // 0001100          19
    {  7, 0x08 },       // 0001000          20
    {  7, 0x17 },       // 0010111          21
    {  7, 0x03 },       // 0000011          22
    {  7, 0x04 },       // 0000100          23
    {  7, 0x28 },       // 0101000          24
    {  7, 0x2b },       // 0101011          25
    {  7, 0x13 },       // 0010011          26
    {  7, 0x24 },       // 0100100          27
    {  7, 0x18 },       // 0011000          28
    {  8, 0x02 },       // 00000010         29
    {  8, 0x03 },       // 00000011         30
    {  8, 0x1a },       // 00011010         31
    {  8, 0x1b },       // 00011011         32
    {  8, 0x12 },       // 00010010         33
    {  8, 0x13 },       // 00010011         34
    {  8, 0x14 },       // 00010100         35
    {  8, 0x15 },       // 00010101         36
    {  8, 0x16 },       // 00010110         37
    {  8, 0x17 },       // 00010111         38
    {  8, 0x28 },       // 00101000         39
    {  8, 0x29 },       // 00101001         40
    {  8, 0x2a },       // 00101010         41
    {  8, 0x2b },       // 00101011         42
    {  8, 0x2c },       // 00101100         43
    {  8, 0x2d },       // 00101101         44
    {  8, 0x04 },       // 00000100         45
    {  8, 0x05 },       // 00000101         46
    {  8, 0x0a },       // 00001010         47
    {  8, 0x0b },       // 00001011         48
    {  8, 0x52 },       // 01010010         49
    {  8, 0x53 },       // 01010011         50
    {  8, 0x54 },       // 01010100         51
    {  8, 0x55 },       // 01010101         52
    {  8, 0x24 },       // 00100100         53
    {  8, 0x25 },       // 00100101         54
    {  8, 0x58 },       // 01011000         55
    {  8, 0x59 },       // 01011001         56
    {  8, 0x5a },       // 01011010         57
    {  8, 0x5b },       // 01011011         58
    {  8, 0x4a },       // 01001010         59
    {  8, 0x4b },       // 01001011         60
    {  8, 0x32 },       // 00110010         61
    {  8, 0x33 },       // 00110011         62
    {  8, 0x34 },       // 00110100         63
    {  5, 0x1b },       // 11011            64
    {  5, 0x12 },       // 10010           128
    {  6, 0x17 },       // 010111          192
    {  7, 0x37 },       // 0110111         256
    {  8, 0x36 },       // 00110110        320
    {  8, 0x37 },       // 00110111        384
    {  8, 0x64 },       // 01100100        448
    {  8, 0x65 },       // 01100101        512
    {  8, 0x68 },       // 01101000        576
    {  8, 0x67 },       // 01100111        640
    {  9, 0xcc },       // 011001100       704
    {  9, 0xcd },       // 011001101       768
    {  9, 0xd2 },       // 011010010       832
    {  9, 0xd3 },       // 011010011       896
    {  9, 0xd4 },       // 011010100       960
    {  9, 0xd5 },       // 011010101      1024
    {  9, 0xd6 },       // 011010110      1088
    {  9, 0xd7 },       // 011010111      1152
    {  9, 0xd8 },       // 011011000      1216
    {  9, 0xd9 },       // 011011001      1280
    {  9, 0xda },       // 011011010      1344
    {  9, 0xdb },       // 011011011      1408
    {  9, 0x98 },       // 010011000      1472
    {  9, 0x99 },       // 010011001      1536
    {  9, 0x9a },       // 010011010      1600
    {  6, 0x18 },       // 011000         1664
    {  9, 0x9b },       // 010011011      1728
    { 11, 0x08 },       // 00000001000    1792
    { 11, 0x0c },       // 00000001100    1856
    { 11, 0x0d },       // 00000001101    1920
    { 12, 0x12 },       // 000000010010   1984
    { 12, 0x13 },       // 000000010011   2048
    { 12, 0x14 },       // 000000010100   2112
    { 12, 0x15 },       // 000000010101   2176
    { 12, 0x16 },       // 000000010110   2240
    { 12, 0x17 },       // 000000010111   2304
    { 12, 0x1c },       // 000000011100   2368
    { 12, 0x1d },       // 000000011101   2432
    { 12, 0x1e },       // 000000011110   2496
    { 12, 0x1f },       // 000000011111   2560
};

//
// Code table for black runs
//

static const CODETABLE BlackRunCodes[] = {
    
    { 10, 0x37 },       // 0000110111        0
    {  3, 0x02 },       // 010               1
    {  2, 0x03 },       // 11                2
    {  2, 0x02 },       // 10                3
    {  3, 0x03 },       // 011               4
    {  4, 0x03 },       // 0011              5
    {  4, 0x02 },       // 0010              6
    {  5, 0x03 },       // 00011             7
    {  6, 0x05 },       // 000101            8
    {  6, 0x04 },       // 000100            9
    {  7, 0x04 },       // 0000100          10
    {  7, 0x05 },       // 0000101          11
    {  7, 0x07 },       // 0000111          12
    {  8, 0x04 },       // 00000100         13
    {  8, 0x07 },       // 00000111         14
    {  9, 0x18 },       // 000011000        15
    { 10, 0x17 },       // 0000010111       16
    { 10, 0x18 },       // 0000011000       17
    { 10, 0x08 },       // 0000001000       18
    { 11, 0x67 },       // 00001100111      19
    { 11, 0x68 },       // 00001101000      20
    { 11, 0x6c },       // 00001101100      21
    { 11, 0x37 },       // 00000110111      22
    { 11, 0x28 },       // 00000101000      23
    { 11, 0x17 },       // 00000010111      24
    { 11, 0x18 },       // 00000011000      25
    { 12, 0xca },       // 000011001010     26
    { 12, 0xcb },       // 000011001011     27
    { 12, 0xcc },       // 000011001100     28
    { 12, 0xcd },       // 000011001101     29
    { 12, 0x68 },       // 000001101000     30
    { 12, 0x69 },       // 000001101001     31
    { 12, 0x6a },       // 000001101010     32
    { 12, 0x6b },       // 000001101011     33
    { 12, 0xd2 },       // 000011010010     34
    { 12, 0xd3 },       // 000011010011     35
    { 12, 0xd4 },       // 000011010100     36
    { 12, 0xd5 },       // 000011010101     37
    { 12, 0xd6 },       // 000011010110     38
    { 12, 0xd7 },       // 000011010111     39
    { 12, 0x6c },       // 000001101100     40
    { 12, 0x6d },       // 000001101101     41
    { 12, 0xda },       // 000011011010     42
    { 12, 0xdb },       // 000011011011     43
    { 12, 0x54 },       // 000001010100     44
    { 12, 0x55 },       // 000001010101     45
    { 12, 0x56 },       // 000001010110     46
    { 12, 0x57 },       // 000001010111     47
    { 12, 0x64 },       // 000001100100     48
    { 12, 0x65 },       // 000001100101     49
    { 12, 0x52 },       // 000001010010     50
    { 12, 0x53 },       // 000001010011     51
    { 12, 0x24 },       // 000000100100     52
    { 12, 0x37 },       // 000000110111     53
    { 12, 0x38 },       // 000000111000     54
    { 12, 0x27 },       // 000000100111     55
    { 12, 0x28 },       // 000000101000     56
    { 12, 0x58 },       // 000001011000     57
    { 12, 0x59 },       // 000001011001     58
    { 12, 0x2b },       // 000000101011     59
    { 12, 0x2c },       // 000000101100     60
    { 12, 0x5a },       // 000001011010     61
    { 12, 0x66 },       // 000001100110     62
    { 12, 0x67 },       // 000001100111     63
    { 10, 0x0f },       // 0000001111       64
    { 12, 0xc8 },       // 000011001000    128
    { 12, 0xc9 },       // 000011001001    192
    { 12, 0x5b },       // 000001011011    256
    { 12, 0x33 },       // 000000110011    320
    { 12, 0x34 },       // 000000110100    384
    { 12, 0x35 },       // 000000110101    448
    { 13, 0x6c },       // 0000001101100   512
    { 13, 0x6d },       // 0000001101101   576
    { 13, 0x4a },       // 0000001001010   640
    { 13, 0x4b },       // 0000001001011   704
    { 13, 0x4c },       // 0000001001100   768
    { 13, 0x4d },       // 0000001001101   832
    { 13, 0x72 },       // 0000001110010   896
    { 13, 0x73 },       // 0000001110011   960
    { 13, 0x74 },       // 0000001110100  1024
    { 13, 0x75 },       // 0000001110101  1088
    { 13, 0x76 },       // 0000001110110  1152
    { 13, 0x77 },       // 0000001110111  1216
    { 13, 0x52 },       // 0000001010010  1280
    { 13, 0x53 },       // 0000001010011  1344
    { 13, 0x54 },       // 0000001010100  1408
    { 13, 0x55 },       // 0000001010101  1472
    { 13, 0x5a },       // 0000001011010  1536
    { 13, 0x5b },       // 0000001011011  1600
    { 13, 0x64 },       // 0000001100100  1664
    { 13, 0x65 },       // 0000001100101  1728
    { 11, 0x08 },       // 00000001000    1792
    { 11, 0x0c },       // 00000001100    1856
    { 11, 0x0d },       // 00000001101    1920
    { 12, 0x12 },       // 000000010010   1984
    { 12, 0x13 },       // 000000010011   2048
    { 12, 0x14 },       // 000000010100   2112
    { 12, 0x15 },       // 000000010101   2176
    { 12, 0x16 },       // 000000010110   2240
    { 12, 0x17 },       // 000000010111   2304
    { 12, 0x1c },       // 000000011100   2368
    { 12, 0x1d },       // 000000011101   2432
    { 12, 0x1e },       // 000000011110   2496
    { 12, 0x1f },       // 000000011111   2560
};

#ifdef USELSB

//
// Table used to reverse fill order from MSB to LSB
//

static const BYTE BitReverseTable[256] = {

    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

#endif

#endif	// !_FAXTABLE_H_

