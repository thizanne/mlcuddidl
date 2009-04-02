/* $Id: cuddaux.h,v 1.1.1.1 2002/05/14 13:04:04 bjeannet Exp $ */

#ifndef __CUDDAUX_H__
#define __CUDDAUX_H__

#include "cudd.h"
#include "caml/mlvalues.h"

struct list_t {
  struct list_t* next;
  DdNode* node;
};
typedef struct list_t list_t;

/* Type of ternary apply operator. */
typedef void* DDAUX_IDOP;
typedef DdNode * (*DDAUX_AOP1)(DdManager *, DDAUX_IDOP pid, DdNode *);
typedef DdNode * (*DDAUX_AOP2)(DdManager *, DDAUX_IDOP pid, DdNode *, DdNode *);
typedef DdNode * (*DDAUX_AOP3)(DdManager *, DDAUX_IDOP pid, DdNode *, DdNode *, DdNode *);

/* File cuddauxAddIte.c */
/* f is a BDD, g and h are ADDs */
DdNode* Cuddaux_addIte(DdManager* dd, DdNode* f, DdNode* g, DdNode* h);
DdNode* Cuddaux_addBddAnd(DdManager* dd, DdNode* f, DdNode* g);
DdNode* Cuddaux_addIteConstant(DdManager* dd, DdNode* f, DdNode* g, DdNode* h);
DdNode* Cuddaux_addEvalConst(DdManager* dd, DdNode* f, DdNode* g);

/* File cuddauxBridge.c */
/* f is an ADD */
DdNode* Cuddaux_addTransfer(DdManager* ddS, DdManager* ddD, DdNode* f);

/* File cuddauxGenCof.c */
/* f and c are BDDs */
DdNode* Cuddaux_bddRestrict(DdManager * dd, DdNode * f, DdNode * c);
/* f is an ADD, c a BDD */
DdNode* Cuddaux_addRestrict(DdManager * dd, DdNode * f, DdNode * c);
DdNode* Cuddaux_addConstrain(DdManager * dd, DdNode * f, DdNode * c);

/* File cuddauxTDGenCof.c */
/* f and c are BDDs */
DdNode* Cuddaux_bddTDRestrict(DdManager* dd, DdNode* f, DdNode* c);
DdNode* Cuddaux_bddTDConstrain(DdManager* dd, DdNode* f, DdNode* c);
/* f is an ADD, c a BDD */
DdNode* Cuddaux_addTDRestrict(DdManager* dd, DdNode* f, DdNode* c);
DdNode* Cuddaux_addTDConstrain(DdManager* dd, DdNode* f, DdNode* c);
/* inf and sup are BDDs */
DdNode* Cuddaux_bddTDSimplify(DdManager* dd, DdNode* inf, DdNode* sup);
/* phi,f,g are ADDs with the distinguished background value */
DdNode* Cuddaux_addTDSimplify(DdManager* dd, DdNode* phi);

/* File cuddauxCompose.c */
/* f is an ADD, g a BDD */
DdNode* Cuddaux_addCompose(DdManager* dd, DdNode* f, DdNode* g, int v);
DdNode* Cuddaux_addVarMap(DdManager* dd, DdNode* f);
int Cuddaux_SetVarMap(DdManager *manager, int* array);
/* f is an ADD, vector an array of BDDs */
DdNode* Cuddaux_addVectorCompose(DdManager* dd, DdNode* f, DdNode** vector);
/* f is an ADD, vector an array of BDDs, op an operator applied to leaves */
DdNode* Cuddaux_addApplyVectorCompose(DdManager* dd, DD_MAOP op, DdNode* f, DdNode** vector);

/* File cuddauxAddApply.c */
/* f, g and h are ADDs */
DdNode* Cuddaux_addApply1(DdManager* dd, DdHashTable** table, DDAUX_IDOP pid, DDAUX_AOP1 op, DdNode* f);
DdNode* Cuddaux_addApply2(DdManager* dd, DdHashTable** table, DDAUX_IDOP pid, int commutative, DDAUX_AOP2 op, DdNode* f, DdNode* g);
int Cuddaux_addTest2(DdManager* dd, DdHashTable** table, DDAUX_IDOP pid, int commutative, DDAUX_AOP2 op, DdNode* f, DdNode* g);
DdNode* Cuddaux_addApply3(DdManager* dd, DdHashTable** table, DDAUX_IDOP pid, DDAUX_AOP3 op, DdNode* f, DdNode* g, DdNode* h);
DdNode* Cuddaux_addAbstract(DdManager* dd, DdHashTable** table, DdHashTable** tableop, DDAUX_IDOP pid, DDAUX_AOP2 op, DdNode* f, DdNode* cube);
DdNode* Cuddaux_addApplyAbstract(DdManager* dd, DdHashTable** table, DdHashTable** tableop, DdHashTable** tableop1, DDAUX_IDOP pid, DDAUX_IDOP pid1, DDAUX_AOP2 op, DDAUX_AOP1 op1, DdNode* f, DdNode* cube);
DdNode* Cuddaux_addBddAndAbstract(DdManager* dd, DdHashTable** table, DdHashTable** tableop, DDAUX_IDOP pid, DDAUX_AOP2 op, DdNode* f, DdNode* g, DdNode* cube, DdNode* background);
DdNode* Cuddaux_addApplyBddAndAbstract(DdManager* dd, DdHashTable** table, DdHashTable** tableop, DdHashTable** tableop1, DDAUX_IDOP pid, DDAUX_IDOP pid1, DDAUX_AOP2 op, DDAUX_AOP1 op1, DdNode* f, DdNode* g, DdNode* cube, DdNode* background);

/* File cuddauxMisc.c */
/* f is a BDD/ADD node */
DdNode* Cuddaux_Support(DdManager* dd, DdNode* f);
int Cuddaux_SupportSize(DdManager* dd, DdNode* f);
int Cuddaux_ClassifySupport(DdManager* dd, DdNode* f, DdNode* g, DdNode** common, DdNode** onlyF, DdNode** onlyG);
/* f is a BDD/ADD node and var a projection function */
int Cuddaux_IsVarIn(DdManager* dd, DdNode* f, DdNode* var);
/* f is a BDD/ADD node and level a level. */
list_t* Cuddaux_NodesBelowLevel(DdManager* dd, DdNode* f, int level, size_t max, size_t* psize, int take_background);
void list_free(list_t* l);
/* f and h are ADDs */
DdNode* Cuddaux_addGuardOfNode(DdManager* dd, DdNode* f, DdNode* h);

/* File cuddauxAddCamlTable.c */
DdNode* Cuddaux_addCamlConst(DdManager * unique, value value);
int Cuddaux_addCamlPreGC(DdManager* unique, const char* s, void* data);

#endif
