/* Conversion of datatypes and common functions */

/* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "caml/fail.h"
#include "caml/alloc.h"
#include "caml/custom.h"
#include "caml/memory.h"
#include "caml/callback.h"
#include "caml/camlidlruntime.h"
#include "cudd_caml.h"

/* %======================================================================== */
/* \section{Global tuning (Garbage collection)} */
/* %======================================================================== */

static mlsize_t camlidl_cudd_heap = 1 << 20;
static value camlidl_cudd_gc_fun = Val_unit;
static value camlidl_cudd_reordering_fun = Val_unit;

static char camlidl_cudd_msg[160];

value camlidl_cudd_set_gc(value _v_heap, value _v_gc, value _v_reordering)
{
  CAMLparam3(_v_heap,_v_gc,_v_reordering);
  int firstcall;
  
  camlidl_cudd_heap = Int_val(_v_heap);
  firstcall = (camlidl_cudd_gc_fun==Val_unit);
  camlidl_cudd_gc_fun = _v_gc;
  camlidl_cudd_reordering_fun = _v_reordering;
  if (firstcall){
    caml_register_global_root(&camlidl_cudd_gc_fun);
    caml_register_global_root(&camlidl_cudd_reordering_fun);
  }
  CAMLreturn(Val_unit);
}

int camlidl_cudd_garbage(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_gc_fun==Val_unit){
      fprintf(stderr,"mlcuddidl: cudd_caml.o: internal error: the \"let _ = set_gc ...\" line in manager.ml has not been executed\n");
      abort();
  }
  caml_callback(camlidl_cudd_gc_fun,Val_unit);
  return 1;
}

int camlidl_cudd_reordering(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_reordering_fun==Val_unit){
    fprintf(stderr,"mlcuddidl: cudd_caml.o: internal error: the \"let _ = set_gc ...\" line in manager.ml has not been executed\n");
    abort();
  }
  camlidl_cudd_custom_hook(dd,s,data);
  caml_callback(camlidl_cudd_reordering_fun,Val_unit);
  return 1;
}

/* %======================================================================== */
/* \section{Custom datatypes} */
/* %======================================================================== */

/* %------------------------------------------------------------------------ */
/* \subsection{Custom functions} */
/* %------------------------------------------------------------------------ */


/* \subsubsection{Managers} */

void camlidl_cudd_man_free(struct man__t* man)
{
  assert(man->count>=1);
  if (man->count<=1){
    assert(Cudd_CheckZeroRef(man->man)==0);
    Cudd_Quit(man->man);
    free(man);
  }
  else if (man->count != SIZE_MAX)
    man->count--;
}

void camlidl_custom_man_finalize(value val)
{
  struct man__t* man = man_of_vmanager(val);
  camlidl_cudd_man_free(man);
}
int camlidl_custom_man_compare(value val1, value val2)
{
  CAMLparam2(val1,val2);
  int res;
  DdManager* man1 = DdManager_of_vmanager(val1);
  DdManager* man2 = DdManager_of_vmanager(val2);
  res = (long)man1==(long)man2 ? 0 : (long)man1<(long)man2 ? -1 : 1;
  CAMLreturnT(int,res);
}
long camlidl_custom_man_hash(value val)
{
  CAMLparam1(val);
  DdManager* man = DdManager_of_vmanager(val);
  long hash = (long)man;
  CAMLreturnT(long,hash);
}

struct custom_operations camlidl_custom_manager = {
  "camlidl_cudd_custom_node",
  &camlidl_custom_man_finalize,
  &camlidl_custom_man_compare,
  &camlidl_custom_man_hash,
  custom_serialize_default,
  custom_deserialize_default
};

/* \subsubsection{Standard nodes (BDDs \& ADDs)} */

void camlidl_custom_node_finalize(value val)
{
  node__t* no = node_of_vnode(val);
  DdNode* node = no->node;
  assert (Cudd_Regular(node)->ref >= 1);
  Cudd_RecursiveDeref(no->man->man,node);
  camlidl_cudd_man_free(no->man);
}
int camlidl_custom_node_compare(value val1, value val2)
{
  CAMLparam2(val1,val2);
  int res;
  DdManager* man1 = DdManager_of_vnode(val1);
  DdNode* node1 = DdNode_of_vnode(val1);
  DdManager* man2 = DdManager_of_vnode(val2);
  DdNode* node2 = DdNode_of_vnode(val2);

  res = (long)man1==(long)man2 ? 0 : ( (long)man1<(long)man2 ? -1 : 1);
  if (res==0)
    res = (long)node1==(long)node2 ? 0 : ( (long)node1<(long)node2 ? -1 : 1);
  CAMLreturnT(int,res);
}
long camlidl_custom_node_hash(value val)
{
  CAMLparam1(val);
  DdNode* node = DdNode_of_vnode(val);
  long hash = (long)node;
  CAMLreturnT(long,hash);
}

struct custom_operations camlidl_custom_node = {
  "camlidl_cudd_custom_node",
  &camlidl_custom_node_finalize,
  &camlidl_custom_node_compare,
  &camlidl_custom_node_hash,
  custom_serialize_default,
  custom_deserialize_default
};

/* \subsubsection{BDD nodes} */

