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

IDLMODULES = man bdd add vdd

MLMODULES = man bdd custom add weakke pWeakke vdd mtbdd mtbddc mapleaf user 
MLSRC = $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%.ml) custom.ml custom.mli mapleaf.ml mapleaf.mli weakke.ml weakke.mli pWeakke.ml pWeakke.mli mtbdd.ml mtbdd.mli mtbddc.ml mtbddc.mli
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
	$(OCAMLMKTOP) -verbose $(OCAMLFLAGS) -o $@ -custom -cc "$(CC) -g" -ccopt -L. cudd.cma -noautolink \
	-ccopt -L. -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd" \
	-cclib "-lcamlrun"
# Example of compilation command
# If the library is installed somewhere, add a -I $(PATH) option
example.byte: example.ml cudd.cma
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -o $@ -custom -cc "$(CC)" \
	cudd.cma example.ml
example.opt: example.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" \
	cudd.cmxa example.ml

# Example of compilation command if the autolink feature is disabled
# This may be useful for selecting debug version of libraries
example.opt2: example.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" -noautolink \
	cudd.cmxa example.ml \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
	-cclib "-lasmrun"

%.byte: %.ml cudd.cma libcudd_caml_debug.a
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -o $@ -cc "$(CC) -g" -custom -verbose -noautolink \
	cudd.cma $*.ml \
	-ccopt -L. -ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd_debug -lmtr -lst -lutil -lepd" \
	-cclib "-lcamlrun"
%.opt: %.ml cudd.cmxa libcudd_caml_debug.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" -verbose -noautolink \
	cudd.cmxa $*.ml \
	-ccopt -L. -ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd_debug -lmtr -lst -lutil -lepd" \
	-cclib "-lasmrun"

install: 
	mkdir -p $(INCDIR) $(LIBDIR) $(BINDIR)
	cp -f $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) $(LIBDIR)
	cp -f $(CCINC_TOINSTALL) $(INCDIR)
	for i in $(CCLIB_TOINSTALL); do if test -f $$i; then cp -f $$i $(LIBDIR); fi; done
	for i in $(CCBIN_TOINSTALL); do if test -f $$i; then cp -f $$i $(BINDIR); fi; done

distclean:
	(cd $(INCDIR); /bin/rm -f $(CCINC_TOINSTALL))
	(cd $(LIBDIR); /bin/rm -f $(CCLIB_TOINSTALL) $(LIB_TOINSTALLx) $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) )
	(cd $(BINDIR); /bin/rm -f $(CCBIN_TOINSTALL))

