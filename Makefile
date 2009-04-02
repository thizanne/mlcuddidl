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
-I$(CUDD_PREFIX)/include \
-I$(CAML_PREFIX)/lib/ocaml -I$(CAMLIDL_PREFIX)/lib/ocaml

#---------------------------------------
# Files
#---------------------------------------

IDLMODULES = man bdd rdd vdd 

MLMODULES = weakke man bdd custom rdd vdd mapleaf mtbdd
MLSRC = $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%.ml) mtbdd.ml mtbdd.mli
MLINT = $(MLMODULES:%=%.cmi)
MLOBJ = $(MLMODULES:%=%.cmo)
MLOBJx = $(MLMODULES:%=%.cmx)
MLLIB_TOINSTALL = $(IDLMODULES:%=%.idl) cudd.cmi cudd.cma
MLLIB_TOINSTALLx = cudd.cmx cudd.cmxa cudd.a

CCMODULES = \
	cuddauxAddCamlTable cuddauxAddIte cuddauxBridge cuddauxCompose \
	cuddauxGenCof cuddauxMisc cuddauxTDGenCof cuddauxAddApply \
	$(IDLMODULES:%=%_caml) custom_caml cudd_caml

CCSRC = cuddaux.h cuddauxInt.h cudd_caml.h $(CCMODULES:%=%.c)

CCBIN_TOINSTALL = cuddtop
CCLIB_TOINSTALL = libcudd_caml.a libcudd_caml_debug.a
CCINC_TOINSTALL = cuddaux.h cuddauxInt.h cudd_caml.h

#---------------------------------------
# Rules
#---------------------------------------

# Global rules
all: cudd.cmi cudd.cmo cudd.cmx cudd.cma cudd.cmxa libcudd_caml.a libcudd_caml_debug.a

cuddtop: cudd.cma libcudd_caml.a
	ocamlmktop -verbose $(OCAMLFLAGS) -o $@ -custom -cc "$(CC) -g" -ccopt -L. cudd.cma -noautolink \
	-ccopt -L. -ccopt -L$(MLCUDDIDL_INSTALL)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_INSTALL)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_INSTALL)/lib -cclib "-lcudd_debug -lmtr -lst -lutil -lepd" \
	-ccopt -L$(CAML_INSTALL)/lib/ocaml -cclib "-lcamlrun_debug"
# Example of compilation command
# If the library is installed somewhere, add a -I $(PATH) option
example.byte: example.ml cudd.cma 
	$(OCAMLC) $(OCAMLFLAGS) -o $@ -custom -cc "$(CC)" \
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

%.byte: %.ml cudd.cma libcudd_caml_debug.a
	$(OCAMLC) $(OCAMLFLAGS) -o $@ -cc "$(CC) -g" -custom -verbose -noautolink \
	cudd.cma $*.ml \
	-ccopt -L. -ccopt -L$(MLCUDDIDL_INSTALL)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_INSTALL)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_INSTALL)/lib -cclib "-lcudd_debug -lmtr -lst -lutil -lepd" \
	-ccopt -L$(CAML_INSTALL)/lib/ocaml -cclib "-lcamlrun_debug"
%.opt: %.ml cudd.cmxa libcudd_caml_debug.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -o $@ -cc "$(CC)" -verbose -noautolink \
	cudd.cmxa $*.ml \
	-ccopt -L. -ccopt -L$(MLCUDDIDL_INSTALL)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_INSTALL)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_INSTALL)/lib -cclib "-lcudd_debug -lmtr -lst -lutil -lepd" \
	-ccopt -L$(CAML_INSTALL)/lib/ocaml -cclib "-lasmrun_debug"

install: cudd.ml cudd.mli
	mkdir -p $(INCDIR) $(LIBDIR) $(BINDIR)
	cp -f $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) cudd.ml cudd.mli $(LIBDIR)
	rm cudd.ml cudd.mli
	cp -f $(CCINC_TOINSTALL) $(INCDIR)
	for i in $(CCLIB_TOINSTALL); do if test -f $$i; then cp -f $$i $(LIBDIR); fi; done
	for i in $(CCBIN_TOINSTALL); do if test -f $$i; then cp -f $$i $(BINDIR); fi; done