void camlidl_custom_bdd_finalize(value val)
{
  node__t* no = node_of_vnode(val);
  DdNode* node = no->node;
  assert((Cudd_Regular(node))->ref >= 1);
  Cudd_IterDerefBdd(no->man->man,node);
  camlidl_cudd_man_free(no->man);
}

struct custom_operations camlidl_custom_bdd = {
  "camlidl_cudd_custom_bdd",
  &camlidl_custom_bdd_finalize,
  &camlidl_custom_node_compare,
  &camlidl_custom_node_hash,
  custom_serialize_default,
  custom_deserialize_default
};

/* %------------------------------------------------------------------------ */
/* \subsection{ML/C conversion functions} */
/* %------------------------------------------------------------------------ */

/* \subsubsection{Managers} */
value camlidl_cudd_man_c2ml(man__t* man)
{
  value val;
  if((*man)->man==NULL)
    failwith("Cudd: a function returned a null manager");
  val = caml_alloc_custom(&camlidl_custom_manager, sizeof(man__t*), 0, 1);
  *((man__t*)(Data_custom_val(val))) = *man;
  managerRef(*man);
  return val;
}
void camlidl_cudd_man_ml2c(value val, man__t* man)
{
  *man = *((man__t*)(Data_custom_val(val)));
}

/* \subsubsection{Standard nodes (BDDs \& ADDs)} */

#ifndef NDEBUG
int node_compteur = 0;
int bdd_compteur=0;
#define START_node 0
#define START_bdd 0
#define FREQ_node 500
#define FREQ_bdd 5000
#endif

value camlidl_cudd_node_c2ml(struct node__t* no)
{
  value val;

  if(no->node==0){
    Cudd_ErrorType err = Cudd_ReadErrorCode(no->man->man);
    Cudd_ClearErrorCode(no->man->man);
    char *s;
    switch(err){
    case CUDD_NO_ERROR: s = "CUDD_NO_ERROR"; break;
    case CUDD_MEMORY_OUT: s = "CUDD_MEMORY_OUT"; break;
    case CUDD_TOO_MANY_NODES: s = "CUDD_TOO_MANY_NODES"; break;
    case CUDD_MAX_MEM_EXCEEDED: s = "CUDD_MAX_MEM_EXCEEDED"; break;
    case CUDD_INVALID_ARG: s = "CUDD_INVALID_ARG"; break;
    case CUDD_INTERNAL_ERROR: s = "CUDD_INTERNAL_ERROR"; break;
    default: s = "CUDD_UNKNOWN"; break;
    }
    sprintf(camlidl_cudd_msg,
	    "Cudd: a function returned a null ADD/BDD node; ErrorCode = %s",
	    s);
    failwith(camlidl_cudd_msg);
  }
  cuddRef(no->node);
  managerRef(no->man);
  /*
  caml_gc_full_major(Val_unit);
  cuddGarbageCollect(no->man->man,1);
  assert(Cudd_CheckKeys(no->man->man)==0);
  assert(Cudd_DebugCheck(no->man->man)==0);
  */
  /*
#ifndef NDEBUG
  node_compteur++;
  if (node_compteur > START_node && node_compteur % FREQ_node == 0){
    int res1,res2;
    fprintf(stderr,"node_check(%d,%d)...",node_compteur,bdd_compteur);
    gc_full_major(Val_unit);
    res1 = Cudd_ReduceHeap(no->man,CUDD_REORDER_NONE,0);
    res2 = Cudd_DebugCheck(no->man);
    if (!res1 || res2){
      fprintf(stderr,"node\nnode_compteur=%d, bdd_compteur=%d\n",
	      node_compteur,bdd_compteur);
      abort();
    }
    fprintf(stderr,"done\n");
  }
#endif
  */

  val = caml_alloc_custom(&camlidl_custom_node, sizeof(struct node__t), 1, camlidl_cudd_heap);
  ((node__t*)(Data_custom_val(val)))->man = no->man;
  ((node__t*)(Data_custom_val(val)))->node = no->node;
  return val;
}
void camlidl_cudd_node_ml2c(value val, struct node__t *node)
{
  node->man = ((node__t*)(Data_custom_val(val)))->man;
  node->node = ((node__t*)(Data_custom_val(val)))->node;
}

/* \subsubsection{BDD nodes} */

value camlidl_cudd_bdd_c2ml(struct node__t* bdd)
{
  value val;

  if(bdd->node==0){
    Cudd_ErrorType err = Cudd_ReadErrorCode(bdd->man->man);
    char *s;
    switch(err){
    case CUDD_NO_ERROR: s = "CUDD_NO_ERROR"; break;
    case CUDD_MEMORY_OUT: s = "CUDD_MEMORY_OUT"; break;
    case CUDD_TOO_MANY_NODES: s = "CUDD_TOO_MANY_NODES"; break;
    case CUDD_MAX_MEM_EXCEEDED: s = "CUDD_MAX_MEM_EXCEEDED"; break;
    case CUDD_INVALID_ARG: s = "CUDD_INVALID_ARG"; break;
    case CUDD_INTERNAL_ERROR: s = "CUDD_INTERNAL_ERROR"; break;
    default: s = "CUDD_UNKNOWN"; break;
    }
    sprintf(camlidl_cudd_msg,
	    "Cudd: a function returned a null BDD node; ErrorCode = %s",
	    s);
    failwith(camlidl_cudd_msg);
  }

  cuddRef(bdd->node);
  managerRef(bdd->man);
  /*
  caml_gc_full_major(Val_unit);
  cuddGarbageCollect(bdd->man->man,1);
  assert(Cudd_CheckKeys(bdd->man->man)==0);
  assert(Cudd_DebugCheck(bdd->man->man)==0);
  */
  val = caml_alloc_custom(&camlidl_custom_bdd, sizeof(struct node__t), 1, camlidl_cudd_heap);
  ((node__t*)(Data_custom_val(val)))->man = bdd->man;
  ((node__t*)(Data_custom_val(val)))->node = bdd->node;
  return val;
}

