/**CFile***********************************************************************

  FileName    [cuddauxAddApply.c]

  PackageName [cuddaux]

  Synopsis    [Variation of cuddAddApply.c module,
	       using a local cache (easier for interfacing
	       with external languages like OCaml]

  Description [Miscellaneous operations..]

	    External procedures included in this module:
		<ul>
		<li> Cuddaux_addApply1()
		<li> Cuddaux_addApply2()
		<li> Cuddaux_addTest2()
		<li> Cuddaux_addApply3()
		<li> Cuddaux_addAbstract()
		<li> Cuddaux_addBddAndAbstract()
		<li> Cuddaux_addApplyBddAndAbstract()
		</ul>
	    Internal procedures included in this module:
		<ul>
		<li> cuddauxAddApply1Recur()
		<li> cuddauxAddApply2Recur()
		<li> cuddauxAddApply3Recur()
		<li> cuddauxAddTest2Recur()
		<li> cuddauxAddAbstractRecur()
		<li> cuddauxAddBddAndAbstractRecur()
		<li> cuddauxAddApplyBddAndAbstractRecur()
		</ul>
		]

  Author      [Bertrand Jeannet]

  Copyright   []

******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "cuddInt.h"
#include "util.h"
#include "st.h"

#include "cuddauxInt.h"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int bddCheckPositiveCube (DdManager *manager, DdNode *cube);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Applies op to the discriminants of f.]

  Description [Applies op to the discriminants of f.
  Returns a pointer to the result if succssful; NULL otherwise.

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addApply1(DdManager * dd,
		  DdHashTable** table,
		  /* if table==NULL, use global cache,
		     otherwise, if *table!=NULL, use the hashtable
		     otherwise, creates an hashtable and stores it in *table */
		  DDAUX_IDOP pid,
		  /* Identifier of the operation (for use in global cache */
		  DDAUX_AOP1 op,
		  /* The operation itself */
		  DdNode * f)
{
  DdNode *res;
  int tableNULL=(*table==NULL);

  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache after garbage collection  */
    if (dd->reordered == 1 && !tableNULL){
      assert(*table==NULL);
      *table = cuddHashTableInit(dd,1,2);
      if (*table == NULL) return(NULL);
    }
    dd->reordered = 0;
    res = cuddauxAddApply1Recur(dd,*table,pid,op,f);
  } while (dd->reordered == 1);
  return(res);

} /* end of Cuddaux_addApply1 */

/**Function********************************************************************

  Synopsis    [Applies op to the corresponding discriminants of f and g.]

  Description [Applies op to the corresponding discriminants of f and g.
  Returns a pointer to the result if succssful; NULL otherwise.

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addApply2(DdManager * dd,
		  DdHashTable** table,
		  DDAUX_IDOP pid,
		  int commutative,
		  DDAUX_AOP2 op,
		  DdNode * f,
		  DdNode * g)
{
  DdNode *res;
  int tableNULL=(*table==NULL);

  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache after garbage collection  */
    if (dd->reordered == 1 && !tableNULL){
      assert(*table==NULL);
      *table = cuddHashTableInit(dd,2,2);
      if (*table == NULL) return(NULL);
    }
    dd->reordered = 0;
    res = cuddauxAddApply2Recur(dd,*table,pid,commutative,op,f,g);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addApply2 */

/**Function********************************************************************

  Synopsis    [Applies op to the corresponding discriminants of f and g.]

  Description [Applies op to the corresponding discriminants of f and g.
  Returns a pointer to the result if succssful; NULL otherwise.

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
Cuddaux_addTest2(DdManager * dd,
		 DdHashTable** table,
		 DDAUX_IDOP pid,
		 int commutative,
		 DDAUX_AOP2 op,
		 DdNode * f,
		 DdNode * g)
{
  DdNode *res;
  int ret;
  int tableNULL=(*table==NULL);

  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache after garbage collection  */
    if (dd->reordered == 1 && !tableNULL){
      assert(*table==NULL);
      *table = cuddHashTableInit(dd,2,2);
      if (*table == NULL) return(-1);
    }
    dd->reordered = 0;
    res = cuddauxAddTest2Recur(dd,*table,pid,commutative,op,f,g);
  } while (dd->reordered == 1);
  ret = res==NULL ? (-1) : (res==DD_ONE(dd));
  return ret;
} /* end of Cuddaux_addTest2 */

/**Function********************************************************************

  Synopsis    [Applies op to the corresponding discriminants of f, g and h.]

  Description [Applies op to the corresponding discriminants of f, g and h.
  Returns a pointer to the result if succssful; NULL otherwise.

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

DdNode*
Cuddaux_addApply3(DdManager * dd,
		  DdHashTable** table,
		  DDAUX_IDOP pid,
		  DDAUX_AOP3 op,
		  DdNode * f,
		  DdNode * g,
		  DdNode * h)
{
  DdNode *res;
  int tableNULL=(*table==NULL);

  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache after garbage collection  */
    if (dd->reordered == 1 && !tableNULL){
      assert(*table==NULL);
      *table = cuddHashTableInit(dd,3,2);
      if (*table == NULL) return(NULL);
    }
    dd->reordered = 0;
    res = cuddauxAddApply3Recur(dd,*table,pid,op,f,g,h);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addApply3 */

