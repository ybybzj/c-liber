#ifndef __CONSOLE_COLOR_H__
#define __CONSOLE_COLOR_H__
/*
*	Console output colorizing tools 
*/
//foreground color
#define cls_Black 30
#define cls_Red 31
#define cls_Green 32
#define cls_Yellow 33
#define cls_Blue 34
#define cls_Magenta 35
#define cls_Cyan 36
#define cls_White 37

#define cls_hBlack 90
#define cls_hRed 91
#define cls_hGreen 92
#define cls_hYellow 93
#define cls_hBlue 94
#define cls_hMagenta 95
#define cls_hCyan 96
#define cls_hWhite 97

//background color
#define cls_bBlack 40
#define cls_bRed 41
#define cls_bGreen 42
#define cls_bYellow 43
#define cls_bBlue 44
#define cls_bMagenta 45
#define cls_bCyan 46
#define cls_bWhite 47

#define cls_bhBlack 100
#define cls_bhRed 101
#define cls_bhGreen 102
#define cls_bhYellow 103
#define cls_bhBlue 104
#define cls_bhMagenta 105
#define cls_bhCyan 106
#define cls_bhWhite 107

//font
#define cls_Bold 1       //bold and high intensity
#define cls_Underline 4
           
#define cls_Crossout 9
#define cls_Conceal 8

// #define cls_FontPrimary 10
// #define cls_Font1 11
// #define cls_Font2 12
// #define cls_Font3 13
// #define cls_Font4 14
// #define cls_Font5 15
// #define cls_Font6 16
// #define cls_Font7 17
// #define cls_Font8 18
// #define cls_Font9 19




#define ANSI_STYLE(s)   "\x1b[" #s "m"


#define ANSI_COLOR_RESET   "\x1b[m"


#define cls_s(str,style) ANSI_STYLE(style) str ANSI_COLOR_RESET //customized style
#define cls_c(str,c) cls_s(str,c)                                //colorize
#define cls_b(str) cls_s(str,1)								 	 //bold
#define cls_u(str) cls_s(str,4) 								 //underline
#define cls_ub(str) cls_s(str,1;4)							 	 //bold and underline

#endif