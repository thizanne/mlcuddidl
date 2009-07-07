/* $Id: cuddauxInt.h,v 1.1.1.1 2002/05/14 13:04:04 bjeannet Exp $ */

#ifndef __CUDDAUXINT_H__
#define __CUDDAUXINT_H__

#include "cuddaux.h"
#include "cuddInt.h"

/**Macro***********************************************************************

  Synopsis    [Returns the background node.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
#define DD_BACKGROUND(dd)		((dd)->background)

typedef union cuddauxType {
  CUDD_VALUE_TYPE dbl;	/* for constant nodes */
  value value;            /* for constant nodes */
  DdChildren kids;	/* for internal nodes */
} cuddauxType;
typedef struct cuddauxDdNode {
  DdHalfWord index;
  DdHalfWord ref;	/* reference count */
  DdNode *next;		/* next pointer for unique table */
  cuddauxType type;
} cuddauxDdNode;

#define cuddauxCamlV(_node_) (((cuddauxDdNode*)_node_)->type.value)

/* File cuddauxAddIte.c */
/* f is a BDD, g and h are ADDs */
DdNode* cuddauxAddIteRecur(DdManager* dd, DdNode* f, DdNode* g, DdNode* h);
DdNode* cuddauxAddBddAndRecur(DdManager* dd, DdNode* f, DdNode* g);

/* File cuddauxBridge.c */
/* f is an ADD */
DdNode* cuddauxAddTransfer(DdManager* ddS, DdManager* ddD, DdNode* f);
DdNode* cuddauxAddCamlTransfer(DdManager* ddS, DdManager* ddD, DdNode* f);

/* File cuddauxGenCof.c */
DdNode* cuddauxAddRestrictRecur(DdManager * dd, DdNode * f, DdNode * c);
DdNode* cuddauxAddConstrainRecur(DdManager * dd, DdNode * f, DdNode * c);

/* File cuddauxTDGenCof.c */
/* inf and sup are BDDs */
DdNode* cuddauxBddTDSimplifyRecur(DdManager* dd, DdNode* inf, DdNode* sup);
/* phi,f,g are ADDs with the distinguished background value */
DdNode* cuddauxAddTDSimplifyRecur(DdManager* dd, DdNode* phi);
DdNode* cuddauxAddTDUnify(DdManager* dd, DdNode* f, DdNode* g);

/* File cuddauxCompose.c */
/* f is an ADD, vector an array of BDDs */
DdNode* cuddauxAddVarMapRecur(DdManager *manager, DdNode* f);
DdNode* cuddauxAddComposeRecur(DdManager* dd, DdNode* f, DdNode* g, DdNode* proj);
DdNode* cuddauxAddVectorComposeRecur(
  DdManager * dd /* DD manager */,
  DdHashTable * table /* computed table */,
  DdNode * f /* ADD in which to compose */,
  DdNode ** vector /* functions to substitute */,
  int  deepest /* depth of deepest substitution */);
DdNode* cuddauxAddApplyVectorComposeRecur(
  DdManager * dd /* DD manager */,
  DdHashTable * table /* computed table */,
  DD_MAOP op,
  DdNode * f /* ADD in which to compose */,
  DdNode ** vector /* functions to substitute */);

/* File cuddauxAddApply.c */
/* f, g and h are ADDs */

DdNode* cuddauxAddApply1Recur(DdManager* dd, DdHashTable* table, DDAUX_IDOP pid, DDAUX_AOP1 op, DdNode* f);
DdNode* cuddauxAddApply2Recur(DdManager* dd, DdHashTable* table, DDAUX_IDOP pid, int commutative, DDAUX_AOP2 op, DdNode* f, DdNode* g);
DdNode* cuddauxAddTest2Recur(DdManager* dd, DdHashTable* table, DDAUX_IDOP pid, int commutative, DDAUX_AOP2 op, DdNode* f, DdNode* g);
DdNode* cuddauxAddApply3Recur(DdManager* dd, DdHashTable* table, DDAUX_IDOP pid, DDAUX_AOP3 op, DdNode* f, DdNode* g, DdNode* h);
DdNode*
cuddauxAddAbstractRecur(DdManager * dd,
			DdHashTable* table,
			DdHashTable* tableop,
			DDAUX_IDOP pid,
			DDAUX_AOP2 op,
			DdNode * G, /* ADD */
			DdNode * cube /* BDD (cube) */);
DdNode*
cuddauxAddApplyAbstractRecur(DdManager * dd,
			     DdHashTable* table,
			     DdHashTable* tableop,
			     DdHashTable* tableop1,
			     DDAUX_IDOP pid,
			     DDAUX_IDOP pid1,
			     DDAUX_AOP2 op,
			     DDAUX_AOP1 op1,
			     DdNode * G, /* ADD */
			     DdNode * cube /* BDD (cube) */);
DdNode*
cuddauxAddBddAndAbstractRecur(DdManager * dd,
			      DdHashTable* table,
			      DdHashTable* tableop,
			      DDAUX_IDOP pid,
			      DDAUX_AOP2 op,
			      DdNode * f, /* BDD */
			      DdNode * G, /* ADD */
			      DdNode * cube, /* BDD (cube) */
			      DdNode * background /* CST ADD */);
DdNode*
cuddauxAddApplyBddAndAbstractRecur(DdManager * dd,
				   DdHashTable* table,
				   DdHashTable* tableop,
				   DdHashTable* tableop1,
				   DDAUX_IDOP pid,
				   DDAUX_IDOP pid1,
				   DDAUX_AOP2 op,
				   DDAUX_AOP1 op1,
				   DdNode * f, /* BDD */
				   DdNode * G, /* ADD */
				   DdNode * cube, /* BDD (cube) */
				   DdNode * background /* CST ADD */);

/* File cuddauxMisc.c */
/* f is a BDD/ADD node */
DdNode* cuddauxSupportRecur(DdManager* dd, DdNode* f);
/* f is a BDD/ADD node and index the index of a variable.*/
DdNode* cuddauxIsVarInRecur(DdManager* manager, DdNode* Var, DdNode* f);
/* f and h are ADDs */
DdNode* cuddauxAddGuardOfNodeRecur(DdManager* manager, DdNode* f, DdNode* h);

/* File cuddauxAddCamlTable.c */
DdNode* cuddauxUniqueType(int is_value, DdManager* man, cuddauxType* type);
void cuddauxAddCamlConstRehash(DdManager* unique, int offset);

/* Cache tags for 3-operand operators.
   Look at cuddInt.h for already used tags.
   Begins from the end */

#define DDAUX_ADD_ITE_TAG            0xee
#define DDAUX_ADD_ITE_CONSTANT_TAG   0xea
#define DDAUX_ADD_COMPOSE_RECUR_TAG  0xe6

#endif