distclean: mostlyclean
	(cd $(INCDIR); /bin/rm -f $(CCINC_TOINSTALL))
	(cd $(LIBDIR); /bin/rm -f $(CCLIB_TOINSTALL) $(LIB_TOINSTALLx) $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) cudd.ml cudd.mli)
	(cd $(BINDIR); /bin/rm -f $(CCBIN_TOINSTALL))

mostlyclean: clean
	/bin/rm -f $(IDLMODULES:%=%.ml) $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%_caml.c) tmp/* html/*
	/bin/rm -f mlcuddidl.?? mlcuddidl.??? mlcuddidl.info example example.opt *.tex *.dvi style.css ocamldoc.sty

clean:
	/bin/rm -f cuddtop *.byte *.opt
	/bin/rm -f cuddaux.?? cuddaux.??? cuddaux.info
	/bin/rm -f cudd.ml cudd.mli *.[ao] *.cm[ioxa] *.cmxa *.opt *.opt2 *.annot
	/bin/rm -f cmttb*
	/bin/rm -fr html

tar: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes example.ml session.ml mlcuddidl.texi texinfo.tex sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

dist: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes example.ml session.ml mlcuddidl.texi texinfo.tex mlcuddidl.pdf mlcuddidl.info html sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

# CAML rules
cudd.cma: cudd.cmo
	$(OCAMLC) $(OCAMLFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDDAUX_PREFIX)/lib -cclib -lcuddaux \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
cudd.cmxa: cudd.cmx
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDDAUX_PREFIX)/lib -cclib -lcuddaux \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
	$(RANLIB) cudd.a

cudd.cmo cudd.cmi: $(MLOBJ)
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -pack -o $@ $(MLOBJ)

cudd.cmx cudd.o: $(MLOBJx)
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -pack -o $@ $(MLOBJx)


libcudd_caml.a: $(CCMODULES:%=%.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

libcudd_caml_debug.a: $(CCMODULES:%=%_debug.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

# HTML and LATEX rules
.PHONY: html
# mlcuddidl.pdf: mlcuddidl.texi
#	$(TEXI2DVI) $<
# mlcuddidl.info: mlcuddidl.texi
#	makeinfo --no-split $<
# html: mlcuddidl.texi
#	$(TEXI2HTML) -split=chapter -nosec_nav -subdir=html $<

mlcuddidl.pdf: mlcuddidl.dvi
	$(DVIPDF) mlcuddidl.dvi
mlcuddidl.dvi: mlcuddidl.odoc $(MLMODULES:%=%.mli)
	$(OCAMLDOC) $(OCAMLINC) \
-t "MLCUDDIDL: OCaml interface for CUDD library, version 1.3" \
-latextitle 1,chapter -latextitle 2,section -latextitle 3,subsection -latextitle 4,subsubsection -latextitle 5,paragraph -latextitle 6,subparagraph \
-latex -intro mlcuddidl.odoc -o ocamldoc.tex $(MLMODULES:%=%.mli)
	$(SED) -e 's/\\documentclass\[11pt\]{article}/\\documentclass[10pt,twosdie,a4paper]{book}\\usepackage{ae,fullpage,makeidx,fancyhdr}\\usepackage[ps2pdf]{hyperref}\\pagestyle{fancy}\\setlength{\\parindent}{0em}\\setlength{\\parskip}{0.5ex}\\sloppy\\makeindex\\author{Bertrand Jeannet}/' -e 's/\\end{document}/\\appendix\\printindex\\end{document}/' ocamldoc.tex >mlcuddidl.tex
	$(LATEX) mlcuddidl
	$(MAKEINDEX) mlcuddidl
	$(LATEX) mlcuddidl
	$(LATEX) mlcuddidl

html: mlcuddidl.odoc $(MLMODULES:%=%.mli)
	mkdir -p html
	$(OCAMLDOC) $(OCAMLINC) -html -d html -colorize-code -intro mlcuddidl.odoc $(MLMODULES:%=%.mli)

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

%.o: %.c
	$(CC) $(CFLAGS) $(ICFLAGS) $(XCFLAGS) -c -o $@ $<
%_debug.o: %.c
	$(CC) $(CFLAGS_DEBUG) $(ICFLAGS) $(XCFLAGS) -c -o $@ $<

#-----------------------------------
# CAML
#-----------------------------------

%.cmi: %.mli
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmo: %.ml %.cmi
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -c $<

%.cmx: %.ml %.cmi
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -for-pack Cudd -c $<

cudd.ml: $(MLMODULES:%=%.ml)
	echo "" >$@
	for i in $(MLMODULES); do \
		echo "" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "" >>$@; \
		echo "$$i" | sed -e "s/\(.*\)/module \u\1 = struct/g" >>$@; \
		cat <$$i.ml | sed -e "s/^/  /g" >>$@; \
		echo "end" >>$@; \
	done

cudd.mli: $(MLMODULES:%=%.mli)
	echo "" >$@
	for i in $(MLMODULES); do \
		echo "" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "(*  ********************************************************************** *)" >>$@; \
		echo "" >>$@; \
		echo "$$i" | sed -e "s/\(.*\)/module \u\1 : sig/g" >>$@; \
		cat <$$i.mli | sed -e "s/^/  /g" >>$@; \
		echo "end" >>$@; \
	done

#-----------------------------------
# Dependencies
#-----------------------------------

cuddaux%.o: cuddaux.h cuddauxInt.h

%_.caml.o: cudd_caml.h cuddaux.h cuddauxInt.h

bdd.cmo: man.cmi bdd.cmi
bdd.cmx: man.cmx bdd.cmi
custom.cmo: man.cmi bdd.cmi custom.cmi
custom.cmx: man.cmx bdd.cmx custom.cmi
essai.cmo: rdd.cmi manager.cmi bdd.cmi
essai.cmx: rdd.cmx manager.cmx bdd.cmx
example.cmo: manager.cmi bdd.cmi
example.cmx: manager.cmx bdd.cmx
manager.cmo: manager.cmi
manager.cmx: manager.cmi
man.cmo: man.cmi
man.cmx: man.cmi
mapleaf.cmo: vdd.cmi man.cmi bdd.cmi mapleaf.cmi
mapleaf.cmx: vdd.cmx man.cmx bdd.cmx mapleaf.cmi
mtbdd.cmo: weakke.cmi vdd.cmi mapleaf.cmi custom.cmi mtbdd.cmi
mtbdd.cmx: weakke.cmx vdd.cmx mapleaf.cmx custom.cmx mtbdd.cmi
rdd.cmo: man.cmi custom.cmi bdd.cmi rdd.cmi
rdd.cmx: man.cmx custom.cmx bdd.cmx rdd.cmi
session.cmo: vdd.cmi rdd.cmi manager.cmi bdd.cmi
session.cmx: vdd.cmx rdd.cmx manager.cmx bdd.cmx
t.cmo: weakke.cmi
t.cmx: weakke.cmx
vdd.cmo: rdd.cmi man.cmi custom.cmi bdd.cmi vdd.cmi
vdd.cmx: rdd.cmx man.cmx custom.cmx bdd.cmx vdd.cmi
weakke.cmo: weakke.cmi
weakke.cmx: weakke.cmi
bdd.cmi: man.cmi
custom.cmi: man.cmi bdd.cmi
manager.cmi:
man.cmi:
mapleaf.cmi: vdd.cmi man.cmi bdd.cmi
mtbdd.cmi: weakke.cmi vdd.cmi man.cmi custom.cmi bdd.cmi
rdd.cmi: man.cmi custom.cmi bdd.cmi
vdd.cmi: man.cmi custom.cmi bdd.cmi
weakke.cmi:
