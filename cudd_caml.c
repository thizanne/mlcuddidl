/* $Id: cudd_caml.c,v 1.3 2005/06/14 14:41:05 bjeannet Exp $ */

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

value camlidl_dd_set_gc(value _v_heap, value _v_gc, value _v_reordering)
{
  CAMLparam3(_v_heap,_v_gc,_v_reordering);
  int heap, res;

  heap = Int_val(_v_heap);
  camlidl_cudd_heap = heap;
  if (camlidl_cudd_gc_fun == Val_unit){
    register_global_root(&camlidl_cudd_gc_fun);
    register_global_root(&camlidl_cudd_reordering_fun);
  }
  camlidl_cudd_gc_fun = _v_gc;
  camlidl_cudd_reordering_fun = _v_reordering;
  CAMLreturn(Val_unit);
}

int camlidl_dd_garbage(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_gc_fun==Val_unit)
    camlidl_cudd_gc_fun = *(caml_named_value("gc_full_major"));
  else 
    callback(camlidl_cudd_gc_fun,Val_unit);
  return 1;
}

int camlidl_dd_reordering(DdManager* dd, const char* s, void* data)
{
  if (camlidl_cudd_reordering_fun==Val_unit)
    camlidl_cudd_reordering_fun = *(caml_named_value("gc_full_major"));
  else 
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
value manager_c2ml(manager__t* man)
{
  value val;

  if(*man==NULL)
    failwith("Cudd: a function returned a null manager");
  val = alloc_custom(&camlidl_custom_manager, sizeof(manager__t), 0, 1);
  *((manager__t*)(Data_custom_val(val))) = *man;
  return val;
}
void manager_ml2c(value val, manager__t* man)
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

value node_c2ml(struct node__t* no)
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
void node_ml2c(value val, struct node__t *node)
{
  node->man = ((node__t*)(Data_custom_val(val)))->man;
  node->node = ((node__t*)(Data_custom_val(val)))->node;
}

/* \subsubsection{BDD nodes} */

value bdd_c2ml(struct node__t* bdd)
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

value camlidl_bdd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal3(vres,vthen,velse);
  bdd__t no;
  DdNode* N;

  node_ml2c(vno, &no);
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
    vthen = bdd_c2ml(&bthen);
    velse = bdd_c2ml(&belse);
    vres = alloc_small(3,1);
    Field(vres,0) = Val_int(N->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_rdd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal4(vres,vthen,velse,vr);
  rdd__t no;

  node_ml2c(vno, &no);
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
    vthen = node_c2ml(&bthen);
    velse = node_c2ml(&belse);
    vres = alloc_small(3,1);
    Field(vres,0) = Val_int(no.node->index);
    Field(vres,1) = vthen;
    Field(vres,2) = velse;
  }
  CAMLreturn(vres);
}

value camlidl_idd_inspect(value vno)
{
  CAMLparam1(vno); CAMLlocal4(vres,vthen,velse,vr);
  rdd__t no;

  node_ml2c(vno, &no);
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
    vthen = node_c2ml(&bthen);
    velse = node_c2ml(&belse);
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

value camlidl_dd_list_of_support(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,r);
  bdd__t node;
  DdNode * N;

  node_ml2c(_v_no,&node);
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

value camlidl_bdd_vectorsupport(value _v_vec)
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
  node_ml2c(_v_no, &_no);
  _res.man = _no.man;

  for (i = 0; i<size; i++) {
    _v_no = Field(_v_vec, i);
    node_ml2c(_v_no, &_no);
    vec[i] = _no.node;
    if (_no.man != _res.man)
      failwith("Bdd.vectorsupport called with BDDs belonging to different managers !");
  }
  _res.node = Cudd_VectorSupport(_res.man, vec, size);
  _v_res = bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}