mostlyclean: clean
	/bin/rm -f $(IDLMODULES:%=%.ml) $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%_caml.c) tmp/* html/*
	/bin/rm -f mlcuddidl.?? mlcuddidl.??? mlcuddidl.info example example.opt *.tex *.dvi style.css ocamldoc.sty

clean:
	/bin/rm -f cuddtop *.byte *.opt
	/bin/rm -f cuddaux.?? cuddaux.??? cuddaux.info
	/bin/rm -f *.[ao] *.cm[ioxa] *.cmxa *.opt *.opt2 *.annot
	/bin/rm -f cmttb*
	/bin/rm -fr html

tar: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes example.ml session.ml mlcuddidl.texi texinfo.tex sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

dist: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model README Changes test_mtbdd.ml session.ml mlcuddidl.odoc mlcuddidl.pdf html sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))

# CAML rules
cudd.cma: cudd.cmo
	$(OCAMLC) $(OCAMLFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lutil -lepd"
cudd.cmxa: cudd.cmx
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -a -o $@ $^ \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
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
-t "MLCUDDIDL: OCaml interface for CUDD library, version 2.1" \
-latextitle 1,chapter -latextitle 2,section -latextitle 3,subsection -latextitle 4,subsubsection -latextitle 5,paragraph -latextitle 6,subparagraph \
-latex -intro mlcuddidl.odoc -o ocamldoc.tex man.mli bdd.mli add.mli vdd.mli mtbdd.mli mtbddc.mli mapleaf.mli user.mli custom.mli weakke.mli pWeakke.mli
	$(SED) -e 's/\\documentclass\[11pt\]{article}/\\documentclass[10pt,twosdie,a4paper]{book}\\usepackage{ae,fullpage,makeidx,fancyhdr}\\usepackage[ps2pdf]{hyperref}\\pagestyle{fancy}\\setlength{\\parindent}{0em}\\setlength{\\parskip}{0.5ex}\\sloppy\\makeindex\\author{Bertrand Jeannet}/' -e 's/\\end{document}/\\appendix\\printindex\\end{document}/' ocamldoc.tex >mlcuddidl.tex
	$(LATEX) mlcuddidl
	$(MAKEINDEX) mlcuddidl
	$(LATEX) mlcuddidl
	$(LATEX) mlcuddidl

html: mlcuddidl.odoc $(MLMODULES:%=%.mli)
	mkdir -p html
	$(OCAMLDOC) $(OCAMLINC) -html -d html -colorize-code -intro mlcuddidl.odoc man.mli bdd.mli add.mli mtbdd.mli custom.mli mapleaf.mli weakke.mli pWeakke.mli vdd.mli

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

#cudd.ml: $(MLMODULES:%=%.ml)
#	echo "" >$@
#	for i in $(MLMODULES); do \
#		echo "" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "" >>$@; \
#		echo "$$i" | sed -e "s/\(.*\)/module \u\1 = struct/g" >>$@; \
#		cat <$$i.ml | sed -e "s/^/  /g" >>$@; \
#		echo "end" >>$@; \
#	done
#
#cudd.mli: $(MLMODULES:%=%.mli)
#	echo "" >$@
#	for i in $(MLMODULES); do \
#		echo "" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "(*  ********************************************************************** *)" >>$@; \
#		echo "" >>$@; \
#		echo "$$i" | sed -e "s/\(.*\)/module \u\1 : sig/g" >>$@; \
#		cat <$$i.mli | sed -e "s/^/  /g" >>$@; \
#		echo "end" >>$@; \
#	done

#-----------------------------------
# Dependencies
#-----------------------------------

cuddaux%.o: cuddaux.h cuddauxInt.h

%_.caml.o: cudd_caml.h cuddaux.h cuddauxInt.h

add.cmo: man.cmi custom.cmi bdd.cmi add.cmi
add.cmx: man.cmx custom.cmx bdd.cmx add.cmi
bdd.cmo: man.cmi bdd.cmi
bdd.cmx: man.cmx bdd.cmi
custom.cmo: bdd.cmi custom.cmi
custom.cmx: bdd.cmx custom.cmi
example.cmo: man.cmi bdd.cmi add.cmi
example.cmx: man.cmx bdd.cmx add.cmx
man.cmo: man.cmi
man.cmx: man.cmi
mapleaf.cmo: vdd.cmi bdd.cmi mapleaf.cmi
mapleaf.cmx: vdd.cmx bdd.cmx mapleaf.cmi
mtbddc.cmo: vdd.cmi pWeakke.cmi man.cmi bdd.cmi mtbddc.cmi
mtbddc.cmx: vdd.cmx pWeakke.cmx man.cmx bdd.cmx mtbddc.cmi
mtbdd.cmo: vdd.cmi pWeakke.cmi mtbdd.cmi
mtbdd.cmx: vdd.cmx pWeakke.cmx mtbdd.cmi
pWeakke.cmo: weakke.cmi pWeakke.cmi
pWeakke.cmx: weakke.cmx pWeakke.cmi
session.cmo: vdd.cmi man.cmi bdd.cmi add.cmi
session.cmx: vdd.cmx man.cmx bdd.cmx add.cmx
test_bdd.cmo: man.cmi bdd.cmi
test_bdd.cmx: man.cmx bdd.cmx
test_mtbdd.cmo: pWeakke.cmi mtbddc.cmi man.cmi bdd.cmi add.cmi
test_mtbdd.cmx: pWeakke.cmx mtbddc.cmx man.cmx bdd.cmx add.cmx
user.cmo: vdd.cmi man.cmi custom.cmi bdd.cmi user.cmi
user.cmx: vdd.cmx man.cmx custom.cmx bdd.cmx user.cmi
vdd.cmo: man.cmi bdd.cmi add.cmi vdd.cmi
vdd.cmx: man.cmx bdd.cmx add.cmx vdd.cmi
weakke.cmo: weakke.cmi
weakke.cmx: weakke.cmi
add.cmi: man.cmi custom.cmi bdd.cmi
bdd.cmi: man.cmi
custom.cmi: bdd.cmi
man.cmi:
mapleaf.cmi: vdd.cmi bdd.cmi
mtbddc.cmi: vdd.cmi pWeakke.cmi man.cmi bdd.cmi
mtbdd.cmi: vdd.cmi pWeakke.cmi man.cmi bdd.cmi
pWeakke.cmi: weakke.cmi
user.cmi: vdd.cmi bdd.cmi
vdd.cmi: man.cmi bdd.cmi
weakke.cmi:
