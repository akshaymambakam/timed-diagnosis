%option noyywrap

%{
#include <stdio.h>

#define YY_DECL int yylex()

#include "dtre.tab.h"

%}

%%

[ \t]	; // ignore all whitespace
[0-9]+\.[0-9]+ 	{yylval.fstr = strdup(yytext); return T_FLOAT;}
x[0-9]+   	   {
                 yylval.ival = atoi(yytext+1);
                 return X_STRING;
               }

[ \t\r\n]+     { ; }

"^+"    {return K_PLUS;}
"-"		{return T_MINUS;}
"<="    {yylval.ival=1; return LEQ;}
"<"     {yylval.ival=2; return LE;}
">="    {yylval.ival=3; return GEQ;}
">"     {yylval.ival=4; return GE;}
"&"    {return T_AND;}
"|"    {return T_OR;}
"%"    {return T_SEQCOMP;}
"["    {return T_SLEFT;}
"]"    {return T_SRIGHT;}
"("		{return T_LEFT;}
")"		{return T_RIGHT;}
"!"     {return T_NOT;}
"EPS"   {return T_EPS;}
"+@"    {yylval.ival = 1; return T_RISE;}
"-@"     {yylval.ival = 0; return T_FALL;}

%%