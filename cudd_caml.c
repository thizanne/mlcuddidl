/* $Id: cudd_caml.c,v 1.3 2005/06/14 14:41:05 bjeannet Exp $ */

/* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  */

#include <assert.h>
#include <stdio.h>
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
  if (camlidl_cudd_gc_fun == Val_unit){
    caml_register_global_root(&camlidl_cudd_gc_fun);
    caml_register_global_root(&camlidl_cudd_reordering_fun);
  }
  camlidl_cudd_gc_fun = _v_gc;
  camlidl_cudd_reordering_fun = _v_reordering;
  CAMLreturn(Val_unit);
}

int camlidl_cudd_garbage(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_gc_fun==Val_unit)
    camlidl_cudd_gc_fun = *(caml_named_value("gc_full_major"));
  if (camlidl_cudd_gc_fun == NULL){
    fprintf(stderr,"mlcuddidl: cudd_caml.c: camlidl_cudd_garbage: internal error");
    abort();
  }
  callback(camlidl_cudd_gc_fun,Val_unit);
  return 1;
}

int camlidl_cudd_reordering(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_reordering_fun==Val_unit)
    camlidl_cudd_reordering_fun = *(caml_named_value("gc_full_major"));
  if (camlidl_cudd_reordering_fun == NULL){
    fprintf(stderr,"mlcuddidl: cudd_caml.c: camlidl_cudd_reordering: internal error");
    abort();
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

int camlidl_custom_manager_compare(value val1, value val2)
{
  CAMLparam2(val1,val2);
  int res;
  DdManager* man1 = *(manager__t*)(Data_custom_val(val1));
  DdManager* man2 = *(manager__t*)(Data_custom_val(val2));
  res = (long)man1==(long)man2 ? 0 : (long)man1<(long)man2 ? -1 : 1;
  CAMLreturn(res);
}
long camlidl_custom_manager_hash(value val)
{
  CAMLparam1(val);
  DdManager* man1 = *(manager__t*)(Data_custom_val(val));
  long hash = (long)man1;
  CAMLreturn(hash);
}

struct custom_operations camlidl_custom_manager = {
  "camlidl_cudd_custom_node",
  custom_finalize_default,
  &camlidl_custom_manager_compare,
  &camlidl_custom_manager_hash,
  custom_serialize_default,
  custom_deserialize_default
};

/* \subsubsection{Standard nodes (BDDs \& ADDs)} */

void camlidl_custom_node_finalize(value val)
{
  DdManager* man = ((node__t*)(Data_custom_val(val)))->man;
  DdNode* node = ((node__t*)(Data_custom_val(val)))->node;
  assert (Cudd_Regular(node)->ref >= 1);
  Cudd_RecursiveDeref(man,node);
}
int camlidl_custom_node_compare(value val1, value val2)
{
  CAMLparam2(val1,val2);
  int res;
  DdManager* man1 = ((node__t*)(Data_custom_val(val1)))->man;
  DdNode* node1 = ((node__t*)(Data_custom_val(val1)))->node;
  DdManager* man2 = ((node__t*)(Data_custom_val(val2)))->man;
  DdNode* node2 = ((node__t*)(Data_custom_val(val2)))->node;

  res = (long)man1==(long)man2 ? 0 : ( (long)man1<(long)man2 ? -1 : 1);
  if (res==0)
    res = (long)node1==(long)node2 ? 0 : ( (long)node1<(long)node2 ? -1 : 1);
  CAMLreturn(res);
}
long camlidl_custom_node_hash(value val)
{
  CAMLparam1(val);
  DdNode* node = ((node__t*)(Data_custom_val(val)))->node;
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
  node__t* no;
  DdManager* man;
  DdNode* node;
  no = (node__t*)(Data_custom_val(val));
  man = no->man;
  node = no->node;
  assert((Cudd_Regular(node))->ref >= 1);
  Cudd_IterDerefBdd(man,node);
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

  if(*man==NULL)
    failwith("Cudd: a function returned a null manager");
  val = alloc_custom(&camlidl_custom_manager, sizeof(manager__t), 0, 1);
  *((manager__t*)(Data_custom_val(val))) = *man;
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
    Cudd_ErrorType err = Cudd_ReadErrorCode(no->man);
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
    Cudd_ErrorType err = Cudd_ReadErrorCode(bdd->man);
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
   if (no.node == DD_ONE(no.man))
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
  _res.node = Cudd_VectorSupport(_res.man, vec, size);
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
  _res.node = Cudd_VectorSupport(_res.man, vec, size);
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
  _res.node = Cudd_bddVectorCompose(no.man, no.node, vec);
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
  _res.node = Cuddaux_addVectorCompose(no.man, no.node, vec);
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
  _res.node = Cudd_bddPermute(no.man,no.node,permut);
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
  _res.node = Cudd_addPermute(no.man,no.node,permut);
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
  if (Cudd_ReorderingStatus(no.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man);
  }
  snode.man = no.man;
  Cudd_ForeachNode(no.man,no.node,gen,snode.node)
    {
      _v_snode = camlidl_cudd_node_c2ml(&snode);
      callback(_v_closure,_v_snode);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
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
  if (Cudd_ReorderingStatus(no.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man);
  }
  size = no.man->size;
  Cudd_ForeachCube(no.man,no.node,gen,array,val)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	  /* Allowed according to caml/memory.h memory.c */
	}
      }
      callback(_v_closure,_v_array);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
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
  if (Cudd_ReorderingStatus(no.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man);
  }
  size = no.man->size;
  Cudd_ForeachCube(no.man,no.node,gen,array,val)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = alloc(size,0);
	for(i=0; i<size; i++){
	Store_field(_v_array,i,Val_int(array[i]));
	/* Allowed according to caml/memory.h memory.c */
	}
      }
      _v_val = copy_double(val);
      callback2(_v_closure,_v_array,_v_val);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
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
  if (Cudd_ReorderingStatus(no.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man);
  }
  else
    autodyn=0;

  size = no.man->size;
  Cudd_ForeachCube(no.man,no.node,gen,array,val)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	  /* Allowed according to caml/memory.h memory.c */
	}
      }
      ival = (int)val;
      _v_val = Val_int(ival);
      callback2(_v_closure,_v_array,_v_val);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
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
    failwith("Bdd.iter_ime called with BDDs belonging to different managers !");
  }
  autodyn = 0;
  if (Cudd_ReorderingStatus(lower.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(lower.man);
  }
  size = lower.man->size;
  Cudd_ForeachPrime(lower.man,lower.node,upper.node,gen,array)
    {
      if (size==0) {
	_v_array = Atom(0);
      }
      else {
	_v_array = alloc(size,0);
	for(i=0; i<size; i++){
	  Store_field(_v_array,i,Val_int(array[i]));
	  /* Allowed according to caml/memory.h memory.c */
	}
      }
      callback(_v_closure,_v_array);
    }
  if (autodyn) Cudd_AutodynEnable(lower.man,CUDD_REORDER_SAME);
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
  _res.node = Cudd_FindEssential(no.man,no.node);
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
  maxsize = Cudd_ReadSize(man);
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
  _res.node = Cudd_CubeArrayToBdd(man,array);
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

  zero = Cudd_Not(DD_ONE(node.man));
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
  size = no.man->size;
  string = size>1024 ? (char*)malloc(size) : array;
  if (string==NULL){
    failwith("Bdd.pick_minterm: out of memory");
  }
  res = Cudd_bddPickOneCube(no.man,no.node,string);
  if (res==0){
    if (size>1024) free(string);
    failwith("Bdd.pick_minterm: (probably) second argument is not a positive cube");
  }
  _v_array = alloc(size,0);
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
  ret = array_of_support(no2.man,no2.node,&vars,&size);
  if (ret==1){
    failwith("Bdd.pick_cube_on_support: the second argument is not a positive cube");
  }
  else if (ret==2){
    failwith("Bdd.pick_cube_on_support: empty support or out of memory");
  }
  res.man = no1.man;
  res.node = Cudd_bddPickOneMinterm(no1.man,no1.node,vars,size);
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
  ret = array_of_support(no2.man,no2.node,&vars,&size);
  if (ret==1){
    failwith("Bdd.pick_cubes_on_support: the second argument is not a positive cube");
  }
  else if (ret==2){
    failwith("Bdd.pick_cube_on_support: empty support or out of memory");
  }
  array = Cudd_bddPickArbitraryMinterms(no1.man,no1.node,vars,size,k);
  free(vars);
  if (array==NULL){
    failwith("Bdd.pick_cubes_on_support: out of memory, or first argument is false, or wrong support, or number of minterms < k");
  }
  
  if (k==0){
    _v_res = Atom(0);
  }
  else {
    _v_res = alloc(k,0);
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
value camlidl_cudd_rdd_nodes_below_level(value _v_no, value _v_olevel)
{
  CAMLparam2(_v_no,_v_olevel);
  CAMLlocal2(res,v);
  node__t no;
  int level,i,size;
  list_t *p, *list;

  camlidl_cudd_node_ml2c(_v_no,&no);
  if (Is_long(_v_olevel))
    level = CUDD_MAXINDEX;
  else {
    value _v_level = Field(_v_olevel,0);
    level = Int_val(_v_level);
  }

  list = Cuddaux_NodesBelowLevel(no.man,no.node,level);

  /* Now, we build the array of nodes */
  /* First, count the elements */
  size = 0;
  for(p=list; p!=NULL; p=p->next) size++;
  /* Create and fill the array */
  if (size==0){
    res = Atom(0);
  }
  else {
    res = alloc(size,0);
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
  int val,i,size;

  camlidl_cudd_node_ml2c(_v_no,&no);
  assert(no.node->ref>=1);

  list = Cuddaux_NodesBelowLevel(no.man,no.node,CUDD_MAXINDEX);

  /* Now, we build the array of nodes */
  /* First, count the elements */
  size = 0;
  for(p=list; p!=NULL; p=p->next) size++;
  /* Create and fill the array */
  if (size==0){
    res = Atom(0);
  }
  else {
    res = alloc(size,0);
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
  int i,size;
  double val;

  camlidl_cudd_node_ml2c(_v_no,&no);
  list = Cuddaux_NodesBelowLevel(no.man,no.node,CUDD_MAXINDEX);

  /* Now, we build the array of nodes */
  /* First, count the elements */
  size = 0;
  for(p=list; p!=NULL; p=p->next) size++;
  /* Create and fill the array */
  if (size==0){
    res = Atom(Double_array_tag);
  }
  else {
    res = alloc(size * Double_wosize,Double_array_tag);
    for(p=list,i=0; p!=NULL; p=p->next,i++){
      assert(p->node->ref>=1);
      val = cuddV(p->node);
      Store_double_field(res,i,val);
    }
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
  Cudd_PrintMinterm(no.man,no.node);
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
    _v_f = _v_val = 0;
    Begin_roots2(_v_f,_v_val)
      if (camlidl_rddidd_is_idd){
	_v_f = Val_int((int)(cuddV(f)));
	_v_val = callback_exn(camlidl_rddidd_op_closure, _v_f);
	if (Is_exception_result(_v_val)){
	  camlidl_rddidd_op_exn = _v_val;
	  exception=1;
	}
	else
	  val = (double)(Int_val(_v_val));
      }
      else {
	_v_f = copy_double(cuddV(f));
	_v_val = callback_exn(camlidl_rddidd_op_closure, _v_f);
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
  if (cuddIsConstant(F) && cuddIsConstant(G)) {
    exception = 0;
    _v_F = _v_G = _v_val = 0;
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
    _v_F = _v_G = _v_H = _v_val = 0;
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
value camlidl_cudd_rddidd_mapunop(value _v_is_idd, value _v_f, value _v_no)
{
  CAMLparam3(_v_is_idd,_v_f,_v_no); CAMLlocal1(_v_res);
  node__t no,_res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  if (camlidl_rddidd_op_closure != Val_unit){
    failwith("RddIdd.mapunop: this family of functions cannot be called recursively !");
  }
  camlidl_rddidd_op_closure = _v_f;
  camlidl_rddidd_op_exn = Val_unit;
  camlidl_rddidd_op_val1 = _v_no;
  camlidl_cudd_node_ml2c(_v_no,&no);
  _res.man = no.man;
  _res.node = Cuddaux_AddApply1(no.man, camlidl_rddidd_mapunop_aux, no.node);
  camlidl_rddidd_op_closure = Val_unit;
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
value camlidl_cudd_rddidd_mapbinop(value _v_is_idd, value _v_bool, value _v_f, value _v_no1, value _v_no2)
{
  CAMLparam5(_v_is_idd,_v_bool,_v_f,_v_no1,_v_no2); CAMLlocal1(_v_res);
  node__t no1,no2,_res;

  if (!camlidl_op_initialized) camlidl_op_initialize();
  camlidl_rddidd_is_idd = Int_val(_v_is_idd);
  camlidl_rddidd_op_commutative = Int_val(_v_bool);
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
  _res.node = Cuddaux_AddApply2(no1.man, camlidl_rddidd_mapbinop_aux, no1.node, no2.node);
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
  _res.node = Cuddaux_AddApply3(no1.man, camlidl_rddidd_mapterop_aux, no1.node, no2.node, no3.node);
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