value camlidl_rdd_vectorsupport2(value _v_vec1, value _v_vec2)
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
  node_ml2c(_v_no, &_no);
  _res.man = _no.man;

  index = 0;
  for (i = 0; i<size1; i++) {
    _v_no = Field(_v_vec1, i);
    node_ml2c(_v_no, &_no);
    vec[index++] = _no.node;
    if (_no.man != _res.man)
      failwith("Rdd.vectorsupport2 called with BDDs belonging to different managers !");
  }
  for (i = 0; i<size2; i++) {
    _v_no = Field(_v_vec2, i);
    node_ml2c(_v_no, &_no);
    vec[index++] = _no.node;
    if (_no.man != _res.man)
      failwith("Rdd.vectorsupport2 called with BDDs belonging to different managers !");
  }
  _res.node = Cudd_VectorSupport(_res.man, vec, size);
  _v_res = bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}


/* %======================================================================== */
/* \section{Logical operations} */
/* %======================================================================== */

value camlidl_bdd_vectorcompose(value _v_vec, value _v_no)
{
  CAMLparam2(_v_vec,_v_no); CAMLlocal2(_v,_vres);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t no; /*in*/
  bdd__t _res;
  int i;

  node_ml2c(_v_no, &no);
  size = Wosize_val(_v_vec);
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  for (i = 0; i<size; i++) {
    bdd__t _no;
    _v = Field(_v_vec, i);
    node_ml2c(_v, &_no);
    if (_no.man != no.man)
      failwith("Bdd.vectorcompose called with BDDs belonging to different managers !");
    vec[i] = _no.node;
  }
  _res.man = no.man;
  _res.node = Cudd_bddVectorCompose(no.man, no.node, vec);
  _vres = bdd_c2ml(&_res);
  free(vec);
  CAMLreturn(_vres);
}

value camlidl_rdd_vectorcompose(value _v_vec, value _v_no)
{
  CAMLparam2(_v_vec,_v_no); CAMLlocal2(_v,_vres);
  DdNode **vec; /*in*/
  int size; /*in*/
  bdd__t no; /*in*/
  bdd__t _res;
  int i;

  node_ml2c(_v_no, &no);
  size = Wosize_val(_v_vec);
  vec = (DdNode**)malloc(size * sizeof(DdNode*));
  for (i = 0; i<size; i++) {
    bdd__t _no;
    _v = Field(_v_vec, i);
    node_ml2c(_v, &_no);
    if (_no.man != no.man)
      failwith("Bdd.vectorcompose called with BDDs belonging to different managers !");
    vec[i] = _no.node;
  }
  _res.man = no.man;
  _res.node = Cuddaux_addVectorCompose(no.man, no.node, vec);
  _vres = node_c2ml(&_res);
  free(vec);
  CAMLreturn(_vres);
}

/* %======================================================================== */
/* \section{Variable Mapping} */
/* %======================================================================== */

value camlidl_bdd_permute(value _v_no, value _v_permut)
{
  CAMLparam2(_v_no,_v_permut); CAMLlocal1(_vres);
  bdd__t no; /*in*/
  int *permut; /*in*/
  bdd__t _res;
  mlsize_t i,size;

  node_ml2c(_v_no, &no);
  size = Wosize_val(_v_permut);
  permut = malloc(size * sizeof(int));
  for (i=0; i < size; i++) {
    value v = Field(_v_permut, i);
    permut[i] = Int_val(v);
  }
  _res.man = no.man;
  _res.node = Cudd_bddPermute(no.man,no.node,permut);
  _vres = bdd_c2ml(&_res);
  free(permut);
  CAMLreturn(_vres);
}

value camlidl_rdd_permute(value _v_no, value _v_permut)
{
  CAMLparam2(_v_no,_v_permut); CAMLlocal1(_vres);
  bdd__t no; /*in*/
  int *permut; /*in*/
  bdd__t _res;
  mlsize_t i,size;

  node_ml2c(_v_no, &no);
  size = Wosize_val(_v_permut);
  permut = malloc(size * sizeof(int));
  for (i=0; i < size; i++) {
    value v = Field(_v_permut, i);
    permut[i] = Int_val(v);
  }
  _res.man = no.man;
  _res.node = Cudd_addPermute(no.man,no.node,permut);
  _vres = node_c2ml(&_res);
  free(permut);
  CAMLreturn(_vres);
}


