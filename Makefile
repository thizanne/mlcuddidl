#---------------------------------------
# Directories
#---------------------------------------

SRCDIR = $(shell pwd)
#
# Installation directory prefix
#
PREFIX = $(MLCUDDIDL_INSTALL)
# C include and lib directories
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin
#
# Where to find necessary files
#
# Caml and Camlidl lib directory
CAMLLIBDIR = $(CAML_INSTALL)/lib/ocaml
CAMLIDLLIBDIR = $(CAMLIDL_INSTALL)/lib/ocaml
# CUDD include and lib directories
CUDDINCDIR = $(CUDD_INSTALL)/include
CUDDLIBDIR = $(CUDD_INSTALL)/lib
# CUDDAUX include and lib directories
CUDDAUXLIBDIR = $(CUDDAUX_INSTALL)/include
CUDDAUXINCDIR = $(CUDDAUX_INSTALL)/include

#---------------------------------------
# CAML part
#---------------------------------------
OCAMLC = ocamlc.opt 
OCAMLOPT = ocamlopt.opt
OCAMLDEP = ocamldep
OCAMLLEX = ocamllex.opt
OCAMLYACC = ocamlyacc
OCAMLINC =
OCAMLFLAGS = -g
OCAMLOPTFLAGS = -inline 20

CAMLIDL = camlidl
M4 = m4


#---------------------------------------
# C part
#---------------------------------------

CC = gcc
ICFLAGS = \
-I$(CUDDINCDIR) -I$(CUDDAUXINCDIR) \
-I$(CAMLLIBDIR) -I$(CAMLIDLLIBDIR) \
-Winline -Wimplicit-function-declaration 
#
# XCFLAGS should be the same than the flags with which CUDD has been compiled
#
XCFLAGS = -mcpu=pentium4 -malign-double -DHAVE_IEEE_754 -DBSD
#XCFLAGS = -mcpu=ultrasparc -DHAVE_IEEE_754 -DUNIX100

CFLAGS = $(ICFLAGS) $(XCFLAGS) -O3 -DNDEBUG
CFLAGS_DEBUG = $(ICFLAGS) $(XCFLAGS) -O0 -g -UNDEBUG
CFLAGS_PROF = $(CFLAGS) -g -pg

#---------------------------------------
# Files
#---------------------------------------

IDLMODULES = manager bdd rdd idd

MLMODULES = $(IDLMODULES) mtbdd
MLSRC = $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%.ml) mtbdd.ml mtbdd.mli
MLINT = $(MLMODULES:%=%.cmi)
MLOBJ = $(MLMODULES:%=%.cmo)
MLOBJx = $(MLMODULES:%=%.cmx)
MLLIB_TOINSTALL = $(MLMODULES:%=%.mli) $(MLMODULES:%=%.cmi) cudd.cma 
MLLIB_TOINSTALLx = $(MLMODULES:%=%.mli) $(MLMODULES:%=%.cmx) cudd.cmxa cudd.a

CCMODULES = cudd_caml $(IDLMODULES:%=%_caml)
CCSRC = cudd_caml.h $(CCMODULES:%=%.c)

CCBIN_TOINSTALL = cuddrun
CCLIB_TOINSTALL = libcudd_caml.a libcudd_caml_debug.a libcudd_caml_prof.a
CCINC_TOINSTALL = cudd_caml.h

#---------------------------------------
# Rules
#---------------------------------------

# Global rules
all: common libcudd_caml.a
debug: common libcudd_caml_debug.a 
prof: common libcudd_caml_prof.a
common: $(MLINT) $(MLOBJ) $(MLOBJx) cudd.cma cudd.cmxa

cuddrun: cudd.cma libcudd_caml.a 
	$(OCAMLC) $(OCAMLFLAGS) -o $@ -make_runtime -cc "$(CC)" cudd.cma \
	-cclib "-L. -lcudd_caml" \
	-cclib "-L$(CAMLIDLLIBDIR) -lcamlidl" \
	-cclib "-L$(CUDDAUXLIBDIR) -lcuddaux" \
	-cclib "-L$(CUDDLIBDIR) -lcudd -lmtr -lst -lutil -lepd"
cuddtop: cudd.cma libcudd_caml_debug.a 
	ocamlmktop $(OCAMLFLAGS) -o $@ -custom -cc "$(CC)" cudd.cma \
	-cclib "-L. -lcudd_caml_debug" \
	-cclib "-L$(CAMLIDLLIBDIR) -lcamlidl" \
	-cclib "-L$(CUDDAUXLIBDIR) -lcuddaux_debug" \
	-cclib "-L$(CUDDLIBDIR) -lcudd -lmtr -lst -lutil -lepd"


install:
	mkdir -p $(INCDIR) $(LIBDIR) $(BINDIR)
	cp -f $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) $(LIBDIR)
	cp -f $(CCINC_TOINSTALL) $(INCDIR)
	for i in $(CCLIB_TOINSTALL); do if test -f $$i; then cp -f $$i $(LIBDIR); fi; done
	for i in $(CCBIN_TOINSTALL); do if test -f $$i; then cp -f $$i $(BINDIR); fi; done

