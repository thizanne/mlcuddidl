/* $Id: cudd_caml.c,v 1.3 2005/06/14 14:41:05 bjeannet Exp $ */

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
  int heap, res;

  heap = Int_val(_v_heap);
  camlidl_cudd_heap = heap;
  if (camlidl_cudd_gc_fun == Val_unit)
    caml_register_global_root(&camlidl_cudd_gc_fun);
  if (camlidl_cudd_reordering_fun == Val_unit)
    caml_register_global_root(&camlidl_cudd_reordering_fun);
  camlidl_cudd_gc_fun = _v_gc;
  camlidl_cudd_reordering_fun = _v_reordering;
  CAMLreturn(Val_unit);
}

int camlidl_cudd_garbage(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_gc_fun==Val_unit){
    value* p = caml_named_value("gc_full_major");
    if (p == 0){
      fprintf(stderr,"mlcuddidl: cudd_caml.o: internal error: the \"let _ = Callback.register ...\" line in manager.ml has not been executed\n");
      abort();
    }
    camlidl_cudd_gc_fun = *p;
    caml_register_global_root(&camlidl_cudd_gc_fun);
  }
  callback(camlidl_cudd_gc_fun,Val_unit);
  return 1;
}

int camlidl_cudd_reordering(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_reordering_fun==Val_unit){
     value* p = caml_named_value("gc_full_major");
     if (p == NULL){
      fprintf(stderr,"mlcuddidl: cudd_caml.o: internal error: the \"let _ = Callback.register ...\" line in manager.ml has not been executed\n");
      abort();
     }
     camlidl_cudd_reordering_fun = *p;
     caml_register_global_root(&camlidl_cudd_reordering_fun);
  }
  callback(camlidl_cudd_reordering_fun,Val_unit);
  return 1;
}

/* %======================================================================== */
/* \section{Custom datatypes} */
/* %======================================================================== */

/* %------------------------------------------------------------------------ */
/* \subsection{Custom functions} */
/* %------------------------------------------------------------------------ */


/* \subsubsection{Managers} */

void manager_free(struct manager__t* man)
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

void camlidl_custom_manager_finalize(value val)
{
  struct manager__t* man = manager_of_vmanager(val);
  manager_free(man);
}
int camlidl_custom_manager_compare(value val1, value val2)
{
  CAMLparam2(val1,val2);
  int res;
  DdManager* man1 = DdManager_of_vmanager(val1);
  DdManager* man2 = DdManager_of_vmanager(val2);
  res = (long)man1==(long)man2 ? 0 : (long)man1<(long)man2 ? -1 : 1;
  CAMLreturn(res);
}
long camlidl_custom_manager_hash(value val)
{
  CAMLparam1(val);
  DdManager* man = DdManager_of_vmanager(val);
  long hash = (long)man;
  CAMLreturn(hash);
}