/* %======================================================================== */
/* \section{Iterators} */
/* %======================================================================== */

value camlidl_dd_iter_node(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal1(_v_snode);
  DdGen* gen;
  bdd__t no;
  bdd__t snode;
  int autodyn;
  Cudd_ReorderingType heuristic;

  node_ml2c(_v_no,&no);
  autodyn = 0;
  if (Cudd_ReorderingStatus(no.man,&heuristic)){
    autodyn = 1;
    Cudd_AutodynDisable(no.man);
  }
  snode.man = no.man;
  Cudd_ForeachNode(no.man,no.node,gen,snode.node)
    {
      _v_snode = node_c2ml(&snode);
      callback(_v_closure,_v_snode);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

value camlidl_bdd_iter_cube(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal1(_v_array);
  bdd__t no;
  DdGen* gen;
  int* array;
  double val;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  node_ml2c(_v_no,&no);
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

value camlidl_rdd_iter_cube(value _v_closure, value _v_no)
{
  CAMLparam2(_v_closure,_v_no); CAMLlocal2(_v_array,_v_val);
  bdd__t no;
  DdGen* gen;
  int* array;
  double val;
  int size,i;
  int autodyn;
  Cudd_ReorderingType heuristic;

  node_ml2c(_v_no,&no);
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

value camlidl_idd_iter_cube(value _v_closure, value _v_no)
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

  node_ml2c(_v_no,&no);
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
      ival = (int)val;
      _v_val = Val_int(ival);
      callback2(_v_closure,_v_array,_v_val);
    }
  if (autodyn) Cudd_AutodynEnable(no.man,CUDD_REORDER_SAME);
  CAMLreturn(Val_unit);
}

/* %======================================================================== */
/* \section{Quantifications} */
/* %======================================================================== */

value camlidl_dd_cube_of_bdd(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal1(_v_res);
  node__t no;
  bdd__t _res;
  node_ml2c(_v_no,&no);
  _res.man = no.man;
  _res.node = Cudd_FindEssential(no.man,no.node);
  _v_res = bdd_c2ml(&_res);
  CAMLreturn(_v_res);
}

value camlidl_dd_list_of_cube(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal3(res,r,elt);
  bdd__t node;
  DdNode *zero, *f, *fv, *fnv;
  int index,sign;

  node_ml2c(_v_no,&node);
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

/* %======================================================================== */
/* \section{Guards and leaves} */
/* %======================================================================== */

/* List of nodes below a optional level */
value camlidl_rdd_nodes_below_level(value _v_no, value _v_olevel)
{
  CAMLparam2(_v_no,_v_olevel);
  CAMLlocal2(res,v);
  node__t no;
  int level,i,size;
  list_t *p, *list;

  node_ml2c(_v_no,&no);
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
      v = node_c2ml(&no);
      Store_field(res,i,v);
    }
  }
  list_free(list);
  CAMLreturn(res);
}

/* List of leaves of an idd. */
value camlidl_idd_leaves(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal2(res,v);
  node__t no;
  list_t *p, *list;
  int val,i,size;

  node_ml2c(_v_no,&no);
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
value camlidl_rdd_leaves(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal1(res);
  node__t no;
  list_t *p, *list;
  int i,size;
  double val;

  node_ml2c(_v_no,&no);
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

value camlidl_dd_print(value _v_no)
{
  CAMLparam1(_v_no);
  node__t no;

  node_ml2c(_v_no,&no);
  fflush(stdout);
  Cudd_PrintMinterm(no.man,no.node);
  fflush(stdout);
  CAMLreturn(Val_unit);
}