distclean: mostlyclean
	(cd $(INCDIR); /bin/rm -f $(CCINC_TOINSTALL))
	(cd $(LIBDIR); /bin/rm -f $(LIB_TOINSTALL) $(LIB_TOINSTALLx))
	(cd $(BINDIR); /bin/rm -f $(CCBIN_TOINSTALL))

mostlyclean: clean
	/bin/rm -f $(IDLMODULES:%=%.ml) $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%_caml.c) tmp/* html/*
	/bin/rm -f mlcuddidl.?? mlcuddidl.??? mlcuddidl.info sedscript

clean:
	/bin/rm -f cuddrun cuddtop 
	/bin/rm -f *.[ao] *.cm[ioxa] *.cmxa

tar: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile README Changes session.ml mlcuddidl.texi texinfo.tex
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

dist: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile README Changes session.ml mlcuddidl.texi texinfo.tex mlcuddidl.dvi mlcuddidl.info html
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

#
# Compilation Example
#
# bytecode
dummy.cmo: dummy.ml
	$(OCAMLC) -I $(LIBDIR) -o dummy.cmo dummy.ml
dummy: dummy.cmo
	$(OCAMLC) -I $(LIBDIR) -use-runtime cuddrun -o $@ $< cudd.cma
# native code
dummy.cmx: dummy.ml
	$(OCAMLOPT) -I $(LIBDIR) -o $@ dummy.ml
dummy.opt: dummy.cmx
	$(OCAMLOPT) -I $(LIBDIR) -o $@ dummy.ml cudd.cmxa \
	-cclib "-L. -lcudd_caml" \
	-cclib "-L$(CAMLIDLLIBDIR) -lcamlidl" \
	-cclib "-L$(CUDDAUXLIBDIR) -lcuddaux" \
	-cclib "-L$(CUDDLIBDIR) -lcudd -lmtr -lst -lutil -lepd"

# CAML rules
cudd.cma: $(MLOBJ)
	$(OCAMLC) $(OCAMLFLAGS) -a -o $@ $^
cudd.cmxa: $(MLOBJx)
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -a -o $@ $^
libcudd_caml.a: $(CCMODULES:%=%.o)
	ar rcs $@ $^
libcudd_caml_debug.a: $(CCMODULES:%=%_debug.o)
	ar rcs $@ $^
libcudd_caml_prof.a: $(CCMODULES:%=%_prof.o)
	ar rcs $@ $^

# TEX rules
mlcuddidl.dvi: mlcuddidl.texi
	texi2dvi $<
mlcuddidl.ps: mlcuddidl.dvi
	dvips -o $@ $<
mlcuddidl.info: mlcuddidl.texi
	makeinfo --no-split $<
mlcuddidl.html: mlcuddidl.texi
	texi2html -split=chapter -nosec_nav -subdir=html $<

#--------------------------------------------------------------
# IMPLICIT RULES AND DEPENDENCIES
#--------------------------------------------------------------

.SUFFIXES: .tex .fig .c .h .o .ml .mli .cmi .cmo .cmx .idl _debug.o _prof.o _caml.c

#-----------------------------------
# IDL
#-----------------------------------

# sed -f sedscript allows to remove prefixes generated by camlidl
# grep --extended-regexp '^(.)+$$' removes blanks lines

%_caml.c %.ml %.mli: %.idl macros.m4 sedscript
	cp $*.idl tmp/$*.idl
	$(CAMLIDL) -no-include -prepro "$(M4) macros.m4" -I $(SRCDIR) tmp/$*.idl
	cp tmp/$*_stubs.c $*_caml.c
	sed -f sedscript tmp/$*.ml | grep --extended-regexp '^(.)+$$' >$*.ml
	sed -f sedscript tmp/$*.mli | grep --extended-regexp '^(.)+$$' >$*.mli

sedscript: Makefile
	echo "\
s/manager__t/t/g; \
s/bdd__t/t/g; \
s/rdd__t/t/g; \
s/idd__t/t/g; \
s/external manager_/external /g; \
s/external bdd_/external /g; \
s/external rdd_/external /g; \
s/external idd_/external /g; \
s/CUDD_//g; \
	" >$@

bdd_caml.c bdd.ml bdd.mli: manager.idl
rdd_caml.c rdd.ml rdd.mli: manager.idl bdd.idl

#-----------------------------------
# C
#-----------------------------------

%.o: %.c cudd_caml.h 
	$(CC) $(CFLAGS) -c -o $@ $<
%_debug.o: %.c cudd_caml.h 
	$(CC) $(CFLAGS_DEBUG) -c -o $@ $<
%_prof.o: %.c cudd_caml.h 
	$(CC) $(CFLAGS_PROF) -c -o $@ $<

#-----------------------------------
# CAML
#-----------------------------------

%.cmi: %.mli
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmo: %.ml %.cmi
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmx: %.ml %.cmi
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -c $<

# specifiques
depend: $(MLMODULES:%=%.mli) $(MLMODULES:%=%.ml)
	$(OCAMLDEP) $(OCAMLINC) $^ >Makefile.depend

Makefile.depend:
	touch $@

include Makefile.depend

