/*
 * A n t l r  S e t s / E r r o r  F i l e  H e a d e r
 *
 * Generated from: antlr.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-2001
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"

#include "pcctscfg.h"
#include "set.h"
#include <ctype.h>
#include "syn.h"
#include "hash.h"
#include "generic.h"
#define zzcr_attr(attr,tok,t)
#define zzSET_SIZE 20
#include "antlr.h"
#include "tokens.h"
#include "dlgdef.h"
#include "err.h"

ANTLRChar *zztokens[157]={
	/* 00 */	"Invalid",
	/* 01 */	"Eof",
	/* 02 */	"QuotedTerm",
	/* 03 */	"\\n|\\r|\\r\\n",
	/* 04 */	"\\(\\n|\\r|\\r\\n)",
	/* 05 */	"\\~[]",
	/* 06 */	"~[\\n\\r\"\\]+",
	/* 07 */	"\"",
	/* 08 */	"\\n|\\r|\\r\\n",
	/* 09 */	"\\(\\n|\\r|\\r\\n)",
	/* 10 */	"\\~[]",
	/* 11 */	"~[\\n\\r\"\\]+",
	/* 12 */	"'",
	/* 13 */	"\\n|\\r|\\r\\n",
	/* 14 */	"\\~[]",
	/* 15 */	"~[\\n\\r'\\]+",
	/* 16 */	"\\*/",
	/* 17 */	"\\*",
	/* 18 */	"\\n|\\r|\\r\\n",
	/* 19 */	"~[\\n\\r\\*]+",
	/* 20 */	"\\*/",
	/* 21 */	"\\*",
	/* 22 */	"\\n|\\r|\\r\\n",
	/* 23 */	"~[\\n\\r\\*]+",
	/* 24 */	"\\n|\\r|\\r\\n",
	/* 25 */	"~[\\n\\r]+",
	/* 26 */	"\\n|\\r|\\r\\n",
	/* 27 */	"~[\\n\\r]+",
	/* 28 */	"\\n|\\r|\\r\\n",
	/* 29 */	"~[\\n\\r]+",
	/* 30 */	"\\*/",
	/* 31 */	"\\*",
	/* 32 */	"\\n|\\r|\\r\\n",
	/* 33 */	"~[\\n\\r\\*]+",
	/* 34 */	"Action",
	/* 35 */	"Pred",
	/* 36 */	"PassAction",
	/* 37 */	"consumeUntil\\( [\\ \\t]* \\{~[\\}]+\\} [\\ \\t]* \\)",
	/* 38 */	"consumeUntil\\( ~[\\)]+ \\)",
	/* 39 */	"\\n|\\r|\\r\\n",
	/* 40 */	"\\>",
	/* 41 */	"$",
	/* 42 */	"$$",
	/* 43 */	"$\\[\\]",
	/* 44 */	"$\\[",
	/* 45 */	"$[0-9]+",
	/* 46 */	"$[0-9]+.",
	/* 47 */	"$[0-9]+.[0-9]+",
	/* 48 */	"$[_a-zA-Z][_a-zA-Z0-9]*",
	/* 49 */	"#0",
	/* 50 */	"#\\[\\]",
	/* 51 */	"#\\(\\)",
	/* 52 */	"#[0-9]+",
	/* 53 */	"#line[\\ \\t]* [0-9]+ {[\\ \\t]* \"~[\"]+\" ([\\ \\t]* [0-9]*)* } (\\n|\\r|\\r\\n)",
	/* 54 */	"#line ~[\\n\\r]* (\\n|\\r|\\r\\n)",
	/* 55 */	"#[_a-zA-Z][_a-zA-Z0-9]*",
	/* 56 */	"#\\[",
	/* 57 */	"#\\(",
	/* 58 */	"#",
	/* 59 */	"\\)",
	/* 60 */	"\\[",
	/* 61 */	"\\(",
	/* 62 */	"\\\\]",
	/* 63 */	"\\\\)",
	/* 64 */	"\\>",
	/* 65 */	"'",
	/* 66 */	"\"",
	/* 67 */	"\\$",
	/* 68 */	"\\#",
	/* 69 */	"\\(\\n|\\r|\\r\\n)",
	/* 70 */	"\\~[\\]\\)>$#]",
	/* 71 */	"/",
	/* 72 */	"/\\*",
	/* 73 */	"\\*/",
	/* 74 */	"//",
	/* 75 */	"~[\\n\\r\\)\\(\\$#\\>\\]\\[\"'/]+",
	/* 76 */	"[\\t\\ ]+",
	/* 77 */	"\\n|\\r|\\r\\n",
	/* 78 */	"\\[",
	/* 79 */	"\\<\\<",
	/* 80 */	"\"",
	/* 81 */	"/\\*",
	/* 82 */	"\\*/",
	/* 83 */	"//",
	/* 84 */	"#line[\\ \\t]* [0-9]+ {[\\ \\t]* \"~[\"]+\" ([\\ \\t]* [0-9]*)* } (\\n|\\r|\\r\\n)",
	/* 85 */	"#line ~[\\n\\r]* (\\n|\\r|\\r\\n)",
	/* 86 */	"\\>\\>",
	/* 87 */	"WildCard",
	/* 88 */	"\\@",
	/* 89 */	"LABEL",
	/* 90 */	"grammar-element",
	/* 91 */	"meta-symbol",
	/* 92 */	"Pragma",
	/* 93 */	"FirstSetSymbol",
	/* 94 */	"{\\}#header",
	/* 95 */	"{\\}#first",
	/* 96 */	"{\\}#parser",
	/* 97 */	"{\\}#tokdefs",
	/* 98 */	"\\}",
	/* 99 */	"class",
	/* 100 */	"NonTerminal",
	/* 101 */	"TokenTerm",
	/* 102 */	"\\{",
	/* 103 */	"!",
	/* 104 */	"\\<",
	/* 105 */	"\\>",
	/* 106 */	":",
	/* 107 */	";",
	/* 108 */	"{\\}#lexaction",
	/* 109 */	"{\\}#lexmember",
	/* 110 */	"{\\}#lexprefix",
	/* 111 */	"{\\}#pred",
	/* 112 */	"\\|\\|",
	/* 113 */	"&&",
	/* 114 */	"\\(",
	/* 115 */	"\\)",
	/* 116 */	"{\\}#lexclass",
	/* 117 */	"{\\}#errclass",
	/* 118 */	"{\\}#tokclass",
	/* 119 */	"..",
	/* 120 */	"{\\}#token",
	/* 121 */	"=",
	/* 122 */	"[0-9]+",
	/* 123 */	"\\|",
	/* 124 */	"\\~",
	/* 125 */	"^",
	/* 126 */	"approx",
	/* 127 */	"LL\\(1\\)",
	/* 128 */	"LL\\(2\\)",
	/* 129 */	"\\*",
	/* 130 */	"\\+",
	/* 131 */	"?",
	/* 132 */	"=>",
	/* 133 */	"exception",
	/* 134 */	"default",
	/* 135 */	"catch",
	/* 136 */	"{\\}#[A-Za-z0-9_]*",
	/* 137 */	"[\\t\\ ]+",
	/* 138 */	"\\n|\\r|\\r\\n",
	/* 139 */	"//",
	/* 140 */	"/\\*",
	/* 141 */	"#ifdef",
	/* 142 */	"#if",
	/* 143 */	"#ifndef",
	/* 144 */	"#else",
	/* 145 */	"#endif",
	/* 146 */	"#undef",
	/* 147 */	"#import",
	/* 148 */	"ID",
	/* 149 */	"#define",
	/* 150 */	"INT",
	/* 151 */	"enum",
	/* 152 */	"\\{",
	/* 153 */	"=",
	/* 154 */	",",
	/* 155 */	"\\}",
	/* 156 */	";"
};
SetWordType zzerr1[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr2[20] = {0xfc,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xf3,
	0xbf,0xff,0xff,0xff, 0xff,0xff,0xff,0x1f};
SetWordType zzerr3[20] = {0xfc,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xfb,
	0x3b,0xf7,0xf7,0xc7, 0xff,0xff,0xff,0x1f};
SetWordType zzerr4[20] = {0x4,0x0,0x0,0x0, 0x10,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x80,0x7,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType setwd1[157] = {0x0,0x50,0xa0,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x6a,0x20,0xa0,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x0,0x0,0x20,0x20,0x21,
	0x21,0x21,0x21,0x6e,0x6e,0x64,0x20,0x0,
	0x20,0xa0,0xa0,0xa0,0x20,0x6a,0x6a,0x6a,
	0x6e,0x20,0x20,0x20,0x20,0x66,0x6e,0x6e,
	0x20,0x66,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x62,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20};
SetWordType zzerr5[20] = {0x0,0x0,0x0,0x0, 0x10,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x1,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr6[20] = {0x4,0x0,0x0,0x0, 0x10,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x7,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr7[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x6,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr8[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x4,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr9[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf0,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType setwd2[157] = {0x0,0xf8,0x6,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xf8,0x0,0x1,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xf8,0xf8,0xf8,0x0,0x0,
	0x0,0x1,0x2,0x6,0x0,0xf8,0xf8,0xf8,
	0xf8,0x0,0x0,0x0,0x0,0xf8,0xf8,0xf8,
	0x0,0xf8,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0xe8,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr10[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0xbc,0xf8,0x74,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr11[20] = {0x0,0x0,0x0,0x0, 0x8,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0xa0,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr12[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr13[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0xa0,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType setwd3[157] = {0x0,0xfa,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xfa,0xfa,0xfa,0x5,0x0,
	0x5,0x0,0x0,0x0,0xe2,0xfa,0xfa,0xfa,
	0xfa,0xc0,0x80,0x5,0xe0,0xfa,0xfa,0xfa,
	0x0,0xfa,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0xfa,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr14[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x20,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr15[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr16[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr17[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x40,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr18[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x24,0x0,0x80,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr19[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x20,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr20[20] = {0x6,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x74,0x3, 0x20,0x0,0x0,0x0};
SetWordType zzerr21[20] = {0x6,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x70,0x3, 0x20,0x0,0x0,0x0};
SetWordType setwd4[157] = {0x0,0xe5,0xda,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xe5,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xed,0xe5,0xe7,0x1a,0x0,
	0x0,0x0,0x0,0x0,0xc0,0xe5,0xe5,0xe5,
	0xe5,0x0,0x0,0x0,0x0,0xe5,0xe5,0xe5,
	0x0,0xe5,0x40,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr22[20] = {0x6,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x3c,0xf8,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr23[20] = {0x6,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr24[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr25[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x70,0x1, 0x20,0x0,0x0,0x0};
SetWordType zzerr26[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x5,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType setwd5[157] = {0x0,0x1f,0xc1,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xdf,0xc0,0xc0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xc0,0x0,0xc0,0x0,0x0,0xc0,0xc0,0x0,
	0x0,0x0,0x0,0x7f,0x1f,0xdf,0xc0,0xc0,
	0x0,0x0,0xc0,0x0,0x67,0x1f,0x1f,0x1f,
	0x1f,0x0,0x0,0xc0,0x60,0x1f,0x1f,0x1f,
	0x0,0x1f,0x0,0x0,0x40,0xc0,0x0,0x0,
	0x0,0x0,0xc0,0xc0,0x0,0x0,0x5f,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr27[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x0,0x0,0x0,0x10, 0x0,0x0,0x0,0x0};
SetWordType zzerr28[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x80,0x2,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr29[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x20,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr30[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xd,
	0x0,0x0,0x80,0x0, 0x20,0x0,0x0,0x0};
SetWordType zzerr31[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xd,
	0x0,0x0,0x0,0x0, 0x20,0x0,0x0,0x0};
SetWordType zzerr32[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x5,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr33[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x20,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType setwd6[157] = {0x0,0x0,0xfd,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xe1,0xe1,0xe1,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xfd,0x60,0xe9,0x0,0x0,0xe1,0xe1,0x0,
	0x0,0x0,0x0,0xe2,0x0,0xfd,0xfd,0xe1,
	0x20,0x0,0xe1,0x0,0xe2,0x0,0x0,0x0,
	0x0,0x0,0x0,0xe1,0xe2,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0xe2,0xe0,0x20,0x0,
	0x0,0x0,0xe1,0xe1,0x0,0x0,0xe2,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr34[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xd,
	0x0,0x0,0x80,0x0, 0x20,0x0,0x0,0x0};
SetWordType zzerr35[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xd,
	0x0,0x0,0x0,0x0, 0x20,0x0,0x0,0x0};
SetWordType zzerr36[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x5,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr37[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xc,
	0x0,0x0,0x0,0x0, 0x20,0x0,0x0,0x0};
SetWordType zzerr38[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x84,0x9,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr39[20] = {0x0,0x0,0x0,0x0, 0x10,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x1,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr40[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x9,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr41[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr42[20] = {0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x80,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType setwd7[157] = {0x0,0x0,0xdf,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xdf,0xdf,0xff,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xdf,0x3,0xdf,0x0,0x0,0xdf,0xdf,0x0,
	0x0,0x0,0x0,0xdf,0x0,0xdf,0xdf,0xdf,
	0x1,0x30,0xdf,0x0,0xdf,0x0,0x0,0x0,
	0x0,0x0,0x0,0xdf,0xdf,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0xdf,0xdf,0x1,0x0,
	0x0,0x0,0xdf,0xdf,0x0,0x0,0xdf,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr43[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr44[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xc0, 0x1,0x0,0x0,0x0};
SetWordType zzerr45[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x30,
	0x40,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr46[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr47[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x20,
	0x40,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr48[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x2,0x0, 0x10,0x0,0x0,0x0};
SetWordType zzerr49[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x8,0x8,0x18, 0x20,0x0,0x0,0x0};
SetWordType zzerr50[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x8,0xa,0x18, 0x30,0x0,0x0,0x0};
SetWordType zzerr51[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x4,0x8,0x8,0x18, 0x28,0x0,0x0,0x0};
SetWordType zzerr52[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x40,0x0,0x4,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr53[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x4,
	0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType setwd8[157] = {0x0,0x0,0xe1,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0xe1,0xe1,0xe1,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xe1,0x0,0xe1,0x0,0x0,0xe3,0xe7,0x0,
	0x0,0x0,0x0,0xe1,0x0,0xe1,0xe1,0xef,
	0x0,0x0,0xe1,0x0,0xe1,0x0,0x0,0x0,
	0x0,0x0,0x10,0xef,0xe1,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0xe1,0xe1,0x0,0x0,
	0x0,0x0,0xe1,0xe1,0x0,0x10,0xe1,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr54[20] = {0x2,0x0,0x0,0x0, 0x14,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x78,0x9, 0xe0,0x0,0x0,0x0};
SetWordType zzerr55[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x78,0x9, 0x60,0x0,0x0,0x0};
SetWordType zzerr56[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x30,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
SetWordType zzerr57[20] = {0x2,0x0,0x0,0x0, 0x4,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x1c,0xf8,0x78,0x9, 0xe0,0x0,0x0,0x0};
SetWordType setwd9[157] = {0x0,0x7c,0x1,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x7f,0x1,0x1,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x1,0x0,0x1,0x0,0x0,0x1,0x1,0x0,
	0x0,0x0,0x0,0x7f,0x7e,0x7f,0x1,0x1,
	0x0,0x0,0x1,0x0,0x7d,0x7e,0x7e,0x7e,
	0x7e,0x0,0x0,0x1,0x7d,0x7e,0x7e,0x7e,
	0x0,0x7e,0x0,0x0,0x7d,0x1,0x0,0x0,
	0x0,0x0,0x1,0x1,0x0,0x0,0x7f,0x64,
	0x64,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x80,0x0,0x0,0x0,0x0,0x0,0x80,0x0,
	0x80,0x0,0x0,0x0,0x0,0x0};
SetWordType zzerr58[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0xa0,0x0};
SetWordType zzerr59[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x80,0xa0,0x0};
SetWordType zzerr60[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0xa0,0x0};
SetWordType zzerr61[20] = {0x2,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x80,0xa0,0x0};
SetWordType zzerr62[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xe};
SetWordType zzerr63[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xe};
SetWordType zzerr64[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0xe};
SetWordType zzerr65[20] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0, 0x0,0x0,0x10,0xc};
SetWordType setwd10[157] = {0x0,0xc,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x3,0x0,
	0x3,0x0,0x0,0xf0,0xf0,0x0};
SetWordType setwd11[157] = {0x0,0x1,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x1,0x0,0x0,0x0,0x0,0x0};