/**Function********************************************************************

  Synopsis:
	       exist(ite(var,cube,false),ite(var,f+,f-)) = 
	         op(exist(cube,f+),exist(cube,f-))
	       exist(true,f) = f

  Assumptions:
	       op(f,f) = f,

  SideEffects [None

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addAbstract(DdManager * dd,
		    DdHashTable** table,
		    DdHashTable** tableop,
		    DDAUX_IDOP pid,
		    DDAUX_AOP2 op,
		    DdNode * f,
		    DdNode * cube)

{
  DdNode *res;
  int tableNULL=(*table==NULL);
  int tableopNULL=(*tableop==NULL);

  if (bddCheckPositiveCube(dd, cube) == 0) {
    (void) fprintf(dd->err,
		   "Error: Can only abstract positive cubes\n");
    dd->errorCode = CUDD_INVALID_ARG;
    return(NULL);
  }
  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache(s) after garbage collection  */
    if (dd->reordered == 1){
      if (!tableNULL){
	assert(*table==NULL);
	*table = cuddHashTableInit(dd,2,2);
	if (*table == NULL) return(NULL);
      }
      if (!tableopNULL){
	assert(*tableop==NULL);
	*tableop = cuddHashTableInit(dd,2,2);
	if (*tableop == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  return(NULL);
	}
      }
    }
    dd->reordered = 0;
    res = cuddauxAddAbstractRecur(dd, *table, *tableop, pid, op, f, cube);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addAbstract */

/**Function********************************************************************

  Synopsis:    exist(cube,op1(f))

	       existapply(ite(var,cube,false),ite(var,f+,f-)) = 
	         op(existapply(cube,f+),existapply(cube,f-))
	       existapply(true,f) = op1 f 


  Assumptions:
	       op(f,f) = f,

  SideEffects [None

  Be careful to put a hook to reinitialize the table in case of reordering
  ]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addApplyAbstract(DdManager * dd,
			 DdHashTable** table,
			 DdHashTable** tableop,
			 DdHashTable** tableop1,
			 DDAUX_IDOP pid,
			 DDAUX_IDOP pid1,
			 DDAUX_AOP2 op,
			 DDAUX_AOP1 op1,
			 DdNode * f,
			 DdNode * cube)
{
  DdNode *res;
  int tableNULL=(*table==NULL);
  int tableopNULL=(*tableop==NULL);
  int tableop1NULL=(*tableop1==NULL);

  if (bddCheckPositiveCube(dd, cube) == 0) {
    (void) fprintf(dd->err,
		   "Error: Can only abstract positive cubes\n");
    dd->errorCode = CUDD_INVALID_ARG;
    return(NULL);
  }
  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache(s) after garbage collection  */
    if (dd->reordered == 1){
      if (!tableNULL){
	assert(*table==NULL);
	*table = cuddHashTableInit(dd,2,2);
	if (*table == NULL) return(NULL);
      }
      if (!tableopNULL){
	assert(*tableop==NULL);
	*tableop = cuddHashTableInit(dd,2,2);
	if (*tableop == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  return(NULL);
	}
      }
      if (!tableop1NULL){
	assert(*tableop1==NULL);
	*tableop1 = cuddHashTableInit(dd,1,2);
	if (*tableop1 == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  cuddHashTableQuit(*tableop);
	  *tableop = NULL;
	  return(NULL);
	}
      }
    }
    dd->reordered = 0;
    res = cuddauxAddApplyAbstractRecur(dd, *table, *tableop, *tableop1, pid, pid1, op, op1, f, cube);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addAbstract */

