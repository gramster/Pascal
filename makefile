CC=bcc
CFLAGS=-O -DDEBUG -v -ls
LFLAGS=-v -ls
CSUF=.cpp

all: pm.exe dis.exe

pm.exe: pm.obj parser.obj lex.obj namestore.obj error.obj symtab.obj code.obj pcode.obj debugger.obj pmachine.obj
	$(CC) $(LFLAGS) pm.obj parser.obj lex.obj namestore.obj error.obj symtab.obj code.obj pcode.obj debugger.obj pmachine.obj

prun.exe: prun.obj pmachine.obj pcode.obj
	$(CC) $(LFLAGS) prun.obj pmachine.obj pcode.obj

dis.exe: dis.obj pcode.obj
	$(CC) $(LFLAGS) dis.obj pcode.obj

pm.obj: pm$(CSUF) lex$(CSUF) error.h namestore.h parser.h symtab.h code.h pmachine.h debugger.h
	$(CC) -c $(CFLAGS) pm$(CSUF)

parser.obj: parser$(CSUF) parser.h lex.h error.h namestore.h debugger.h code.h
	$(CC) -c $(CFLAGS) parser$(CSUF)

lex.obj: lex$(CSUF) lex.h error.h namestore.h code.h
	$(CC) -c $(CFLAGS) lex$(CSUF)

namestore.obj: namestore$(CSUF) namestore.h error.h
	$(CC) -c $(CFLAGS) namestore$(CSUF)

error.obj: error$(CSUF) error.h lex.h
	$(CC) -c $(CFLAGS) error$(CSUF)

symtab.obj: symtab$(CSUF) symtab.h error.h namestore.h 
	$(CC) -c $(CFLAGS) symtab$(CSUF)

dis.obj: dis$(CSUF) pcode.h
	$(CC) -c $(CFLAGS) dis$(CSUF)

pcode.obj: pcode$(CSUF) pcode.h
	$(CC) -c $(CFLAGS) pcode$(CSUF)

code.obj: code$(CSUF) code.h symtab.h error.h
	$(CC) -c $(CFLAGS) code$(CSUF)

pmachine.obj: pmachine$(CSUF) pmachine.h debugger.h
	$(CC) -c $(CFLAGS) pmachine$(CSUF)

prun.obj: prun$(CSUF) pmachine.h debugger.h
	$(CC) -c $(CFLAGS) prun$(CSUF)

debugger.obj: debugger$(CSUF) debugger.h
	$(CC) -c $(CFLAGS) debugger$(CSUF)

code.h: pcode.h
	touch code.h

debugger.h: symtab.h
	touch debugger.h

parser.h: pcode.h symtab.h
	touch parser.h
	
pmachine.h: pcode.h
	touch pmachine.h

symtab.h: pcode.h
	touch symtab.h

dos2unix:
	mv pm.cpp pm.cc
	mv parser.cpp parser.cc
	mv lex.cpp lex.cc
	mv namestor.cpp namestore.cc
	mv error.cpp error.cc
	mv symtab.cpp symtab.cc
	mv pcode.cpp pcode.cc
	mv code.cpp code.cc
	mv pmachine.cpp pmachine.cc
	mv prun.cpp prun.cc

unix2dos:
	ren *.cc *.cpp

zip:
	zip -u pm pm$(CSUF) parser$(CSUF) lex$(CSUF) namestor*$(CSUF)
	zip -u pm error$(CSUF) symtab$(CSUF) 
	zip -u pm pcode$(CSUF) code$(CSUF) pmachine$(CSUF) prun$(CSUF)
	zip -u pm debugger$(CSUF) dis$(CSUF)
	zip -u pm code.h debugger.h parser.h pmachine.h pcode.h symtab.h
	zip -u pm lex.h namestor*.h error.h
	zip -u pm Makefile *.pm
