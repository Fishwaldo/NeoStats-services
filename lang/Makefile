# Makefile for language module

#include ../../Makefile


LANGOBJS = en_us 
LANGSRCS = en_us.l

#LANGCOMP = ./langcomp
LANGCOMP = ./langcomp -w


all: $(LANGOBJS)


clean:
	rm -f $(LANGOBJS) langcomp language.h

spotless: clean
	rm -f language.h



en_us:    en_us.l    langcomp index
	./langcomp $@.l

langcomp: langcomp.c
	$(CC) $(CFLAGS) langcomp.c -o $@


language.h: index Makefile
	@perl -e <index >$@ '\
		print STDERR "Generating language.h... "; \
		$$i=0; \
		while (<>) { \
			chop; \
			printf "#define %-32s %d\n", $$_, $$i++; \
		} \
		print "\n#define NUM_STRINGS $$i\n"; \
		print STDERR "$$i strings\n";'

index: en_us.l
	grep '^[A-Z]' en_us.l >index