/**Function********************************************************************

  Synopsis:    exist(cube,ite(f,g,background))

               existand(ite(var,cube,false),ite(var,f+,f-),ite(var,g+,g-)) =
	         op(existand(cube,f+,g+),existand(cube,f-,g-)

	       existand(cube,true,g) = exist(cube,g)
	       existand(cube,false,g) = background

	       existand(true,f,g) = ite(f,g,background)

  Assumptions:
	       op(f,f) = f,
	       false AND f = background
	       true AND f = f

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addBddAndAbstract(DdManager * dd,
			  DdHashTable** table,
			  DdHashTable** tableop,
			  DDAUX_IDOP pid,
			  DDAUX_AOP2 op,
			  DdNode * f, /* BDD */
			  DdNode * g, /* ADD */
			  DdNode * cube, /* BDD (cube) */
			  DdNode * background /* CST ADD */)
{
  DdNode *res;
  int tableNULL=(*table==NULL);
  int tableopNULL=(*tableop==NULL);

  if (bddCheckPositiveCube(dd, cube) == 0) {
    (void) fprintf(dd->err,
		   "Error: Can only abstract positive cubes\n");
    dd->errorCode = CUDD_INVALID_ARG;
    return(NULL);
  }
  assert(dd->reordered == 0);
  do {
    /* Reinitialize local cache(s) after garbage collection  */
    if (dd->reordered == 1){
      if (!tableNULL){
	assert(*table==NULL);
	*table = cuddHashTableInit(dd,2,2);
	if (*table == NULL) return(NULL);
      }
      if (!tableopNULL){
	assert(*tableop==NULL);
	*tableop = cuddHashTableInit(dd,2,2);
	if (*tableop == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  return(NULL);
	}
      }
    }
    dd->reordered = 0;
    res = cuddauxAddBddAndAbstractRecur(dd, *table, *tableop, pid, op, f, g, cube, background);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addApplyBddAndAbstract */

/**Function********************************************************************

  Synopsis:    exist(cube,ite(f,op1(g),background))

  Assumptions:
	       existop(f,f) = f,
	       false AND f = background
	       true AND f = f

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Cuddaux_addApplyBddAndAbstract(DdManager * dd,
			       DdHashTable** table,
			       DdHashTable** tableop,
			       DdHashTable** tableop1,
			       DDAUX_IDOP pid,
			       DDAUX_IDOP pid1,
			       DDAUX_AOP2 op,
			       DDAUX_AOP1 op1,
			       DdNode * f, /* BDD */
			       DdNode * G, /* ADD */
			       DdNode * cube, /* BDD (cube) */
			       DdNode * background /* CST ADD */)
{
  DdNode *res;
  int tableNULL=(*table==NULL);
  int tableopNULL=(*tableop==NULL);
  int tableop1NULL=(*tableop==NULL);

  if (bddCheckPositiveCube(dd, cube) == 0) {
    (void) fprintf(dd->err,
		   "Error: Can only abstract positive cubes\n");
    dd->errorCode = CUDD_INVALID_ARG;
    return(NULL);
  }
  assert(dd->reordered == 0);
  do {
    if (dd->reordered == 1){
      if (!tableNULL){
	assert(*table==NULL);
	*table = cuddHashTableInit(dd,2,2);
	if (*table == NULL) return(NULL);
      }
      if (!tableopNULL){
	assert(*tableop==NULL);
	*tableop = cuddHashTableInit(dd,2,2);
	if (*tableop == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  return(NULL);
	}
      }
      if (!tableop1NULL){
	assert(*tableop1==NULL);
	*tableop1 = cuddHashTableInit(dd,1,2);
	if (*tableop1 == NULL){
	  cuddHashTableQuit(*table);
	  *table = NULL;
	  cuddHashTableQuit(*tableop);
	  *tableop = NULL;
	  return(NULL);
	}
      }
    }
    dd->reordered = 0;
    res = cuddauxAddApplyBddAndAbstractRecur(dd, *table, *tableop, *tableop1, pid, pid1, op, op1, f, G, cube, background);
  } while (dd->reordered == 1);
  return(res);
} /* end of Cuddaux_addApplyBddAndAbstract */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cuddaux_addApply1.]

  Description [Performs the recursive step of Cuddaux_addApply1. Returns a
  pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [cuddauxAddApply2Recur]

******************************************************************************/
DdNode *
cuddauxAddApply1Recur(DdManager * dd,
		      DdHashTable* table,
		      DDAUX_IDOP pid,
		      DDAUX_AOP1 op,
		      DdNode * f)
{
  DdNode *res, *ft, *fe, *T, *E;
  unsigned int index;

  statLine(dd);
  assert (f->ref>=1);

  /* Check cache. */
  if (table){
    if (f->ref != 1 && (res = cuddHashTableLookup1(table,f)) != NULL) {
      return(res);
    }
  }
  else {
    res = cuddCacheLookup1(dd,pid,f);
    if (res != NULL) return(res);
  }

  /* Check terminal cases. */
  res = (*op)(dd,pid,f);
  if (res != NULL) goto cuddauxAddApply1Recur_end;
  else if (cuddIsConstant(f)) return NULL;

  /* Recursive step. */
  index = f->index;
  ft = cuddT(f);
  fe = cuddE(f);

  T = cuddauxAddApply1Recur(dd,table,pid,op,ft);
  if (T == NULL) return(NULL);
  cuddRef(T);

  E = cuddauxAddApply1Recur(dd,table,pid,op,fe);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd,T);
    return(NULL);
  }
  cuddRef(E);

  res = (T == E) ? T : cuddUniqueInter(dd,(int)index,T,E);
  if (res == NULL) {
    Cudd_RecursiveDeref(dd, T);
    Cudd_RecursiveDeref(dd, E);
    return(NULL);
  }
  cuddDeref(T);
  cuddDeref(E);

  /* Store result. */
 cuddauxAddApply1Recur_end:
  if (table){
    if (f->ref != 1) {
      ptrint fanout = (ptrint) f->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert1(table,f,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  else {
    cuddCacheInsert1(dd,pid,f,res);
  }
  return(res);
} /* end of cuddauxAddApply1Recur */

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_addApply2.]

  Description [Performs the recursive step of Cudd_addApply2. Returns a
  pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
cuddauxAddApply2Recur(DdManager * dd,
		      DdHashTable* table,
		      DDAUX_IDOP pid,
		      int commutative,
		      DDAUX_AOP2 op,
		      DdNode * f,
		      DdNode * g)
{
  DdNode *res,
    *fv, *fvn, *gv, *gvn,
    *T, *E;
  unsigned int ford, gord;
  unsigned int index;
  DD_CTFP cacheOp;

  statLine(dd);
  assert (f->ref>=1);
  assert (g->ref>=1);

  /* Check cache. */
  if (commutative && f>g){
    DdNode* t = f; f=g; g=t;
  }
  if (table){
    if ((f->ref != 1 || g->ref != 1) &&
	(res = cuddHashTableLookup2(table,f,g)) != NULL) {
      return(res);
    }
  }
  else {
    res = cuddCacheLookup2(dd,pid,f,g);
    if (res != NULL) return(res);
  }
  /* Check terminal cases. */
  res = (*op)(dd,pid,f,g);
  if (res != NULL) goto cuddauxAddApply2Recur_end;
  else if (cuddIsConstant(f) && cuddIsConstant(g)) return NULL;

  /* Recursive step. */
  ford = cuddI(dd,f->index);
  gord = cuddI(dd,g->index);
  if (ford <= gord) {
    index = f->index;
    fv = cuddT(f);
    fvn = cuddE(f);
  } else {
    index = g->index;
    fv = fvn = f;
  }
  if (gord <= ford) {
    gv = cuddT(g);
    gvn = cuddE(g);
  } else {
    gv = gvn = g;
  }

  T = cuddauxAddApply2Recur(dd,table,pid,commutative,op,fv,gv);
  if (T == NULL) return(NULL);
  cuddRef(T);

  E = cuddauxAddApply2Recur(dd,table,pid,commutative,op,fvn,gvn);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd,T);
    return(NULL);
  }
  cuddRef(E);

  res = (T == E) ? T : cuddUniqueInter(dd,(int)index,T,E);
  if (res == NULL) {
    Cudd_RecursiveDeref(dd, T);
    Cudd_RecursiveDeref(dd, E);
    return(NULL);
  }
  cuddDeref(T);
  cuddDeref(E);

  /* Store result. */
 cuddauxAddApply2Recur_end:
  if (table){
    if (f->ref != 1 || g->ref != 1) {
      ptrint fanout = (ptrint) f->ref * g->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert2(table,f,g,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  else {
    cuddCacheInsert2(dd,pid,f,g,res);
  }
  return(res);

} /* end of cuddauxAddApply2Recur */

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_addTest2.]

  Description [Performs the recursive step of Cudd_addTest2. Returns a
  pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
cuddauxAddTest2Recur(DdManager * dd,
		     DdHashTable* table,
		     DDAUX_IDOP pid,
		     int commutative,
		     DDAUX_AOP2 op,
		     DdNode * f,
		     DdNode * g)
{
  DdNode *res,*one,
    *fv, *fvn, *gv, *gvn;
  unsigned int ford, gord;
  unsigned int index;
  DD_CTFP cacheOp;

  /* Check terminal cases. Op may swap f and g to increase the
   * cache hit rate.
   */
  statLine(dd);
  assert (f->ref>=1);
  assert (g->ref>=1);

  one = DD_ONE(dd);

  /* Check cache. */
  if (commutative && f>g){
    DdNode* t = f; f=g; g=t;
  }
  if (table){
    if ((f->ref != 1 || g->ref != 1) &&
	(res = cuddHashTableLookup2(table,f,g)) != NULL) {
      return(res);
    }
  }
  else {
    res = cuddCacheLookup2(dd,pid,f,g);
    if (res != NULL) return(res);
  }
  /* Check terminal cases. */
  res = (*op)(dd,pid,f,g);
  if (res != NULL)  goto cuddauxAddTest2Recur_end;
  else if (cuddIsConstant(f) && cuddIsConstant(g)) return NULL;

  /* Recursive step. */
  ford = cuddI(dd,f->index);
  gord = cuddI(dd,g->index);
  if (ford <= gord) {
    index = f->index;
    fv = cuddT(f);
    fvn = cuddE(f);
  } else {
    index = g->index;
    fv = fvn = f;
  }
  if (gord <= ford) {
    gv = cuddT(g);
    gvn = cuddE(g);
  } else {
    gv = gvn = g;
  }

  res = cuddauxAddTest2Recur(dd,table,pid,commutative,op,fv,gv);
  if (res == NULL) return(NULL);
  if (res==one)
    res = cuddauxAddTest2Recur(dd,table,pid,commutative,op,fvn,gvn);
  if (res==NULL) return NULL;

 cuddauxAddTest2Recur_end:
  if (table){
    if (f->ref != 1 || g->ref != 1) {
      ptrint fanout = (ptrint) f->ref * g->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert2(table,f,g,res,fanout)) {
	return(NULL);
      }
    }
  }
  else {
    cuddCacheInsert2(dd,pid,f,g,res);
  }
  return(res);
} /* end of cuddauxAddTest2Recur */

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cuddaux_addApply3.]

  Description [Performs the recursive step of Cuddaux_addApply3. Returns a
  pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

DdNode*
cuddauxAddApply3Recur(
  DdManager * dd,
  DdHashTable* table,
  DDAUX_IDOP pid,
  DDAUX_AOP3 op,
  DdNode * f,
  DdNode * g,
  DdNode * h
  )
{
    DdNode *res,
	   *fv, *fvn, *gv, *gvn, *hv, *hvn,
	   *T, *E;
    unsigned int ford, gord, hord, ghord, ord;
    unsigned int index;

    statLine(dd);
    assert (f->ref>=1);
    assert (g->ref>=1);
    assert (h->ref>=1);

    /* If problem already solved, look up answer and return. */
    if (table){
      if ((f->ref != 1 || g->ref != 1 || h->ref != 1) &&
	  (res = cuddHashTableLookup3(table,f,g,h)) != NULL) {
	return(res);
      }
    }
    else {
      abort();
    }
    /* Check terminal cases. */
    res = (*op)(dd,pid,f,g,h);
    if (res != NULL) goto cuddauxAddApply3Recur_end;
    else if (cuddIsConstant(f) && cuddIsConstant(g) && cuddIsConstant(h)) return NULL;

    /* Recursive step. */
    ford = cuddI(dd,f->index);
    gord = cuddI(dd,g->index);
    hord = cuddI(dd,h->index);
    ghord = ddMin(gord,hord);
    ord = ddMin(ford,ghord);
    index = -1;
    if (ford==ord){
      index = f-> index;
      fv = cuddT(f);
      fvn = cuddE(f);
    }
    else {
      fv = fvn = f;
    }
    if (gord==ord){
      index = g-> index;
      gv = cuddT(g);
      gvn = cuddE(g);
    }
    else {
      gv = gvn = g;
    }
    if (hord==ord){
      index = h-> index;
      hv = cuddT(h);
      hvn = cuddE(h);
    }
    else {
      hv = hvn = h;
    }
    T = cuddauxAddApply3Recur(dd,table,pid,op,fv,gv,hv);
    if (T == NULL) return(NULL);
    cuddRef(T);

    E = cuddauxAddApply3Recur(dd,table,pid,op,fvn,gvn,hvn);
    if (E == NULL) {
	Cudd_RecursiveDeref(dd,T);
	return(NULL);
    }
    cuddRef(E);

    res = (T == E) ? T : cuddUniqueInter(dd,(int)index,T,E);
    if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
    }
    cuddDeref(T);
    cuddDeref(E);

    /* Do not keep the result if the reference count is only 1, since
    ** it will not be visited again.
    */
 cuddauxAddApply3Recur_end:
    if (f->ref != 1 || g->ref != 1 || h->ref != 1) {
	ptrint fanout = (ptrint) f->ref * g->ref * h->ref;
	cuddSatDec(fanout);
	if (!cuddHashTableInsert3(table,f,g,h,res,fanout)) {
	    Cudd_RecursiveDeref(dd, res);
	    return(NULL);
	}
    }
    return(res);

} /* end of cuddauxAddApply3Recur */


/**Function********************************************************************

  SideEffects [None]

  SeeAlso     [None]

******************************************************************************/
DdNode*
cuddauxAddAbstractRecur(DdManager * dd,
			DdHashTable* table,
			DdHashTable* tableop,
			DDAUX_IDOP pid,
			DDAUX_AOP2 op,
			DdNode * G, /* ADD */
			DdNode * cube /* BDD (cube) */)
{
  DdNode *gt, *ge, *cube2;
  DdNode *one, *zero, *res, *T, *E;
  unsigned int topf, topcube, top, index;

  statLine(dd);
  one = DD_ONE(dd);
  zero = Cudd_Not(one);

  /* Terminal cases. */
  if (cube==one || cuddIsConstant(G)) return G;
  /* Here we can skip the use of cuddI, because the operands are known
  ** to be non-constant.
  */
  top = dd->perm[G->index];
  topcube = dd->perm[cube->index];

  while (topcube < top) {
    cube = cuddT(cube);
    if (cube == one) {
      return G;
    }
    topcube = dd->perm[cube->index];
  }
  /* Now, topcube >= top. */

  /* Check cache. */
  if (table){
    if (G->ref != 1 || cube->ref != 1) {
      res = cuddHashTableLookup2(table, G, cube);
      if (res != NULL) {
	return(res);
      }
    }
  }
  else {
    res = cuddCacheLookup2(dd,pid+4,G,cube);
    if (res != NULL) return(res);
  }
  gt = cuddT(G);
  ge = cuddE(G);

  /* If topcube == top, quantify, else just recurse */
  cube2 = (topcube == top) ? cuddT(cube) : cube;

  T = cuddauxAddAbstractRecur(dd, table, tableop, pid, op, gt, cube2);
  if (T == NULL) return(NULL);
  /* If quantify: Special case: t==ge, implies that ge does not depend on the
     variables in Cube. One thus have
       ge OP (exist Cube ge) =
       ge OP ge =
       ge
    */
  if (topcube==top && T == ge) {
    res = ge;
    goto cuddauxAddAbstractRecur_end;
  }
  cuddRef(T);

  E = cuddauxAddAbstractRecur(dd, table, tableop, pid, op, ge, cube2);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd, T);
    return(NULL);
  }
  if (T == E){
    res = T;
    cuddDeref(T);
  }
  else {
    cuddRef(E);
    if (topcube == top) { /* quantify */
      res = cuddauxAddApply2Recur(dd,tableop,pid,1,op,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddRef(res);
      Cudd_RecursiveDeref(dd, T);
      Cudd_RecursiveDeref(dd, E);
      cuddDeref(res);
    }
    else { /* do not quantify */
      res = cuddUniqueInter(dd,(int)G->index,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddDeref(T);
      cuddDeref(E);
    }
  }
 cuddauxAddAbstractRecur_end:
  if (table){
    if (G->ref != 1 || cube->ref != 1){
      ptrint fanout = (ptrint) G->ref * cube->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert2(table,G,cube,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  else {
    cuddCacheInsert2(dd,pid+4,G,cube,res);
  }
  return (res);
} /* end of cuddauxAddAbstractRecur */

/**Function********************************************************************

  SideEffects [None]

  SeeAlso     [None]

******************************************************************************/
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
			     DdNode * cube /* BDD (cube) */)
{
  DdNode *gt, *ge, *cube2;
  DdNode *one, *zero, *res, *T, *E;
  unsigned int topf, topcube, top, index;

  statLine(dd);
  one = DD_ONE(dd);
  zero = Cudd_Not(one);

  /* Terminal cases. */
  if (cube==one || cuddIsConstant(G)){
    return cuddauxAddApply1Recur(dd,tableop1,pid1,op1,G);
  }
  /* Here we can skip the use of cuddI, because the operands are known
  ** to be non-constant.
  */
  top = dd->perm[G->index];
  topcube = dd->perm[cube->index];

  while (topcube < top) {
    cube = cuddT(cube);
    if (cube == one) {
      return G;
    }
    topcube = dd->perm[cube->index];
  }
  /* Now, topcube >= top. */

  /* Check cache. */
  if (table){
    if (G->ref != 1 || cube->ref != 1) {
      res = cuddHashTableLookup2(table, G, cube);
      if (res != NULL) {
	return(res);
      }
    }
  }
  else {
    res = cuddCacheLookup2(dd,pid+8,G,cube);
    if (res != NULL) return(res);
  }
  gt = cuddT(G);
  ge = cuddE(G);

  /* If topcube == top, quantify, else just recurse */
  cube2 = (topcube == top) ? cuddT(cube) : cube;

  T = cuddauxAddApplyAbstractRecur(dd, table, tableop, tableop1, pid, pid1, op, op1, gt, cube2);
  if (T == NULL) return(NULL);
  /* If quantify: Special case: t==ge, implies that ge does not depend on the
     variables in Cube. One thus have
       ge OP (exist Cube ge) =
       ge OP ge =
       ge
    */
  if (topcube==top && T == ge) {
    res = ge;
    goto cuddauxAddApplyAbstractRecur_end;
  }
  cuddRef(T);

  E = cuddauxAddApplyAbstractRecur(dd, table, tableop, tableop1, pid, pid1, op, op1, ge, cube2);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd, T);
    return(NULL);
  }
  if (T == E){
    res = T;
    cuddDeref(T);
  }
  else {
    cuddRef(E);
    if (topcube == top) { /* quantify */
      res = cuddauxAddApply2Recur(dd,tableop,pid,1,op,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddRef(res);
      Cudd_RecursiveDeref(dd, T);
      Cudd_RecursiveDeref(dd, E);
      cuddDeref(res);
    }
    else { /* do not quantify */
      res = cuddUniqueInter(dd,(int)G->index,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddDeref(E);
      cuddDeref(T);
    }
  }
 cuddauxAddApplyAbstractRecur_end:
  if (table){
    if (G->ref != 1 || cube->ref != 1){
      ptrint fanout = (ptrint) G->ref * cube->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert2(table,G,cube,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  else {
    cuddCacheInsert2(dd,pid+8,G,cube,res);
  }
  return (res);
} /* end of cuddauxAddApplyAbstractRecur */

/**Function********************************************************************

  SideEffects [None]

  SeeAlso     [None]

******************************************************************************/
DdNode*
cuddauxAddBddAndAbstractRecur(DdManager * dd,
			      DdHashTable* table,
			      DdHashTable* tableop,
			      DDAUX_IDOP pid,
			      DDAUX_AOP2 op,
			      DdNode * f, /* BDD */
			      DdNode * G, /* ADD */
			      DdNode * cube, /* BDD (cube) */
			      DdNode * background /* CST ADD */)
{
  DdNode *F, *ft, *fe, *gt, *ge, *cube2;
  DdNode *one, *zero, *res, *T, *E;
  unsigned int topf, topg, topcube, top, index;

  statLine(dd);
  one = DD_ONE(dd);
  zero = Cudd_Not(one);

  /* Terminal cases. */
  if (f==zero || G==background) return background;
  if (cube==one) return cuddauxAddIteRecur(dd,f,G,background);
  if (cuddIsConstant(G)){
    DdNode* res1 = cuddBddExistAbstractRecur(dd,f,cube);
    if (res1==NULL) return NULL;
    cuddRef(res1);
    DdNode* res = cuddauxAddIteRecur(dd,res1,G,background);
    Cudd_IterDerefBdd(dd,res1);
    return res;
  }
  /* f may still be constant one !
   */
  F = Cudd_Regular(f);
  topf = (f==one) ? CUDD_CONST_INDEX : dd->perm[F->index];
  topg = dd->perm[G->index];
  top = ddMin(topf, topg);
  topcube = dd->perm[cube->index];

  while (topcube < top) {
    cube = cuddT(cube);
    if (cube == one) {
      return cuddauxAddIteRecur(dd,f,G,background);
    }
    topcube = dd->perm[cube->index];
  }
  /* Now, topcube >= top. */

  /* Check cache. */
  if (table){
    if (F->ref != 1 || G->ref != 1 || cube->ref != 1) {
      res = cuddHashTableLookup3(table, f, G, cube);
      if (res != NULL) {
	return(res);
      }
    }
  }
  else {
    abort();
  }
  /* Decompose */
  if (topf == top) {
    index = F->index;
    ft = cuddT(F);
    fe = cuddE(F);
    if (Cudd_IsComplement(f)) {
      ft = Cudd_Not(ft);
      fe = Cudd_Not(fe);
    }
  } else {
    index = G->index;
    ft = fe = f;
  }
  if (topg == top) {
    gt = cuddT(G);
    ge = cuddE(G);
  } else {
    gt = ge = G;
  }

  /* If topcube == top, quantify, else just recurse */
  cube2 = (topcube == top) ? cuddT(cube) : cube;

  T = cuddauxAddBddAndAbstractRecur(dd, table, tableop, pid, op, ft, gt, cube2, background);
  if (T == NULL) return(NULL);
  /* If quantifiy: special case: t==ge, implies that ge does not depend on the
       variables in Cube. One thus have
       ge OP (exist cube (fe and ge)) =
       ge OP ite(exist fe,ge,background) =
       ite(exist fe,ge EXISTOP ge,ge EXISTOP background =
       ge
    */
  if (topcube==top && T == ge) {
    res = ge;
    goto cuddauxAddBddAndAbstractRecur_end;
  }
  cuddRef(T);
  E = cuddauxAddBddAndAbstractRecur(dd, table, tableop, pid, op, fe, ge, cube2, background);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd, T);
    return(NULL);
  }
  if (E == NULL) return(NULL);
  if (T == E || (topcube==top && E==background)){
    res = T;
    cuddDeref(T);
  }
  else if (topcube==top && T==background){
    res = E;
    Cudd_RecursiveDeref(dd, T);
  }
  else {
    cuddRef(E);
    if (topcube == top) { /* quantify */
      res = cuddauxAddApply2Recur(dd,tableop,pid,1,op,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddRef(res);
      Cudd_RecursiveDeref(dd, T);
      Cudd_RecursiveDeref(dd, E);
      cuddDeref(res);
    }
    else { /* recurse */
      res = cuddUniqueInter(dd,(int)G->index,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddDeref(E);
      cuddDeref(T);
    }
  }
 cuddauxAddBddAndAbstractRecur_end:
  if (table){
    if (F->ref != 1 || G->ref != 1 || cube->ref != 1){
      ptrint fanout = (ptrint) F->ref * G->ref * cube->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert3(table,f,G,cube,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  return (res);
} /* end of cuddauxAddBddAndAbstractRecur */

/**Function********************************************************************

  SideEffects [None]

  SeeAlso     [None]

******************************************************************************/
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
				   DdNode * background /* CST ADD */)
{
  DdNode *F, *ft, *fe, *gt, *ge, *cube2;
  DdNode *one, *zero, *res, *T, *E;
  unsigned int topf, topg, topcube, top, index;

  statLine(dd);
  one = DD_ONE(dd);
  zero = Cudd_Not(one);

  /* Terminal cases. */
  if (f==zero) return background;
  if (f==one && cube==one){
    return cuddauxAddApply1Recur(dd,tableop1,pid1,op1,G);
  }
  if (cuddIsConstant(G)){
    G = cuddauxAddApply1Recur(dd,tableop1,pid1,op1,G);
    if (G==background) return background;
    if (cube==one){
      return cuddauxAddIteRecur(dd,f,G,background);
    }
    else {
      DdNode* res1 = cuddBddExistAbstractRecur(dd,f,cube);
      if (res1==NULL) return NULL;
      cuddRef(res1);
      DdNode* res = cuddauxAddIteRecur(dd,res1,G,background);
      Cudd_IterDerefBdd(dd,res1);
      return res;
    }
  }
  /* f,cube may still be constant one ! */
  F = Cudd_Regular(f);
  topf = (f==one) ? CUDD_CONST_INDEX : dd->perm[F->index];
  topg = dd->perm[G->index];
  top = ddMin(topf, topg);
  topcube = dd->perm[cube->index];

  while (topcube < top) {
    cube = cuddT(cube);
    if (cube == one) {
      return cuddauxAddIteRecur(dd,f,G,background);
    }
    topcube = dd->perm[cube->index];
  }
  /* Now, topcube >= top. */

  /* Check cache. */
  if (table){
    if (F->ref != 1 || G->ref != 1 || cube->ref != 1) {
      res = cuddHashTableLookup3(table, f, G, cube);
      if (res != NULL) {
	return(res);
      }
    }
  }
  else {
    abort();
  }
  /* Decompose */
  if (topf == top) {
    index = F->index;
    ft = cuddT(F);
    fe = cuddE(F);
    if (Cudd_IsComplement(f)) {
      ft = Cudd_Not(ft);
      fe = Cudd_Not(fe);
    }
  } else {
    index = G->index;
    ft = fe = f;
  }
  if (topg == top) {
    gt = cuddT(G);
    ge = cuddE(G);
  } else {
    gt = ge = G;
  }

  /* If topcube == top, quantify, else just recurse */
  cube2 = (topcube == top) ? cuddT(cube) : cube;

  T = cuddauxAddApplyBddAndAbstractRecur(dd, table, tableop, tableop1, pid, pid1, op, op1, ft, gt, cube2, background);
  if (T == NULL) return(NULL);
  cuddRef(T);
  E = cuddauxAddApplyBddAndAbstractRecur(dd, table, tableop, tableop1, pid, pid1, op, op1, fe, ge, cube2, background);
  if (E == NULL) {
    Cudd_RecursiveDeref(dd, T);
    return(NULL);
  }
  if (E == NULL) return(NULL);
  if (T == E || (topcube==top && E==background)){
    res = T;
    cuddDeref(T);
  }
  else if (topcube==top && T==background){
    res = E;
    Cudd_RecursiveDeref(dd, T);
  }
  else {
    cuddRef(E);
    if (topcube == top) { /* quantify */
      res = cuddauxAddApply2Recur(dd,tableop,pid,1,op,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddRef(res);
      Cudd_RecursiveDeref(dd, T);
      Cudd_RecursiveDeref(dd, E);
      cuddDeref(res);
    }
    else { /* recurse */
      res = cuddUniqueInter(dd,(int)G->index,T,E);
      if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
      }
      cuddDeref(E);
      cuddDeref(T);
    }
  }
 cuddauxAddApplyBddAndAbstractRecur_end:
  if (table){
    if (F->ref != 1 || G->ref != 1 || cube->ref != 1){
      ptrint fanout = (ptrint) F->ref * G->ref * cube->ref;
      cuddSatDec(fanout);
      if (!cuddHashTableInsert3(table,f,G,cube,res,fanout)) {
	Cudd_RecursiveDeref(dd, res);
	return(NULL);
      }
    }
  }
  return (res);
} /* end of cuddauxAddApplyBddAndAbstractRecur */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Checks whether cube is an BDD representing the product of
  positive literals.]

  Description [Returns 1 in case of success; 0 otherwise.]

  SideEffects [None]

******************************************************************************/
static int
bddCheckPositiveCube(
  DdManager * dd,
  DdNode * cube)
{
    if (Cudd_IsComplement(cube)) return(0);
    if (cube == DD_ONE(dd)) return(1);
    if (cuddIsConstant(cube)) return(0);
    if (cuddE(cube) == Cudd_Not(DD_ONE(dd))) {
	return(bddCheckPositiveCube(dd, cuddT(cube)));
    }
    return(0);

} /* end of bddCheckPositiveCube */
