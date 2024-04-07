all: dtre

dtre.tab.c dtre.tab.h: dtre.y
	bison -t -v -d dtre.y

dtre.lex.c: dtre.l dtre.tab.h
	flex -o dtre.lex.c dtre.l

dtre: dtre.lex.c dtre.tab.c dtre.tab.h ./*.hpp
	g++ -o dtre dtre.tab.c dtre.lex.c -lppl -lgmp -lgmpxx

clean:
	rm dtre dtre.tab.c dtre.lex.c dtre.tab.h dtre.output
