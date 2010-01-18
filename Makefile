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
MLSRC = $(MLMODULES:%=%.mli) $(MLMODULES:%=%.ml)
MLOBJ = $(MLMODULES:%=%.cmo)
MLOBJx = $(MLMODULES:%=%.cmx)
MLLIB_TOINSTALL = $(IDLMODULES:%=%.idl) cudd.cmi cudd.cma
MLLIB_TOINSTALLx = cudd.cmx cudd.cmxa cudd.a

CCMODULES = \
	cuddauxAddCamlTable cuddauxAddIte cuddauxBridge cuddauxCompose \
	cuddauxGenCof cuddauxMisc cuddauxTDGenCof cuddauxAddApply \
	$(IDLMODULES:%=%_caml) custom_caml cudd_caml

CCSRC = cuddaux.h cuddauxInt.h cudd_caml.h $(CCMODULES:%=%.c)

CCBIN_TOINSTALL = cuddtop cuddrun
CCLIB_TOINSTALL = libcudd_caml.a libcudd_caml_debug.a libcudd_caml.so libcudd_caml_debug.so
CCINC_TOINSTALL = cuddaux.h cuddauxInt.h cudd_caml.h

#---------------------------------------
# Rules
#---------------------------------------

# Global rules
all: cudd.cmi cudd.cmo cudd.cmx cudd.cma cudd.cmxa libcudd_caml.a

ifneq ($(HAS_SHARED),)
all: libcudd_caml.so dllcudd_caml.so
endif

debug: libcudd_caml_debug.a
ifneq ($(HAS_SHARED),)
debug: libcudd_caml_debug.so dllcudd_caml_debug.so
endif

cuddtop: cudd.cma libcudd_caml.a
	$(OCAMLMKTOP) -verbose $(OCAMLFLAGS) -o $@ -ccopt -L. cudd.cma

# Example of compilation command
# If the library is installed somewhere, add a -I $(PATH) option
%.byte: %.ml cudd.cma
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" \
	cudd.cma example.ml
%.opt: %.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" \
	cudd.cmxa example.ml

# Example of compilation command if the autolink feature is disabled
# This may be useful for selecting debug version of libraries
%.opt2: %.ml cudd.cmxa libcudd_caml.a
	$(OCAMLOPT) $(OCAMLOPTFLAGS) $(OCAMLINC) -o $@ -cc "$(CC)" -noautolink \
	cudd.cmxa example.ml \
	-ccopt -L$(MLCUDDIDL_PREFIX)/lib -cclib -lcudd_caml_debug \
	-ccopt -L$(CAMLIDL_PREFIX)/lib/ocaml -cclib -lcamlidl \
	-ccopt -L$(CUDD_PREFIX)/lib -cclib "-lcudd -lmtr -lst -lcuddutil -lepd"

dist: $(IDLMODULES:%=%.idl) macros.m4 $(MLSRC) $(CCSRC) Makefile Makefile.config.model Makefile.cudd README Changes test_mtbdd.ml example.ml session.ml mlcuddidl.odoc mlcuddidl.pdf html sedscript_c sedscript_caml
	(cd ..; tar zcvf $(HOME)/mlcuddidl.tgz $(^:%=mlcuddidl/%))
install:
	mkdir -p $(INCDIR) $(LIBDIR) $(BINDIR)
	$(INSTALL) $(CCINC_TOINSTALL) $(INCDIR)
	for i in $(CCLIB_TOINSTALL) $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx); do \
		if test -f $$i; then $(INSTALL) $$i $(LIBDIR); fi; \
	done
	for i in dllcudd_caml.so dllcudd_caml_debug.so; do \
		if test -f $$i; then $(INSTALL) $$i $(LIBDIR); fi; \
	done
	for i in $(CCBIN_TOINSTALL); do \
		if test -f $$i; then $(INSTALL) $$i $(BINDIR); fi; \
	done

distclean: uninstall

uninstall:
	(cd $(INCDIR); /bin/rm -f cuddaux* cudd_caml*)
	(cd $(LIBDIR); /bin/rm -f $(MLLIB_TOINSTALL) $(MLLIB_TOINSTALLx) libcudd_caml*.a libcudd_caml*.so dllcudd_caml*.so)
	(cd $(BINDIR); /bin/rm -f $(CCBIN_TOINSTALL))