/* %======================================================================== */
/* \section{Extractors} */
/* %======================================================================== */

value camlidl_cudd_bdd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal3(vres,vthen,velse);
  bdd__t no;
  DdNode* N;

  camlidl_cudd_node_ml2c(vno, &no);
  N = Cudd_Regular(no.node);
  if (cuddIsConstant(N)){
   vres = caml_alloc_small(1,0);
   if (no.node == DD_ONE(no.man->man))
     Field(vres,0) = Val_true;
   else
     Field(vres,0) = Val_false;
  }
  else {
    bdd__t bthen,belse;

    bthen.man = belse.man = no.man;
    bthen.node = cuddT(N);
    belse.node = cuddE(N);
    if (Cudd_IsComplement(no.node)) {
      bthen.node = Cudd_Not(bthen.node);
      belse.node = Cudd_Not(belse.node);
    }
    vthen = camlidl_cudd_bdd_c2ml(&bthen);
    velse = camlidl_cudd_bdd_c2ml(&belse);
    vres = caml_alloc_small(3,1);
    Field(vres,0) = Val_int(N->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_cudd_bdd_cofactors(value v_var, value v_no)
{
  CAMLparam2(v_var,v_no); CAMLlocal3(vthen,velse,vres);
  int var;
  bdd__t no;
  bdd__t nothen,noelse;

  var = Int_val(v_var);
  camlidl_cudd_node_ml2c(v_no, &no);
  
  nothen.man = noelse.man = no.man;
  nothen.node = Cudd_Cofactor(no.man->man,no.node,no.man->man->vars[var]);
  if (nothen.node==NULL){
    vres = camlidl_cudd_bdd_c2ml(&nothen);
    CAMLreturn(vres);
  }
  cuddRef(nothen.node);
  noelse.node = Cudd_Cofactor(no.man->man,no.node,Cudd_Not(no.man->man->vars[var]));
  if (noelse.node==NULL){
    Cudd_IterDerefBdd(no.man->man,nothen.node);
    vres = camlidl_cudd_bdd_c2ml(&noelse);
    CAMLreturn(vres);
  }
  velse = camlidl_cudd_bdd_c2ml(&noelse);
  cuddDeref(nothen.node);
  vthen = camlidl_cudd_bdd_c2ml(&nothen);
  vres = caml_alloc_small(2,0);
  Field(vres,0) = vthen;
  Field(vres,1) = velse;
  CAMLreturn(vres);
}

value camlidl_cudd_add_cofactors(value v_var, value v_no)
{
  CAMLparam2(v_var,v_no); CAMLlocal3(vthen,velse,vres);
  int var;
  add__t no;
  add__t nothen,noelse;

  var = Int_val(v_var);
  camlidl_cudd_node_ml2c(v_no, &no);
  
  nothen.man = noelse.man = no.man;
  nothen.node = Cudd_Cofactor(no.man->man,no.node,no.man->man->vars[var]);
  if (nothen.node==NULL){
    vres = camlidl_cudd_node_c2ml(&nothen);
    CAMLreturn(vres);
  }
  cuddRef(nothen.node);
  noelse.node = Cudd_Cofactor(no.man->man,no.node,Cudd_Not(no.man->man->vars[var]));
  if (noelse.node==NULL){
    Cudd_RecursiveDeref(no.man->man,nothen.node);
    vres = camlidl_cudd_node_c2ml(&noelse);
    CAMLreturn(vres);
  }
  velse = camlidl_cudd_node_c2ml(&noelse);
  cuddDeref(nothen.node);
  vthen = camlidl_cudd_node_c2ml(&nothen);
  vres = caml_alloc_small(2,0);
  Field(vres,0) = vthen;
  Field(vres,1) = velse;
  CAMLreturn(vres);
}

value Val_type(int ddtype, cuddauxType* type)
{
  value val;
  switch(ddtype){
  case 0:
    val = copy_double(type->dbl);
    break;
  case 1:
    {
      int i = (int)(type->dbl);
      val = Val_int(i);
    }
    break;
  case 2:
    val = type->value;
    break;
  default: abort();
  }
  return val;
}

value Val_DdNode(int ddtype, DdNode* node)
{
  cuddauxType type;
  value val;
  type = ((cuddauxDdNode*)node)->type;
  val = Val_type(ddtype,&type);
  return val;
}

cuddauxType Type_val(int ddtype, value val)
{
  cuddauxType type;
  switch(ddtype){
  case 0:
    type.dbl = Double_val(val);
    break;
  case 1:
    type.dbl = (double)(Int_val(val));
    break;
  case 2:
    type.value = val;
    break;
  default: abort();
  }
  return type;
}

inline cuddauxType cuddauxType_DdNode(DdNode* node)
{
  cuddauxType type;
  type = ((cuddauxDdNode*)node)->type;
  return type;
}

inline int cuddauxTypeEqual(int ddtype, cuddauxType* type1, cuddauxType* type2)
{
  return ddtype==2 ? type1->value==type2->value : type1->dbl == type2->dbl;
}

value camlidl_cudd_avdd_dval(value vddtype, value vno)
{
  CAMLparam2(vddtype,vno); CAMLlocal1(vres);
  add__t no;
  int ddtype;

  ddtype = Int_val(vddtype);
  camlidl_cudd_node_ml2c(vno, &no);
  if (!cuddIsConstant(no.node))
    failwith("Add|Vdd.dval: non constant DD");
  vres = Val_DdNode(ddtype,no.node);
  CAMLreturn(vres);
}

value camlidl_cudd_avdd_cst(value vddtype, value vman, value vleaf)
{
  CAMLparam3(vddtype,vman,vleaf); CAMLlocal1(vres);
  int ddtype;
  man__t man;
  node__t _res;
  cuddauxType type;

  ddtype = Int_val(vddtype);
  camlidl_cudd_man_ml2c(vman,&man);
  type = Type_val(ddtype,vleaf);
  _res.man = man;
  _res.node = cuddauxUniqueType(ddtype==2,man->man,&type);
  vres = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(vres);
}

value camlidl_cudd_avdd_inspect(value vddtype, value vno)
{
  CAMLparam2(vddtype,vno); CAMLlocal4(vres,vthen,velse,val);
  add__t no;
  int ddtype;

  ddtype = Int_val(vddtype);
  camlidl_cudd_node_ml2c(vno, &no);
  if (cuddIsConstant(no.node)){
    val = Val_DdNode(ddtype,no.node);
    vres = caml_alloc_small(1,0);
    Field(vres,0) = val;
  }
  else {
    add__t bthen,belse;

    bthen.man = belse.man = no.man;
    bthen.node = cuddT(no.node);
    belse.node = cuddE(no.node);
    vthen = camlidl_cudd_node_c2ml(&bthen);
    velse = camlidl_cudd_node_c2ml(&belse);
    vres = caml_alloc_small(3,1);
    Field(vres,0) = Val_int(no.node->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_cudd_avdd_is_eval_cst(value vddtype, value vno1, value vno2)
{
  CAMLparam3(vddtype,vno1,vno2); CAMLlocal2(v,vres);
  int ddtype;
  node__t no1;
  node__t no2;
  DdNode* node;

  ddtype = Int_val(vddtype);
  camlidl_cudd_node_ml2c(vno1, &no1);
  camlidl_cudd_node_ml2c(vno2, &no2);
  if (no1.man!=no2.man){
    failwith("Dd: binary function called with nodes belonging to different managers !");
  }
  node = Cuddaux_addEvalConst(no1.man->man,no2.node,no1.node);
  if (node==DD_NON_CONSTANT || ! cuddIsConstant(node))
    vres = Atom(0);
  else {
    v = Val_DdNode(ddtype,node);
    vres = caml_alloc_small(1,0);
    Field(vres,0) = v;
  }
  CAMLreturn(vres);
}

value camlidl_cudd_avdd_is_ite_cst(value vddtype, value vno1, value vno2, value vno3)
{
  CAMLparam4(vddtype,vno1,vno2,vno3); CAMLlocal2(v,vres);
  int ddtype;
  node__t no1;
  node__t no2;
  node__t no3;
  DdNode* node;

  ddtype = Int_val(vddtype);
  camlidl_cudd_node_ml2c(vno1, &no1);
  camlidl_cudd_node_ml2c(vno2, &no2);
  camlidl_cudd_node_ml2c(vno3, &no3);
  if (no1.man!=no2.man || no1.man!=no3.man){				
    failwith("Dd: ternary function called with nodes belonging to different managers !");
  }
  node = Cuddaux_addIteConstant(no1.man->man,no1.node,no2.node,no3.node);
  if (node==DD_NON_CONSTANT || ! cuddIsConstant(node))
    vres = Atom(0);
  else {
    v = Val_DdNode(ddtype,node);
    vres = caml_alloc_small(1,0);
    Field(vres,0) = v;
  }
  CAMLreturn(vres);
}

/* %======================================================================== */
/* \section{Supports} */
/* %======================================================================== */

value camlidl_cudd_list_of_support(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,r);
  bdd__t node;
  DdNode * N;

  camlidl_cudd_node_ml2c(_v_no,&node);
  N = node.node;

  res = Val_int(0);
  while (! Cudd_IsConstant(N)){
    if (Cudd_IsComplement(N)){
      failwith("Bdd.int_of_support not called on a positive cube\n");
    }
    r = caml_alloc_small(2,0);
    Field(r,0) = Val_int(N->index);
    Field(r,1) = res;
    res = r;
    if (! Cudd_IsConstant(cuddE(N))){
      failwith("Bdd.list_of_support not called on a positive cube\n");
    }
    N = cuddT(N);
  }
  CAMLreturn(res);
}

value camlidl_cudd_bdd_vectorsupport(value _v_vec)
{
  CAMLparam1(_v_vec); CAMLlocal2(_v_no,_v_res);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t _no,_res;
  int i;

  size = Wosize_val(_v_vec);
  if (size==0)
    failwith ("Bdd.vectorsupport called with an empty array (annoying because unknown manager for true)");
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  _v_no = Field(_v_vec,0);
  camlidl_cudd_node_ml2c(_v_no, &_no);
  _res.man = _no.man;

  for (i = 0; i<size; i++) {
    _v_no = Field(_v_vec, i);
    camlidl_cudd_node_ml2c(_v_no, &_no);
    vec[i] = _no.node;
    if (_no.man != _res.man)
      failwith("Bdd.vectorsupport called with BDDs belonging to different managers !");
  }
  _res.node = Cudd_VectorSupport(_res.man->man, vec, size);
  free(vec);
  _v_res = camlidl_cudd_bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}

value camlidl_cudd_add_vectorsupport2(value _v_vec1, value _v_vec2)
{
  CAMLparam2(_v_vec1,_v_vec2); CAMLlocal2(_v_no,_v_res);
  DdNode **vec; /*in*/
  int size1,size2,size; /*in*/
  bdd__t _no,_res;
  int i,index;

  size1 = Wosize_val(_v_vec1);
  size2 = Wosize_val(_v_vec2);
  size = size1+size2;
  if (size==0)
    failwith ("Add.vectorsupport2 called with two empty arrays (annoying because unknown manager for true)");
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  if (size1>0)
    _v_no = Field(_v_vec1,0);
  else
    _v_no = Field(_v_vec2,0);
  camlidl_cudd_node_ml2c(_v_no, &_no);
  _res.man = _no.man;

  index = 0;
  for (i = 0; i<size1; i++) {
    _v_no = Field(_v_vec1, i);
    camlidl_cudd_node_ml2c(_v_no, &_no);
    vec[index++] = _no.node;
    if (_no.man != _res.man)
      failwith("Add.vectorsupport2 called with BDDs belonging to different managers !");
  }
  for (i = 0; i<size2; i++) {
    _v_no = Field(_v_vec2, i);
    camlidl_cudd_node_ml2c(_v_no, &_no);
    vec[index++] = _no.node;
    if (_no.man != _res.man)
      failwith("Add.vectorsupport2 called with BDDs belonging to different managers !");
  }
  _res.node = Cudd_VectorSupport(_res.man->man, vec, size);
  free(vec);
  _v_res = camlidl_cudd_bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}


/* %======================================================================== */
/* \section{Logical operations} */
/* %======================================================================== */

value camlidl_cudd_bdd_vectorcompose(value _v_vec, value _v_no)
{
  CAMLparam2(_v_vec,_v_no); CAMLlocal2(_v,_vres);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t no; /*in*/
  bdd__t _res;
  int i;

  camlidl_cudd_node_ml2c(_v_no, &no);
  size = Wosize_val(_v_vec);
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  for (i = 0; i<size; i++) {
    bdd__t _no;
    _v = Field(_v_vec, i);
    camlidl_cudd_node_ml2c(_v, &_no);
    if (_no.man != no.man)
      failwith("Bdd.vectorcompose called with BDDs belonging to different managers !");
    vec[i] = _no.node;
  }
  _res.man = no.man;
  _res.node = Cudd_bddVectorCompose(no.man->man, no.node, vec);
  _vres = camlidl_cudd_bdd_c2ml(&_res);
  free(vec);
  CAMLreturn(_vres);
}

value camlidl_cudd_add_vectorcompose(value _v_vec, value _v_no)
{
  CAMLparam2(_v_vec,_v_no); CAMLlocal2(_v,_vres);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t no; /*in*/
  bdd__t _res;
  int i;

  camlidl_cudd_node_ml2c(_v_no, &no);
  size = Wosize_val(_v_vec);
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  for (i = 0; i<size; i++) {
    bdd__t _no;
    _v = Field(_v_vec, i);
    camlidl_cudd_node_ml2c(_v, &_no);
    if (_no.man != no.man)
      failwith("Bdd.vectorcompose called with BDDs belonging to different managers !");
    vec[i] = _no.node;
  }
  _res.man = no.man;
  _res.node = Cuddaux_addVectorCompose(no.man->man, no.node, vec);
  _vres = camlidl_cudd_node_c2ml(&_res);
  free(vec);
  CAMLreturn(_vres);
}

/* %======================================================================== */
/* \section{Variable Mapping} */
/* %======================================================================== */

value camlidl_cudd_bdd_permute(value _v_no, value _v_permut)
{
  CAMLparam2(_v_no,_v_permut); CAMLlocal1(_vres);
  bdd__t no; /*in*/
  int *permut; /*in*/
  bdd__t _res;
  mlsize_t i,size;

  camlidl_cudd_node_ml2c(_v_no, &no);
  size = Wosize_val(_v_permut);
  permut = malloc(size * sizeof(int));
  for (i=0; i < size; i++) {
    value v = Field(_v_permut, i);
    permut[i] = Int_val(v);
  }
  _res.man = no.man;
  _res.node = Cudd_bddPermute(no.man->man,no.node,permut);
  _vres = camlidl_cudd_bdd_c2ml(&_res);
  free(permut);
  CAMLreturn(_vres);
}

value camlidl_cudd_add_permute(value _v_no, value _v_permut)
{
  CAMLparam2(_v_no,_v_permut); CAMLlocal1(_vres);
  bdd__t no; /*in*/
  int *permut; /*in*/
  bdd__t _res;
  mlsize_t i,size;

  camlidl_cudd_node_ml2c(_v_no, &no);
  size = Wosize_val(_v_permut);
  permut = malloc(size * sizeof(int));
  for (i=0; i < size; i++) {
    value v = Field(_v_permut, i);
    permut[i] = Int_val(v);
  }
  _res.man = no.man;
  _res.node = Cudd_addPermute(no.man->man,no.node,permut);
  _vres = camlidl_cudd_node_c2ml(&_res);
  free(permut);
  CAMLreturn(_vres);
}


/* %======================================================================== */
/* \section{Iterators} */
/* %======================================================================== */

value camlidl_cudd_iter_node(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal1(_v_snode);
  DdGen* gen;
  bdd__t no;
  bdd__t snode;
  int autodyn;
  Cudd_ReorderingType heuristic;

  camlidl_cudd_node_ml2c(_v_no,&no);
  autodyn = 0;
  if (Cudd_ReorderingStatus(no.man->man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man->man);
  }
  snode.man = no.man;
  Cudd_ForeachNode(no.man->man,no.node,gen,snode.node)
    {
      _v_snode = camlidl_cudd_node_c2ml(&snode);
      caml_callback(_v_closure,_v_snode);
    }
  if (autodyn) Cudd_AutodynEnable(no.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_cudd_bdd_iter_cube(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal1(_v_array);
  bdd__t no;
  DdGen* gen;
  int* array;
  double val;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  camlidl_cudd_node_ml2c(_v_no,&no);
  autodyn = 0;
  if (Cudd_ReorderingStatus(no.man->man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man->man);
  }
  size = no.man->man->size;
  Cudd_ForeachCube(no.man->man,no.node,gen,array,val)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = caml_alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	  /* Allowed according to caml/memory.h memory.c */
	}
      }
      caml_callback(_v_closure,_v_array);
    }
  if (autodyn) Cudd_AutodynEnable(no.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_cudd_avdd_iter_cube(value _v_ddtype,
				   value _v_closure, value _v_no)
{
  CAMLparam3(_v_ddtype,_v_closure,_v_no); CAMLlocal2(_v_array,_v_val);
  int ddtype;
  bdd__t no;
  DdGen* gen;
  int* array;
  double val;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  ddtype = Int_val(_v_ddtype);
  camlidl_cudd_node_ml2c(_v_no,&no);
  autodyn = 0;
  if (Cudd_ReorderingStatus(no.man->man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man->man);
  }
  size = no.man->man->size;
  Cudd_ForeachCube(no.man->man,no.node,gen,array,val)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = caml_alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	/* Allowed according to caml/memory.h memory.c */
	}
      }
      switch (ddtype){
      case 0:
	_v_val = copy_double(val);
	break;
      case 1:
	{
	  int ival = (int)val;
	  _v_val = Val_int(ival);
	}
	break;
      case 2:
	{
	  cuddauxType type;
	  type.dbl = val;
	  _v_val = type.value;
	}
	break;
      default: abort();
      }
      caml_callback2(_v_closure,_v_array,_v_val);
    }
  if (autodyn) Cudd_AutodynEnable(no.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_cudd_bdd_iter_prime(value _v_closure, value _v_lower, value _v_upper)
{
  CAMLparam3(_v_closure,_v_lower,_v_upper); CAMLlocal1(_v_array);
  bdd__t lower,upper;
  DdGen* gen;
  int* array;
  double val;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  camlidl_cudd_node_ml2c(_v_lower,&lower);
  camlidl_cudd_node_ml2c(_v_upper,&upper);
  if (lower.man!=upper.man){
    failwith("Bdd.iter_prime called with BDDs belonging to different managers !");
  }
  autodyn = 0;
  if (Cudd_ReorderingStatus(lower.man->man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(lower.man->man);
  }
  size = lower.man->man->size;
  Cudd_ForeachPrime(lower.man->man,lower.node,upper.node,gen,array)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = caml_alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	  /* Allowed according to caml/memory.h memory.c */
	}
      }
      caml_callback(_v_closure,_v_array);
    }
  if (autodyn) Cudd_AutodynEnable(lower.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

/* %======================================================================== */
/* \section{Cubes} */
/* %======================================================================== */

value camlidl_cudd_cube_of_bdd(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal1(_v_res);
  node__t no;
  bdd__t _res;
  camlidl_cudd_node_ml2c(_v_no,&no);
  _res.man = no.man;
  _res.node = Cudd_FindEssential(no.man->man,no.node);
  _v_res = camlidl_cudd_bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}

value camlidl_cudd_cube_of_minterm(value _v_man, value _v_array)
{
  CAMLparam2(_v_man,_v_array); CAMLlocal1(_v_res);
  man__t man;
  int i,size,maxsize;
  int* array;
  bdd__t _res;

  camlidl_cudd_man_ml2c(_v_man, &man);
  size = Wosize_val(_v_array);
  maxsize = Cudd_ReadSize(man->man);
  if (size>maxsize){
    caml_failwith("Bdd.cube_of_minterm: array of size greater than the number of variables in manager");
  }
  array = malloc(maxsize*sizeof(int));
  for (i=0; i<size; i++){
    array[i] = Int_val(Field(_v_array,i));
  }
  for (i=size; i<maxsize; i++){
    array[i] = 2;
  }
  _res.man = man;
  _res.node = Cudd_CubeArrayToBdd(man->man,array);
  free(array);
  _v_res = camlidl_cudd_bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}

value camlidl_cudd_list_of_cube(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal3(res,r,elt);
  bdd__t node;
  DdNode *zero, *f, *fv, *fnv;
  int index,sign;

  camlidl_cudd_node_ml2c(_v_no,&node);
  f = node.node;

  zero = Cudd_Not(DD_ONE(node.man->man));
  res = Val_int(0);
  if (f==zero)
    failwith("Bdd.list_of_cube called on a false bdd\n");
  else {
    while (! Cudd_IsConstant(f)){
      index = Cudd_Regular(f)->index;
      fv = Cudd_T(f);
      fnv = Cudd_E(f);
      if (Cudd_IsComplement(f)){
	fv = Cudd_Not(fv);
	fnv = Cudd_Not(fnv);
      }
      if (fv==zero){
	sign = 0;
	f = fnv;
      }
      else if (fnv==zero){
	sign = 1;
	f = fv;
      }
      else
	failwith("Bdd.list_of_cube not called on a cube\n");

      elt = caml_alloc_small(2,0);
      Field(elt,0) = Val_int(index);
      Field(elt,1) = Val_bool(sign);
      r = caml_alloc_small(2,0);
      Field(r,0) = elt;
      Field(r,1) = res;
      res = r;
    }
  }
  CAMLreturn(res);
}

value camlidl_cudd_pick_minterm(value _v_no)
{
  CAMLparam1(_v_no);
  CAMLlocal1(_v_array);
  bdd__t no;
  char array[1024];
  char* string;
  int i,size,res;

  camlidl_cudd_node_ml2c(_v_no,&no);
  size = no.man->man->size;
  string = size>1024 ? (char*)malloc(size) : array;
  if (string==NULL){
    failwith("Bdd.pick_minterm: out of memory");
  }
  res = Cudd_bddPickOneCube(no.man->man,no.node,string);
  if (res==0){
    if (size>1024) free(string);
    failwith("Bdd.pick_minterm: (probably) second argument is not a positive cube");
  }
  _v_array = caml_alloc(size,0);
  for(i=0; i<size; i++){
    Store_field(_v_array,i,Val_int(array[i]));
    /* Allowed according to caml/memory.h memory.c */
  }
  if (size>1024) free(string);
  CAMLreturn(_v_array);
}

int array_of_support(DdManager* man, DdNode* supp, DdNode*** pvars, int* psize)
{
  int i,size;
  DdNode* zero;
  DdNode* one;;
  DdNode* f;
  DdNode** vars;

  f = supp;
  one = DD_ONE(man);
  zero = Cudd_Not(one);
  size = 0;
  while (! Cudd_IsConstant(f)){
    if (Cudd_IsComplement(f) || cuddE(f)!=zero){
      return 1;
    }
    f = cuddT(f);
    size++;
  }
  if (size==0) return 2;
  vars = (DdNode**)malloc(size*sizeof(DdNode*));
  f = supp;
  for (i=0; i<size; i++){
    vars[i] = Cudd_ReadVars(man,f->index);
    f = cuddT(f);
  }
  *pvars = vars;
  *psize=size;
  return 0;
}


value camlidl_cudd_pick_cube_on_support(value _v_no1, value _v_no2)
{
  CAMLparam2(_v_no1,_v_no2);
  CAMLlocal1(_v_res);
  bdd__t no1,no2;
  int size,ret;
  DdNode** vars;
  bdd__t res;

  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  if (no1.man!=no2.man){
    failwith ("Bdd.pick_cube_on_support called with BDDs belonging to different managers !");
  }
  ret = array_of_support(no2.man->man,no2.node,&vars,&size);
  if (ret==1){
    failwith("Bdd.pick_cube_on_support: the second argument is not a positive cube");
  }
  else if (ret==2){
    failwith("Bdd.pick_cube_on_support: empty support or out of memory");
  }
  res.man = no1.man;
  res.node = Cudd_bddPickOneMinterm(no1.man->man,no1.node,vars,size);
  free(vars);
  _v_res = camlidl_cudd_bdd_c2ml(&res);
  CAMLreturn(_v_res);
}

value camlidl_cudd_pick_cubes_on_support(value _v_no1, value _v_no2, value _v_k)
{
  CAMLparam3(_v_no1,_v_no2,_v_k);
  CAMLlocal2(v,_v_res);
  bdd__t no1,no2,no3;
  int i,k;
  int size,ret;
  DdNode** vars;
  DdNode** array;

  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  k = Int_val(_v_k);
  if (no1.man!=no2.man){
    failwith ("Bdd.pick_cubes_on_support called with BDDs belonging to different managers !");
  }
  ret = array_of_support(no2.man->man,no2.node,&vars,&size);
  if (ret==1){
    failwith("Bdd.pick_cubes_on_support: the second argument is not a positive cube");
  }
  else if (ret==2){
    failwith("Bdd.pick_cube_on_support: empty support or out of memory");
  }
  array = Cudd_bddPickArbitraryMinterms(no1.man->man,no1.node,vars,size,k);
  free(vars);
  if (array==NULL){
    failwith("Bdd.pick_cubes_on_support: out of memory, or first argument is false, or wrong support, or number of minterms < k");
  }

  if (k==0){
    _v_res = Atom(0);
  }
  else {
    _v_res = caml_alloc(k,0);
    for(i=0; i<k; i++){
      no3.man = no1.man;
      no3.node = array[i];
      v = camlidl_cudd_bdd_c2ml(&no3);
      Store_field(_v_res,i,v);
    }
  }
  CAMLreturn(_v_res);
}


/* %======================================================================== */
/* \section{Guards and leaves} */
/* %======================================================================== */

/* Guard of a given leaf */
value camlidl_cudd_avdd_guard_of_leaf(value _v_ddtype, value _v_no, value _v_leaf)
{
  CAMLparam3(_v_ddtype,_v_no,_v_leaf);
  CAMLlocal1(_vres);
  node__t _res;
  int ddtype;
  node__t no;
  cuddauxType type;
  DdNode* node;

  ddtype = Int_val(_v_ddtype);
  camlidl_cudd_node_ml2c(_v_no,&no);
  type = Type_val(ddtype,_v_leaf);
  node = cuddauxUniqueType(ddtype==2,no.man->man,&type);
  cuddRef(node);
  _res.man = no.man;
  _res.node = Cuddaux_addGuardOfNode(no.man->man,no.node,node);
  cuddDeref(node);
  _vres = camlidl_cudd_bdd_c2ml(&_res);
  CAMLreturn(_vres);
}

/* List of nodes below a optional level */
value camlidl_cudd_avdd_nodes_below_level(value _v_ddtype, value _v_no, value _v_olevel, value _v_omax)
{
  CAMLparam4(_v_ddtype,_v_no,_v_olevel,_v_omax);
  CAMLlocal2(res,v);
  node__t no;
  int ddtype,i,level;
  size_t max,size;
  list_t *p, *list;

  ddtype = Int_val(_v_ddtype);
  camlidl_cudd_node_ml2c(_v_no,&no);
  if (Is_long(_v_olevel))
    level = CUDD_MAXINDEX;
  else {
    value _v_level = Field(_v_olevel,0);
    level = Int_val(_v_level);
  }
  if (Is_long(_v_omax))
    max = 0;
  else {
    value _v_max = Field(_v_omax,0);
    max = Int_val(_v_max);
  }
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,level,max,&size,ddtype<2);

  /* Create and fill the array */
  if (size==0){
    res = Atom(0);
  }
  else {
    res = caml_alloc(size,0);
    for(p=list, i=0; p!=NULL; p=p->next,i++){
      assert(p->node->ref>=1);
      no.node = p->node;
      v = camlidl_cudd_node_c2ml(&no);
      Store_field(res,i,v);
    }
  }
  list_free(list);
  CAMLreturn(res);
}

/* List of leaves of an add or vdd. */
value camlidl_cudd_avdd_leaves(value _v_ddtype, value _v_no)
{
  CAMLparam2(_v_ddtype,_v_no); CAMLlocal1(res);
  int ddtype;
  node__t no;
  list_t *p, *list;
  int i;
  size_t size;

  ddtype = Int_val(_v_ddtype);
  camlidl_cudd_node_ml2c(_v_no,&no);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,0,&size,ddtype<2);

  /* Create and fill the array */
  if (size==0){
    res = Atom(0);
  }
  else {
    res =
      (ddtype==0) ?
      caml_alloc(size * Double_wosize,Double_array_tag) :
      caml_alloc(size,0)
      ;
    for(p=list,i=0; p!=NULL; p=p->next,i++){
      switch (ddtype){
      case 0:
	Store_double_field(res,i,cuddV(p->node));
	break;
      case 1:
	{
	  int val = (int)(cuddV(p->node));
	  value v = Val_int(val);
	  Store_field(res,i,v);
	}
	break;
      case 2:
	Store_field(res,i,cuddauxCamlV(p->node));
	break;
      default: abort();
      }
    }
  }
  list_free(list);
  CAMLreturn(res);
}

/* Pick a leaf in an add or vdd. */
value camlidl_cudd_avdd_pick_leaf(value _v_ddtype, value _v_no)
{
  CAMLparam2(_v_ddtype,_v_no); CAMLlocal1(res);
  int ddtype;
  node__t no;
  list_t *list;
  size_t size;

  ddtype = Int_val(_v_ddtype);
  camlidl_cudd_node_ml2c(_v_no,&no);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,1,&size,ddtype<2);
  if (list==NULL){
    caml_failwith("A Mtbdd should never contain the CUDD background node !");
  }
  else {
    res = Val_DdNode(ddtype,list->node);
   }
  list_free(list);
  CAMLreturn(res);
}

value camlidl_cudd_print(value _v_no)
{
  CAMLparam1(_v_no);
  node__t no;

  camlidl_cudd_node_ml2c(_v_no,&no);
  fflush(stdout);
  Cudd_PrintMinterm(no.man->man,no.node);
  fflush(stdout);
  CAMLreturn(Val_unit);
}