struct custom_operations camlidl_custom_manager = {
  "camlidl_cudd_custom_node",
  &camlidl_custom_manager_finalize,
  &camlidl_custom_manager_compare,
  &camlidl_custom_manager_hash,
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
  manager_free(no->man);
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
  CAMLreturn(res);
}
long camlidl_custom_node_hash(value val)
{
  CAMLparam1(val);
  DdNode* node = DdNode_of_vnode(val);
  long hash = (long)node;
  CAMLreturn(hash);
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
  manager_free(no->man);
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
value camlidl_cudd_manager_c2ml(manager__t* man)
{
  value val;
  if((*man)->man==NULL)
    failwith("Cudd: a function returned a null manager");
  val = alloc_custom(&camlidl_custom_manager, sizeof(manager__t*), 0, 1);
  *((manager__t*)(Data_custom_val(val))) = *man;
  managerRef(*man);
  return val;
}
void camlidl_cudd_manager_ml2c(value val, manager__t* man)
{
  *man = *((manager__t*)(Data_custom_val(val)));
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

  val = alloc_custom(&camlidl_custom_node, sizeof(struct node__t), 1, camlidl_cudd_heap);
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
#ifndef NDEBUG
  bdd_compteur++;
  if (bdd_compteur > START_bdd && bdd_compteur % FREQ_bdd == 0){
    int res1,res2;
    fprintf(stderr,"bdd_check(%d,%d)...",node_compteur,bdd_compteur);
    gc_full_major(Val_unit);
    res1 = Cudd_ReduceHeap(bdd->man,CUDD_REORDER_NONE,0);
    res2 = Cudd_DebugCheck(bdd->man);
    if (!res1 || res2){
      fprintf(stderr,"bdd\nnode_compteur=%d, bdd_compteur=%d\n",
	      node_compteur,bdd_compteur);
      abort();
    }
    fprintf(stderr,"done\n");
  }
#endif
  */

  val = alloc_custom(&camlidl_custom_bdd, sizeof(struct node__t), 1, camlidl_cudd_heap);
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
   vres = alloc_small(1,0);
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
    vres = alloc_small(3,1);
    Field(vres,0) = Val_int(N->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_cudd_rdd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal4(vres,vthen,velse,vr);
  rdd__t no;

  camlidl_cudd_node_ml2c(vno, &no);
  if (cuddIsConstant(no.node)){
    vr = copy_double(cuddV(no.node));
    vres = alloc_small(1,0);
    Field(vres,0) = vr;
  }
  else {
    rdd__t bthen,belse;

    bthen.man = belse.man = no.man;
    bthen.node = cuddT(no.node);
    belse.node = cuddE(no.node);
    vthen = camlidl_cudd_node_c2ml(&bthen);
    velse = camlidl_cudd_node_c2ml(&belse);
    vres = alloc_small(3,1);
    Field(vres,0) = Val_int(no.node->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_cudd_idd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal4(vres,vthen,velse,vr);
  rdd__t no;

  camlidl_cudd_node_ml2c(vno, &no);
  if (cuddIsConstant(no.node)){
    int r = (int)(cuddV(no.node));
    vr = Val_int(r);
    vres = alloc_small(1,0);
    Field(vres,0) = vr;
  }
  else {
    idd__t bthen,belse;

    bthen.man = belse.man = no.man;
    bthen.node = cuddT(no.node);
    belse.node = cuddE(no.node);
    vthen = camlidl_cudd_node_c2ml(&bthen);
    velse = camlidl_cudd_node_c2ml(&belse);
    vres = alloc_small(3,1);
    Field(vres,0) = Val_int(no.node->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
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
    r = alloc_small(2,0);
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

value camlidl_cudd_rdd_vectorsupport2(value _v_vec1, value _v_vec2)
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
    failwith ("Rdd.vectorsupport2 called with two empty arrays (annoying because unknown manager for true)");
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
      failwith("Rdd.vectorsupport2 called with BDDs belonging to different managers !");
  }
  for (i = 0; i<size2; i++) {
    _v_no = Field(_v_vec2, i);
    camlidl_cudd_node_ml2c(_v_no, &_no);
    vec[index++] = _no.node;
    if (_no.man != _res.man)
      failwith("Rdd.vectorsupport2 called with BDDs belonging to different managers !");
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

value camlidl_cudd_rdd_vectorcompose(value _v_vec, value _v_no)
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

value camlidl_cudd_rdd_permute(value _v_no, value _v_permut)
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
      callback(_v_closure,_v_snode);
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
      callback(_v_closure,_v_array);
    }
  if (autodyn) Cudd_AutodynEnable(no.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_cudd_rdd_iter_cube(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal2(_v_array,_v_val);
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
      _v_val = copy_double(val);
      callback2(_v_closure,_v_array,_v_val);
    }
  if (autodyn) Cudd_AutodynEnable(no.man->man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_cudd_idd_iter_cube(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal2(_v_array,_v_val);
  bdd__t no;
  DdGen* gen;
  int* array;
  double val;
  int ival;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  camlidl_cudd_node_ml2c(_v_no,&no);
  if (Cudd_ReorderingStatus(no.man->man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man->man);
  }
  else
    autodyn=0;

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
      ival = (int)val;
      _v_val = Val_int(ival);
      callback2(_v_closure,_v_array,_v_val);
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
      callback(_v_closure,_v_array);
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
  manager__t man;
  int i,size,maxsize;
  int* array;
  bdd__t _res;

  camlidl_cudd_manager_ml2c(_v_man, &man);
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

      elt = alloc_small(2,0);
      Field(elt,0) = Val_int(index);
      Field(elt,1) = Val_bool(sign);
      r = alloc_small(2,0);
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

static int array_of_support(DdManager* man, DdNode* supp, DdNode*** pvars, int* psize)
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

/* List of nodes below a optional level */
value camlidl_cudd_rdd_nodes_below_level(value _v_no, value _v_olevel, value _v_max)
{
  CAMLparam3(_v_no,_v_olevel, _v_max);
  CAMLlocal2(res,v);
  node__t no;
  int i,level;
  size_t max,size;
  list_t *p, *list;

  camlidl_cudd_node_ml2c(_v_no,&no);
  if (Is_long(_v_olevel))
    level = CUDD_MAXINDEX;
  else {
    value _v_level = Field(_v_olevel,0);
    level = Int_val(_v_level);
  }
  max = Int_val(_v_max);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,level,max,&size);

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

/* List of leaves of an idd. */
value camlidl_cudd_idd_leaves(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,v);
  node__t no;
  list_t *p, *list;
  int val,i,level;
  size_t max,size;

  camlidl_cudd_node_ml2c(_v_no,&no);
  assert(no.node->ref>=1);

  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,0,&size);
  /* Create and fill the array */
  if (size==0){
    res = Atom(0);
  }
  else {
    res = caml_alloc(size,0);
    for(p=list,i=0; p!=NULL; p=p->next,i++){
      assert(p->node->ref>=1);
      val = (int)(cuddV(p->node));
      v = Val_int(val);
      Store_field(res,i,v);
    }
  }
  list_free(list);
  CAMLreturn(res);
}
/* List of leaves of an rdd. */
value camlidl_cudd_rdd_leaves(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal1(res);
  node__t no;
  list_t *p, *list;
  int i;
  size_t size;
  double val;

  camlidl_cudd_node_ml2c(_v_no,&no);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,0,&size);

  /* Create and fill the array */
  if (size==0){
    res = Atom(Double_array_tag);
  }
  else {
    res = caml_alloc(size * Double_wosize,Double_array_tag);
    for(p=list,i=0; p!=NULL; p=p->next,i++){
      assert(p->node->ref>=1);
      val = cuddV(p->node);
      Store_double_field(res,i,val);
    }
  }
  list_free(list);
  CAMLreturn(res);
}

/* Pick a leaf in an idd. */
value camlidl_cudd_idd_pick_leaf(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,v);
  node__t no;
  list_t *list;
  int val;
  size_t size;

  camlidl_cudd_node_ml2c(_v_no,&no);
  assert(no.node->ref>=1);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,1,&size);
  if (list==NULL){
    res = Atom(0);
  }
  else {
    val = (int)(cuddV(list->node));
    v = Val_int(val);
    res = alloc_small(1,0);
    Field(res,0) = v;
  }
  list_free(list);
  CAMLreturn(res);
}

value camlidl_cudd_rdd_pick_leaf(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,v);
  node__t no;
  list_t *list;
  double val;
  size_t size;

  camlidl_cudd_node_ml2c(_v_no,&no);
  assert(no.node->ref>=1);
  list = Cuddaux_NodesBelowLevel(no.man->man,no.node,CUDD_MAXINDEX,1,&size);
  if (list==NULL){
    res = Atom(0);
  }
  else {
    val = cuddV(list->node);
    v = copy_double(val);
    res = alloc_small(1,0);
    Field(res,0) = v;
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

/* %======================================================================== */
/* \section{User operations, using local cache} */
/* %======================================================================== */

static int camlidl_op_initialized = 0;
static int camlidl_rddidd_is_idd = 0;
  /* If false, we have RDD, if true we have IDD */
static int camlidl_rddidd_op_commutative = 0;
  /* Is the operation commutative ? */
static int camlidl_rddidd_op_idempotent = 0;
  /* Is the operation idempotent ? */
static int camlidl_rddidd_op_isabsorbant = 0;
static double camlidl_rddidd_op_absorbant_F = 0.0;
static double camlidl_rddidd_op_absorbant_G = 0.0;
static double camlidl_rddidd_op_absorbant_H = 0.0;
static double camlidl_rddidd_op_absorbant_I = 0.0;
  /* */
static int camlidl_rddidd_op_isneutral = 0;
static double camlidl_rddidd_op_neutral_F = 0.0;
static double camlidl_rddidd_op_neutral_G = 0.0;
  /* */
static int camlidl_rddidd_op_isbottom = 0;
static double camlidl_rddidd_op_bottom = 0.0;
  /* optional bottom */ 
static int camlidl_rddidd_op_istop = 0;
static double camlidl_rddidd_op_top = 0.0;
  /* optional top */ 
static value camlidl_rddidd_unop_closure = Val_unit;
static value camlidl_rddidd_op_closure = Val_unit;
/* Closure for mapunop, mapbinop operations */
static value camlidl_rddidd_op_exn = Val_unit;
/* To keep things temporarily */
static value camlidl_rddidd_op_val1 = Val_unit;
static value camlidl_rddidd_op_val2 = Val_unit;
static value camlidl_rddidd_op_val3 = Val_unit;
/* Exception */

static void camlidl_op_initialize(void)
{
  caml_register_global_root(&camlidl_rddidd_unop_closure);
  caml_register_global_root(&camlidl_rddidd_op_closure);
  caml_register_global_root(&camlidl_rddidd_op_exn);
  caml_register_global_root(&camlidl_rddidd_op_val1);
  caml_register_global_root(&camlidl_rddidd_op_val2);
  caml_register_global_root(&camlidl_rddidd_op_val3);
  camlidl_op_initialized = 1;
}

static DdNode* camlidl_rddidd_mapunop_aux(DdManager* man, DdNode* f)
{
  value _v_f,_v_val;
  DdNode *res;
  double val;
  int exception;

  assert (f->ref>=1);
  if (cuddIsConstant(f)){
    exception = 0;
    _v_f = _v_val = Val_unit;
    Begin_roots2(_v_f,_v_val)
      if (camlidl_rddidd_is_idd){
	_v_f = Val_int((int)(cuddV(f)));
	_v_val = callback_exn(camlidl_rddidd_unop_closure, _v_f);
	if (Is_exception_result(_v_val)){
	  camlidl_rddidd_op_exn = _v_val;
	  exception=1;
	}
	else
	  val = (double)(Int_val(_v_val));
      }
      else {
	_v_f = copy_double(cuddV(f));
	_v_val = callback_exn(camlidl_rddidd_unop_closure, _v_f);
	if (Is_exception_result(_v_val)){
	  camlidl_rddidd_op_exn = _v_val;
	  exception=1;
	}
	else
	  val = Double_val(_v_val);
      }
      res = exception ? NULL : cuddUniqueConst(man,val);
    End_roots()
  }
  else {
    res = NULL;
  }
  return res;
}
static DdNode* camlidl_rddidd_mapbinop_aux(DdManager* man, DdNode** f, DdNode** g)
{
  value _v_F,_v_G,_v_val;
  DdNode *F, *G, *res;
  double val;
  int exception;

  F = *f; G = *g;
  if (camlidl_rddidd_op_commutative && F > G) {
    *f = G;
    *g = F;
  }
  if (camlidl_rddidd_op_idempotent && F==G) {
    return F;
  }
  if (camlidl_rddidd_op_isabsorbant || camlidl_rddidd_op_isneutral){
    if (cuddIsConstant(F)){
      double v = cuddV(F);
      if (camlidl_rddidd_op_isabsorbant && v==camlidl_rddidd_op_absorbant_F){
	return Cudd_addConst(man,camlidl_rddidd_op_absorbant_H);
      }
      if (camlidl_rddidd_op_isneutral && v==camlidl_rddidd_op_neutral_F)
	return G;
    }
    if (cuddIsConstant(G)){
      double v = cuddV(G);
      if (camlidl_rddidd_op_isabsorbant && v==camlidl_rddidd_op_absorbant_G)
	return Cudd_addConst(man,camlidl_rddidd_op_absorbant_I);
      if (camlidl_rddidd_op_isneutral && v==camlidl_rddidd_op_neutral_G)
	return F;
    }
  }
  if (cuddIsConstant(F) && cuddIsConstant(G)) {
    exception = 0;
    _v_F = _v_G = _v_val = Val_unit;
    Begin_roots3(_v_F,_v_G,_v_val)
    if (camlidl_rddidd_is_idd){
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
      _v_val = callback2_exn(camlidl_rddidd_op_closure, _v_F, _v_G);
      if (Is_exception_result(_v_val)){
	camlidl_rddidd_op_exn = _v_val;
	exception=1;
      }
      else
	val = (double)(Int_val(_v_val));
    }
    else {
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
      _v_val = callback2_exn(camlidl_rddidd_op_closure, _v_F, _v_G);
      if (Is_exception_result(_v_val)){
	camlidl_rddidd_op_exn = _v_val;
	exception=1;
      }
      else
	val = Double_val(_v_val);
    }
    res = exception ? NULL : cuddUniqueConst(man,val);
    End_roots()
  }
  else {
    res = NULL;
  }
  return res;
}
static DdNode* camlidl_rddidd_mapterop_aux(DdManager* man, DdNode** f, DdNode** g, DdNode** h)
{
  value _v_F,_v_G,_v_H,_v_val;
  DdNode *F, *G, *H, *res;
  double val;
  int exception;

  F = *f; G = *g; H = *h;
  if (cuddIsConstant(F) && cuddIsConstant(G) && cuddIsConstant(H)) {
    exception = 0;
    _v_F = _v_G = _v_H = _v_val = Val_unit;
    Begin_roots4(_v_F,_v_G,_v_H,_v_val)
    if (camlidl_rddidd_is_idd){
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
      _v_H = Val_int((int)(cuddV(H)));
    }
    else {
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
      _v_H = copy_double(cuddV(H));
    }
    _v_val = callback3_exn(camlidl_rddidd_op_closure, _v_F, _v_G, _v_H);
    if (Is_exception_result(_v_val)){
      camlidl_rddidd_op_exn = _v_val;
      exception=1;
    }
    else
      val = camlidl_rddidd_is_idd ? (double)(Int_val(_v_val)) : Double_val(_v_val);
    res = exception ? NULL : cuddUniqueConst(man,val);
    End_roots()
  }
  else {
    res = NULL;
  }
  return res;
}
static DdNode* camlidl_rddidd_mapcmpop_aux(DdManager* man, DdNode** f, DdNode** g)
{
  value _v_F,_v_G,_v_val;
  DdNode *F, *G, *res, *one;
  int exception=0;
  int val=0;

  F = *f; G = *g;
  one = DD_ONE(man);
  if (
      (camlidl_rddidd_op_isbottom && cuddIsConstant(F) && camlidl_rddidd_op_bottom==cuddV(F)) ||
      (camlidl_rddidd_op_istop && cuddIsConstant(G) && camlidl_rddidd_op_top==cuddV(F)))
    return one;

  if (cuddIsConstant(F) && cuddIsConstant(G)) {
    exception = 0;
    _v_F = _v_G = Val_unit;
    Begin_roots2(_v_F,_v_G)
    if (camlidl_rddidd_is_idd){
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
    }
    else {
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
    }
    _v_val = callback2_exn(camlidl_rddidd_op_closure, _v_F, _v_G);
    if (Is_exception_result(_v_val)){
      camlidl_rddidd_op_exn = _v_val;
      exception=1;
    }
    else
      val = Int_val(_v_val);
    End_roots();
  }
  else {
    res = NULL;
  }
  res = exception ? NULL : (val ? one : Cudd_Not(one));
  return res;
}
value camlidl_cudd_rddidd_mapunop(value _v_is_idd, value _v_f, value _v_no)
{
  CAMLparam3(_v_is_idd,_v_f,_v_no); CAMLlocal1(_v_res);
  node__t no,_res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_unop_closure != Val_unit){
    failwith("RddIdd.mapunop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_unop_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no;
  camlidl_cudd_node_ml2c(_v_no,&no);
  _res.man = no.man;
  _res.node = Cuddaux_addApply1(no.man->man, camlidl_rddidd_mapunop_aux, no.node);
  camlidl_rddidd_unop_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapbinop(value _v_is_idd,
				   value _v_commutative,
				   value _v_idempotent,
				   value _v_oabsorbant,
				   value _v_oneutral,
				   value _v_f, value _v_no1, value _v_no2)
{
  CAMLparam5(_v_is_idd,_v_commutative,_v_idempotent,_v_oabsorbant,_v_oneutral);
  CAMLxparam3(_v_f,_v_no1,_v_no2);
  CAMLlocal1(_v_res);
  node__t no1,no2,_res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  camlidl_rddidd_op_commutative = Int_val(_v_commutative);
  camlidl_rddidd_op_idempotent = Int_val(_v_idempotent);
  camlidl_rddidd_op_isabsorbant = Is_block(_v_oabsorbant);
  camlidl_rddidd_op_isneutral = Is_block(_v_oneutral);
  if (camlidl_rddidd_op_isabsorbant){
    value vF = Field(_v_oabsorbant,0);
    value vG = Field(_v_oabsorbant,1);
    value vH = Field(_v_oabsorbant,2);
    value vI = Field(_v_oabsorbant,3);
    if (camlidl_rddidd_is_idd){
      camlidl_rddidd_op_absorbant_F = (double)(Int_val(vF));
      camlidl_rddidd_op_absorbant_G = (double)(Int_val(vG));
      camlidl_rddidd_op_absorbant_H = (double)(Int_val(vH));
      camlidl_rddidd_op_absorbant_I = (double)(Int_val(vI));
    }
    else {
      camlidl_rddidd_op_absorbant_F = Double_val(vF);
      camlidl_rddidd_op_absorbant_G = Double_val(vG);
      camlidl_rddidd_op_absorbant_H = Double_val(vH);
      camlidl_rddidd_op_absorbant_I = Double_val(vI);
    }
  }
  if (camlidl_rddidd_op_isneutral){
    value vF = Field(_v_oneutral,0);
    value vG = Field(_v_oneutral,1);
    if (camlidl_rddidd_is_idd){
      camlidl_rddidd_op_neutral_F = (double)(Int_val(vF));
      camlidl_rddidd_op_neutral_G = (double)(Int_val(vG));
    }
    else {
      camlidl_rddidd_op_neutral_F = Double_val(vF);
      camlidl_rddidd_op_neutral_G = Double_val(vG);
    }
  }
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("Rdd|Idd.mapbinop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_op_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  _res.man = no1.man;
  _res.node = Cuddaux_addApply2(no1.man->man, camlidl_rddidd_mapbinop_aux, no1.node, no2.node);
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapbinop_byte(value * argv, int argn)
{
  return camlidl_cudd_rddidd_mapbinop(argv[0],argv[1],argv[2],argv[3],
				      argv[4],argv[5],argv[6],argv[7]);
}
value camlidl_cudd_rddidd_mapterop(value _v_is_idd, value _v_f, value _v_no1, value _v_no2, value _v_no3)
{
  CAMLparam5(_v_is_idd,_v_f,_v_no1,_v_no2,_v_no3);
  CAMLlocal1(_v_res);
  node__t no1,no2,no3,_res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("Rdd|Idd.mapterop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_op_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_rddidd_op_val3 = _v_no3;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  camlidl_cudd_node_ml2c(_v_no3,&no3);
  _res.man = no1.man;
  _res.node = Cuddaux_addApply3(no1.man->man, camlidl_rddidd_mapterop_aux, no1.node, no2.node, no3.node);
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  camlidl_rddidd_op_val3 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapcmpop(value _v_is_idd,
				   value _v_obottom,
				   value _v_otop,
				   value _v_f, value _v_no1, value _v_no2)
{
  CAMLparam5(_v_is_idd,_v_obottom,_v_otop,_v_f,_v_no1);
  CAMLxparam1(_v_no2);
  CAMLlocal1(_v_res);
  node__t no1,no2;
  int res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  camlidl_rddidd_op_isbottom = Is_block(_v_obottom);
  camlidl_rddidd_op_istop = Is_block(_v_otop);
  if (camlidl_rddidd_op_isbottom){
    value v = Field(_v_obottom,0);
    if (camlidl_rddidd_is_idd){
      camlidl_rddidd_op_bottom = (double)(Int_val(v));
    }
    else {
      camlidl_rddidd_op_bottom = Double_val(v);
    }
  }
  if (camlidl_rddidd_op_istop){
    value v = Field(_v_otop,0);
    if (camlidl_rddidd_is_idd){
      camlidl_rddidd_op_top = (double)(Int_val(v));
    }
    else {
      camlidl_rddidd_op_top = Double_val(v);
    }
  }
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("Rdd|Idd.mapcmpop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_op_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  res = Cuddaux_addLeq(no1.man->man, camlidl_rddidd_mapcmpop_aux, no1.node, no2.node);
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else if (res==-1){
    Cudd_ErrorType err = Cudd_ReadErrorCode(no1.man->man);
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
	    "Cudd: Cuddaux_addLeq returned -1; ErrorCode = %s",
	    s);
    failwith(camlidl_cudd_msg);
  }
  else {
    _v_res = Val_bool(res);
  }
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapcmpop_byte(value * argv, int argn)
{
  return camlidl_cudd_rddidd_mapcmpop(argv[0],argv[1],argv[2],argv[3],
				      argv[4],argv[5]);
}
value camlidl_cudd_rddidd_mapexistop(value _v_is_idd,
				     value _v_absorbant,
				     value _v_f, value _v_no1, value _v_no2)
{
  CAMLparam5(_v_is_idd,_v_absorbant,_v_f,_v_no1,_v_no2);
  CAMLlocal1(_v_res);
  node__t no1,no2,_res;
  DdNode* background;
  double d;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("Rdd|Idd.mapbinop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_is_idd){
    d = (double)(Int_val(_v_absorbant));
  }
  else {
    d = Double_val(_v_absorbant);
  }
  camlidl_rddidd_op_commutative = 1;
  camlidl_rddidd_op_idempotent = 0;
  camlidl_rddidd_op_isabsorbant = 0;
  camlidl_rddidd_op_isneutral = 0;
  camlidl_rddidd_op_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  background = Cudd_addConst(no1.man->man,d);
  if (background==0){
    _res.man = no1.man;
    _res.node = NULL;
  }
  else {
    cuddRef(background);
    _res.man = no1.man;
    _res.node = Cuddaux_addAbstract(no1.man->man, camlidl_rddidd_mapbinop_aux, no2.node, no1.node, background);
    Cudd_RecursiveDeref(no1.man->man,background);
  }
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapexistandop(value _v_is_idd,
					value _v_absorbant,
					value _v_opexist, value _v_no1, value _v_no2, value _v_no3)
{
  CAMLparam5(_v_is_idd,_v_absorbant,_v_opexist,_v_no1,_v_no2);
  CAMLxparam1(_v_no3);
  CAMLlocal1(_v_res);
  node__t no1,no2,no3,_res;
  DdNode* background;
  double d;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("Rdd|Idd.mapbinop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_is_idd){
    d = (double)(Int_val(_v_absorbant));
  }
  else {
    d = Double_val(_v_absorbant);
  }
  camlidl_rddidd_op_commutative = 1;
  camlidl_rddidd_op_idempotent = 0;
  camlidl_rddidd_op_isabsorbant = 0;
  camlidl_rddidd_op_isneutral = 0;
  camlidl_rddidd_op_closure = _v_opexist;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_rddidd_op_val3 = _v_no3;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  camlidl_cudd_node_ml2c(_v_no3,&no3);
  background = Cudd_addConst(no1.man->man,d);
  if (background==0){
    _res.man = no1.man;
    _res.node = NULL;
  }
  else {
    cuddRef(background);
    _res.man = no1.man;
    _res.node = Cuddaux_addBddAndAbstract(no1.man->man, camlidl_rddidd_mapbinop_aux, no2.node, no3.node, no1.node, background);
    Cudd_RecursiveDeref(no1.man->man,background);
  }
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  camlidl_rddidd_op_val3 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapexistandop_byte(value * argv, int argn)
{
  return camlidl_cudd_rddidd_mapexistandop(argv[0],argv[1],argv[2],argv[3],
					   argv[4],argv[5]);
}
value camlidl_cudd_rddidd_mapexistandapplyop(value _v_is_idd,
					     value _v_absorbant,
					     value _v_op, value _v_opexist, 
					     value _v_no1, value _v_no2, value _v_no3)
{
  CAMLparam5(_v_is_idd,_v_absorbant,_v_op,_v_opexist,_v_no1);
  CAMLxparam2(_v_no2,_v_no3);
  CAMLlocal1(_v_res);
  node__t no1,no2,no3,_res;
  DdNode* background;
  double d;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  if (camlidl_rddidd_op_closure != Val_unit || 
      camlidl_rddidd_unop_closure != Val_unit){
    failwith("Rdd|Idd.mapbinop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_is_idd){
    d = (double)(Int_val(_v_absorbant));
  }
  else {
    d = Double_val(_v_absorbant);
  }
  camlidl_rddidd_op_commutative = 1;
  camlidl_rddidd_op_idempotent = 0;
  camlidl_rddidd_op_isabsorbant = 0;
  camlidl_rddidd_op_isneutral = 0;
  camlidl_rddidd_unop_closure = _v_op;
  camlidl_rddidd_op_closure = _v_opexist;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no1;
  camlidl_rddidd_op_val2 = _v_no2;
  camlidl_rddidd_op_val3 = _v_no3;
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  camlidl_cudd_node_ml2c(_v_no3,&no3);
  background = Cudd_addConst(no1.man->man,d);
  if (background==0){
    _res.man = no1.man;
    _res.node = NULL;
  }
  else {
    cuddRef(background);
    _res.man = no1.man;
    _res.node = Cuddaux_addApplyBddAndAbstract(no1.man->man, camlidl_rddidd_mapunop_aux, camlidl_rddidd_mapbinop_aux, no2.node, no3.node, no1.node, background);
    Cudd_RecursiveDeref(no1.man->man,background);
  }
  camlidl_rddidd_unop_closure = Val_unit;
  camlidl_rddidd_op_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  camlidl_rddidd_op_val2 = Val_unit;
  camlidl_rddidd_op_val3 = Val_unit;
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_cudd_rddidd_mapexistandapplyop_byte(value * argv, int argn)
{
  return camlidl_cudd_rddidd_mapexistandapplyop(argv[0],argv[1],argv[2],argv[3],
					   argv[4],argv[5],argv[6]);
}
value camlidl_cudd_rddidd_mapvectorcomposeapply(value _v_vec, value _v_op, value _v_no)
{
  CAMLparam3(_v_vec,_v_op,_v_no); CAMLlocal2(_v,_v_res);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t no; /*in*/
  bdd__t _res;
  int i;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  if (camlidl_rddidd_op_closure != Val_unit || 
      camlidl_rddidd_unop_closure != Val_unit){
    failwith("Rdd|Idd.mapbinop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_unop_closure = _v_op;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no;

  camlidl_cudd_node_ml2c(_v_no, &no);
  size = Wosize_val(_v_vec);
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  for (i = 0; i<size; i++) {
    bdd__t _no;
    _v = Field(_v_vec, i);
    camlidl_cudd_node_ml2c(_v, &_no);
    if (_no.man != no.man)
      failwith("Rdd.mapvectorcomposeapply called with BDDs belonging to different managers !");
    vec[i] = _no.node;
  }
  _res.man = no.man;
  _res.node = Cuddaux_addApplyVectorCompose(no.man->man, camlidl_rddidd_mapunop_aux, no.node, vec);
  camlidl_rddidd_unop_closure = Val_unit;
  camlidl_rddidd_op_val1 = Val_unit;
  free(vec);
  if (camlidl_rddidd_op_exn!=Val_unit){
    assert(_res.node==NULL);
    assert(Is_exception_result(camlidl_rddidd_op_exn));
    caml_raise(Extract_exception(camlidl_rddidd_op_exn));
  }
  else
    _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