mostlyclean: clean
	/bin/rm -f $(IDLMODULES:%=%.ml) $(IDLMODULES:%=%.mli) $(IDLMODULES:%=%_caml.c) tmp/* html/*
	/bin/rm -f mlcuddidl.?? mlcuddidl.??? mlcuddidl.info example example.opt mlcuddidl.tex ocamldoc.tex *.dvi style.css ocamldoc.sty index.html

clean:
	/bin/rm -f cuddtop *.byte *.opt
	/bin/rm -f cuddaux.?? cuddaux.??? cuddaux.info
	/bin/rm -f *.[ao] *.so *.cm[ioxa] *.cmxa *.opt *.opt2 *.annot
	/bin/rm -f cmttb*
	/bin/rm -fr html

# CAML rules
cudd.cma: cudd.cmo libcudd_caml.a
	$(OCAMLMKLIB) -ocamlc "$(OCAMLC)" -verbose -o cudd -oc cudd_caml \
	cudd.cmo -lcudd -lmtr -lst -lcuddutil -lepd \
	-L$(CUDD_PREFIX)/lib -L$(CAMLIDL_PREFIX)/lib/ocaml \
	-Wl,-rpath,$(CUDD_PREFIX)/lib:$(CAMLIDL_PREFIX)/lib/ocaml
cudd.cmxa: cudd.cmx libcudd_caml.a
	$(OCAMLMKLIB) -ocamlopt "$(OCAMLOPT)" -verbose -o cudd -oc cudd_caml \
	cudd.cmx -lcudd -lmtr -lst -lcuddutil -lepd \
	-L$(CUDD_PREFIX)/lib -L$(CAMLIDL_PREFIX)/lib/ocaml \
	-Wl,-rpath,$(CUDD_PREFIX)/lib:$(CAMLIDL_PREFIX)/lib/ocaml

cudd.cmo cudd.cmi: $(MLOBJ)
	$(OCAMLC) $(OCAMLFLAGS) $(OCAMLINC) -pack -o cudd.cmo $(MLOBJ)

cudd.cmx: $(MLOBJx)
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -pack -o cudd.cmx $(MLOBJx)

libcudd_caml.a: $(CCMODULES:%=%.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@
libcudd_caml_debug.a: $(CCMODULES:%=%_debug.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

mac: cudd.cmo $(CCMODULES:%=%.o)
	$(OCAMLMKLIB) -v -ocamlc "$(OCAMLC)" -verbose -o cudd -oc cudd_caml \
	$^ -L$(CUDD_PREFIX)/lib -L$(CAMLIDL_PREFIX)/lib/ocaml \
	-lcudd -lmtr -lst -lepd -lcuddutil \
	-Wl,-rpath,$(CUDD_PREFIX)/lib:$(CAMLIDL_PREFIX)/lib/ocaml

libcudd_caml.so: $(CCMODULES:%=%.o)
	$(CC) -v $(CFLAGS) -shared -o $@ $(CCMODULES:%=%.o) \
	-L. -L$(CUDD_PREFIX)/lib -L$(CAMLIDL_PREFIX)/lib/ocaml \
	-lcudd -lmtr -lst -lcuddutil -lepd -lcamlidl \
	-Wl,-rpath,$(CUDD_PREFIX)/lib:$(CAMLIDL_PREFIX)/lib/ocaml
libcudd_caml_debug.so: $(CCMODULES:%=%_debug.o)
	$(CC) $(CFLAGS_DEBUG) -shared -o $@ $(CCMODULES:%=%_debug.o) \
	-L. -L$(CUDD_PREFIX)/lib -L$(CAMLIDL_PREFIX)/lib/ocaml \
	-lcudd_debug -lmtr -lst -lcuddutil -lepd -lcamlidl \
	-Wl,-rpath,$(CUDD_PREFIX)/lib:$(CAMLIDL_PREFIX)/lib/ocaml
dllcudd_caml.so: libcudd_caml.so
	ln -s $^ $@
dllcudd_caml_debug.so: libcudd_caml_debug.so
	ln -s $^ $@

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
-t "MLCUDDIDL: OCaml interface for CUDD library, version 2.1.0, 10/08/09" \
-latextitle 1,chapter -latextitle 2,section -latextitle 3,subsection -latextitle 4,subsubsection -latextitle 5,paragraph -latextitle 6,subparagraph \
-latex -intro mlcuddidl.odoc -o ocamldoc.tex man.mli bdd.mli add.mli vdd.mli mtbdd.mli mtbddc.mli mapleaf.mli user.mli custom.mli weakke.mli pWeakke.mli
	$(SED) -e 's/\\documentclass\[11pt\]{article}/\\documentclass[10pt,twosdie,a4paper]{book}\\usepackage{ae,fullpage,makeidx,fancyhdr}\\usepackage[ps2pdf]{hyperref}\\pagestyle{fancy}\\setlength{\\parindent}{0em}\\setlength{\\parskip}{0.5ex}\\sloppy\\makeindex\\author{Bertrand Jeannet}/' -e 's/\\end{document}/\\appendix\\printindex\\end{document}/' ocamldoc.tex >mlcuddidl.tex
	$(LATEX) mlcuddidl
	$(MAKEINDEX) mlcuddidl
	$(LATEX) mlcuddidl
	$(LATEX) mlcuddidl

html: mlcuddidl.odoc $(MLMODULES:%=%.mli)
	mkdir -p html
	$(OCAMLDOC) $(OCAMLINC) -html -d html -colorize-code -intro mlcuddidl.odoc $(MLMODULES:%=%.mli)

homepage: html mlcuddidl.pdf
	hyperlatex index
	cp -r index.html html mlcuddidl.pdf Changes \
		$(HOME)/web/mlxxxidl-forge/mlcuddidl
	chmod -R ugoa+rx $(HOME)/web/mlxxxidl-forge/mlcuddidl
	scp -r $(HOME)/web/mlxxxidl-forge/mlcuddidl johns:/home/wwwpop-art/people/bjeannet/mlxxxidl-forge
	ssh johns chmod -R ugoa+rx /home/wwwpop-art/people/bjeannet/mlxxxidl-forge/mlcuddidl


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
