include Makefile.config

#---------------------------------------
# Directories
#---------------------------------------

SRCDIR = $(shell pwd)
#
# Installation directory prefix
#
PREFIX = $(MLCUDDIDL_PREFIX)
# C include and lib directories
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin

#---------------------------------------
# C part
#---------------------------------------

ICFLAGS = \
-I$(CUDD_PREFIX)/include -I$(CUDDAUX_PREFIX)/include \
-I$(CAML_PREFIX)/lib/ocaml -I$(CAMLIDL_PREFIX)/lib/ocaml

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
MLLIB_TOINSTALLx = $(MLMODULES:%=%.cmx) cudd.cmxa cudd.a

CCMODULES = cudd_caml $(IDLMODULES:%=%_caml)
CCSRC = cudd_caml.h $(CCMODULES:%=%.c)

CCBIN_TOINSTALL = cuddrun cuddtop
CCLIB_TOINSTALL = libcudd_caml.a libcudd_caml_debug.a
CCINC_TOINSTALL = cudd_caml.h

#---------------------------------------
# Rules
#---------------------------------------

# Global rules
all: $(MLINT) $(MLOBJ) $(MLOBJx) cudd.cma cudd.cmxa libcudd_caml.a libcudd_caml_debug.a

cuddrun: cudd.cma libcudd_caml.a
	$(OCAMLC) $(OCAMLFLAGS) -o $@ -make-runtime -cc "$(CC)" -ccopt -L. cudd.cma
cuddtop: cudd.cma libcudd_caml.a
	ocamlmktop $(OCAMLFLAGS) -o $@ -custom -cc "$(CC)" -ccopt -L. cudd.cma

# Example of compilation command
# If the library is installed somewhere, add a -I $(PATH) option
example: example.ml cuddrun
	$(OCAMLC) $(OCAMLFLAGS) -o $@ -use-runtime cuddrun -cc "$(CC)" \
	cudd.cma example.ml
example.opt: example.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -o $@ -cc "$(CC)" \
	cudd.cmxa example.ml

# Example of compilation command if the autolink feature is disabled
# This may be useful for selecting debug version of libraries
example.opt2: example.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -o $@ -cc "$(CC)" -noautolink \
	cudd.cmxa example.ml \
	-ccopt -L$(MLCUDDIDL_INSTALL)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_INSTALL)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDDAUX_INSTALL)/lib -cclib -lcuddaux \
	-ccopt -L$(CUDD_INSTALL)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"

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
	/bin/rm -f mlcuddidl.?? mlcuddidl.??? mlcuddidl.info example example.opt

clean:
	/bin/rm -f cuddrun cuddtop
	/bin/rm -f *.[ao] *.cm[ioxa] *.cmxa
	/bin/rm -f cmttb*
	/bin/rm -fr html

tar: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes session.ml mlcuddidl.texi texinfo.tex sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

dist: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes session.ml mlcuddidl.texi texinfo.tex mlcuddidl.pdf mlcuddidl.info html sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

# CAML rules
cudd.cma: $(MLOBJ)
	$(OCAMLC) $(OCAMLFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDDAUX_PREFIX)/lib -cclib -lcuddaux \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
cudd.cmxa: $(MLOBJx)
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDDAUX_PREFIX)/lib -cclib -lcuddaux \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
	$(RANLIB) cudd.a

libcudd_caml.a: $(CCMODULES:%=%.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

libcudd_caml_debug.a: $(CCMODULES:%=%_debug.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

# TEX rules
.PHONY: html
mlcuddidl.pdf: mlcuddidl.texi
	$(TEXI2DVI) $<
mlcuddidl.info: mlcuddidl.texi
	makeinfo --no-split $<
html: mlcuddidl.texi
	$(TEXI2HTML) -split=chapter -nosec_nav -subdir=html $<

#--------------------------------------------------------------
# IMPLICIT RULES AND DEPENDENCIES
#--------------------------------------------------------------

.SUFFIXES: .c .h .o .ml .mli .cmi .cmo .cmx .idl _debug.o _caml.c

#-----------------------------------
# IDL
#-----------------------------------

# Generates X_caml.c, X.ml, X.mli from X.idl

# sed -f sedscript_caml allows to remove prefixes generated by camlidl
# grep --extended-regexp '^(.)+$$' removes blanks lines

rebuild: macros.m4 sedscript_caml sedscript_c
	mkdir -p tmp
	for i in $(IDLMODULES); do \
		echo "module $$i"; \
		cp $${i}.idl tmp/$${i}.idl; \
		$(CAMLIDL) -no-include -prepro "$(M4) macros.m4" -I $(SRCDIR) tmp/$${i}.idl; \
		$(SED) -f sedscript_c tmp/$${i}_stubs.c >$${i}_caml.c; \
		$(SED) -f sedscript_caml tmp/$${i}.ml >$${i}.ml; \
		$(SED) -f sedscript_caml tmp/$${i}.mli >$${i}.mli; \
	done
	rm -fr tmp

#-----------------------------------
# C
#-----------------------------------

%.o: %.c cudd_caml.h
	$(CC) $(CFLAGS) $(ICFLAGS) $(XCFLAGS) -c -o $@ $<
%_debug.o: %.c cudd_caml.h
	$(CC) $(CFLAGS_DEBUG) $(ICFLAGS) $(XCFLAGS) -c -o $@ $<

#-----------------------------------
# CAML
#-----------------------------------

%.cmi: %.mli
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmo: %.ml %.cmi
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmx: %.ml %.cmi
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -c $<

#-----------------------------------
# Dependencies
#-----------------------------------

%.c: cudd_caml.h

bdd.cmo: manager.cmi bdd.cmi
bdd.cmx: manager.cmx bdd.cmi
example.cmo: manager.cmi bdd.cmi
example.cmx: manager.cmx bdd.cmx
idd.cmo: rdd.cmi manager.cmi bdd.cmi idd.cmi
idd.cmx: rdd.cmx manager.cmx bdd.cmx idd.cmi
manager.cmo: manager.cmi
manager.cmx: manager.cmi
mtbdd.cmo: manager.cmi idd.cmi bdd.cmi mtbdd.cmi
mtbdd.cmx: manager.cmx idd.cmx bdd.cmx mtbdd.cmi
rdd.cmo: manager.cmi bdd.cmi rdd.cmi
rdd.cmx: manager.cmx bdd.cmx rdd.cmi
session.cmo: manager.cmi idd.cmi bdd.cmi
session.cmx: manager.cmx idd.cmx bdd.cmx
bdd.cmi: manager.cmi
idd.cmi: rdd.cmi manager.cmi bdd.cmi
mtbdd.cmi: manager.cmi idd.cmi bdd.cmi
rdd.cmi: manager.cmi bdd.cmi
