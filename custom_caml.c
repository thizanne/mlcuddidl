/* User operations on MTBDDs */

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
#include <string.h>

value camlidl_cudd_custom_copy_shr(value arg)
{
  CAMLparam1(arg);
  CAMLlocal1(res);
  mlsize_t sz, i;
  tag_t tg;

  sz = Wosize_val(arg);
  if (sz == 0) CAMLreturn (arg);
  tg = Tag_val(arg);
  if (tg==Custom_tag){
    struct custom_operations *op = Custom_ops_val(arg);
    if (op->finalize!=NULL){
      fprintf(stderr,"\n\
Library mlcuddidl/Cudd module: custom_caml.c:\n\
camlidl_cudd_custom_copy_shr (probably called from Cudd.Mtbdd.unique):\n\
an OCaml value/type implemented as a custom block with a finalization\n\
function cannot be used as leaves of Mtbdds (for technical reasons).\n\
Sorry for that !\n\
The things to do is to first encapsulate the type into a record with one field:\n\
something like type 'a capsule = { val:'a }\n");
      abort();
    }
  }
  res = caml_alloc_shr(sz, tg);
  if (tg < No_scan_tag) {
    for (i = 0; i < sz; i++) caml_initialize(&Field(res, i), Field(arg, i));
  }
  else {
    memcpy(Bp_val(res), Bp_val(arg), sz * sizeof(value));
  }
  CAMLreturn (res);
}

/* pour debugger test_mtbdd.ml */
value camlidl_string_of_value(value arg)
{
  CAMLparam1(arg);
  CAMLlocal1(res);
  char str[10];

  sprintf(str,"%d",arg);
  res = caml_copy_string(str);
  CAMLreturn(res);
}

/* ========================================================================= */
/* Declarations */
/* ========================================================================= */

typedef struct camlidl_cudd_op_t {
  value closure;
  /* operation on leaves, or background value (for existandXX operations) or
     Val_unit for (existXX operations) */
  value special;
  /* function for special cases (called when one of the argument is a leaf)
     or Val_unit */
  short int optype;
  /* 0: op1
     1: op2
     2: test2
     3: op3
     4: exist
     5: existop
     6: existand
     7: existandop
     8: composeapply
  */
  short int ddtype;
  /* 0: add, 1:idd, 2:vdd */
  short int commutative;
  short int idempotent;
  short int cachetype;
  /* 0: global, 1: local and automatic, 2: local and manual allocation/clearing */
  DdHashTable* table;
    /* If localCache, hashtable */
  value op2;
  /* Either Val_unit or a value encapsulating a camlidl_cudd_op (for existXX) */
  value op1;
  /* Either Val_unit or a value encapsulating a camlidl_cudd_op (for existXXop) */
  struct man__t* man;
  /* Used temporarily during execution */
} camlidl_cudd_op_t;

/* We need to keep somewhere the list of all operations with local caches,
   because in case of reordering these caches should be flushed */
static camlidl_cudd_op_t** camlidl_cudd_tabop = NULL;
static int camlidl_cudd_tabop_size = 0;

/* Exception */
static value camlidl_cudd_avdd_op_exn = Val_unit;

/* ========================================================================= */
/* Allocating and freeing camlidl_cudd_op_t */
/* ========================================================================= */

void camlidl_cudd_op_ml2c(value val, camlidl_cudd_op_t** op)
{
  *op = *((camlidl_cudd_op_t**)(Data_custom_val(val)));
}
void camlidl_custom_op_finalize(value val)
{
  int i;
  camlidl_cudd_op_t* op;

  camlidl_cudd_op_ml2c(val, &op);
  if (op->table != NULL){
    cuddHashTableQuit(op->table);
  }
  caml_remove_global_root(&op->closure);
  if (op->special != Val_unit) caml_remove_global_root(&op->special);
  if (op->op2 != Val_unit) caml_remove_global_root(&op->op2);
  if (op->op1 != Val_unit) caml_remove_global_root(&op->op1);

  /* Remove from the table */
  if (op->cachetype>0){
    for (i=0; i<camlidl_cudd_tabop_size; i++){
      if (camlidl_cudd_tabop[i]==op){
	camlidl_cudd_tabop[i]=NULL;
	break;
      }
    }
    assert(i<=camlidl_cudd_tabop_size);
  }
  free(op);
}
struct custom_operations camlidl_custom_op = {
  "camlidl_cudd_custom_op",
  &camlidl_custom_op_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

value camlidl_cudd_op_c2ml(camlidl_cudd_op_t* op)
{
  value val = caml_alloc_custom(&camlidl_custom_op, sizeof(camlidl_cudd_op_t*), 0, 1);
  *((camlidl_cudd_op_t**)(Data_custom_val(val))) = op;
  return val;
}


/* ========================================================================= */
/* Registering and removing user operations */
/* ========================================================================= */

int camlidl_cudd_custom_hook(DdManager* dd, const char* s, void* data)
{
  int i;
  if (camlidl_cudd_tabop != NULL){
    for (i=0; i<camlidl_cudd_tabop_size; i++){
      if (camlidl_cudd_tabop[i]!=NULL &&
	  camlidl_cudd_tabop[i]->table != NULL &&
	  camlidl_cudd_tabop[i]->table->manager == dd){
	cuddHashTableQuit(camlidl_cudd_tabop[i]->table);
	camlidl_cudd_tabop[i]->table = NULL;
      }
    }
  }
  return 1;
}

value camlidl_cudd_avdd_register_op
(value v_ddtype, value v_cachetype, value v_optype,
 value v_commutative, value v_idempotent,
 value v_op2, value v_op1,
 value v_special, value v_closure)
{
  CAMLparam5(v_cachetype, v_optype, v_ddtype, v_commutative, v_idempotent);
  CAMLxparam4(v_op2, v_op1, v_special, v_closure);
  CAMLlocal1(v_res);
  int i;

  /* if first call */
  if (camlidl_cudd_tabop==NULL){
    caml_register_global_root(&camlidl_cudd_avdd_op_exn);
    camlidl_cudd_tabop_size = 32;
    camlidl_cudd_tabop = malloc(32*sizeof(camlidl_cudd_op_t*));
    for (i=0; i<camlidl_cudd_tabop_size; i++) camlidl_cudd_tabop[i]=NULL;
  }
  /* Allocate the C structure */
  camlidl_cudd_op_t* op = (camlidl_cudd_op_t*)malloc(sizeof(camlidl_cudd_op_t));
  op->closure = v_closure;
  op->special = v_special;
  op->optype = Int_val(v_optype);
  op->ddtype = Int_val(v_ddtype);
  op->commutative = Int_val(v_commutative);
  op->idempotent = Int_val(v_idempotent);
  op->cachetype = Int_val(v_cachetype);
  op->table = NULL;
  op->op2 = v_op2;
  op->op1 = v_op1;
  op->man = NULL;
  caml_register_global_root(&op->closure);
  if (v_special != Val_unit) caml_register_global_root(&op->special);
  if (v_op2 != Val_unit) caml_register_global_root(&op->op2);
  if (v_op1 != Val_unit) caml_register_global_root(&op->op1);

  /* Add to the table */
  if (op->cachetype>0){
    for (i=0; i<camlidl_cudd_tabop_size; i++){
      if (camlidl_cudd_tabop[i]==NULL){
	camlidl_cudd_tabop[i]=op;
	break;
      }
    }
    if (i==camlidl_cudd_tabop_size+1){
      /* Resize the table */
      camlidl_cudd_tabop =
	realloc(camlidl_cudd_tabop,
		2*camlidl_cudd_tabop_size*sizeof(camlidl_cudd_op_t*));
      camlidl_cudd_tabop[i]=op;
      for (i=camlidl_cudd_tabop_size+1; i<2*camlidl_cudd_tabop_size; i++){
	camlidl_cudd_tabop[i]=NULL;
      }
      camlidl_cudd_tabop_size *= 2;
    }
  }
  v_res = camlidl_cudd_op_c2ml(op);
  CAMLreturn(v_res);
}

value camlidl_cudd_avdd_register_op_byte(value * argv, int argn)
{
  assert(argn==9);
  return camlidl_cudd_avdd_register_op(argv[0],argv[1],argv[2],argv[3],
					argv[4],argv[5],argv[6],argv[7],
					argv[8]);
}

value camlidl_cudd_avdd_op2_of_exist(value v_op)
{
   camlidl_cudd_op_t* op;
   camlidl_cudd_op_ml2c(v_op, &op);
   return op->op2;
}
value camlidl_cudd_avdd_op1_of_existop1(value v_op)
{
   camlidl_cudd_op_t* op;
   camlidl_cudd_op_ml2c(v_op, &op);
   return op->op1;
}

value camlidl_cudd_avdd_flush_op(value v_op)
{
  camlidl_cudd_op_t* op;
  camlidl_cudd_op_ml2c(v_op, &op);
  assert(op->table!=NULL && op->cachetype==2);
  cuddHashTableQuit(op->table);
  op->table = NULL;
  return Val_unit;
}
value camlidl_cudd_avdd_flush_allop(value v)
{
  int i;
  if (camlidl_cudd_tabop != NULL){
    for (i=0; i<camlidl_cudd_tabop_size; i++){
      if (camlidl_cudd_tabop[i]!=NULL && camlidl_cudd_tabop[i]->table!=NULL){
	cuddHashTableQuit(camlidl_cudd_tabop[i]->table);
	camlidl_cudd_tabop[i]->table = NULL;
      }
    }
  }
  return Val_unit;
}

/* ========================================================================= */
/* Main user operations */
/* ========================================================================= */
DdNode* camlidl_cudd_avdd_op1(DdManager* man, DDAUX_IDOP ptr, DdNode* f);
DdNode* camlidl_cudd_avdd_op2(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G);
DdNode* camlidl_cudd_avdd_cmpop(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G);
DdNode* camlidl_cudd_avdd_op3(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G, DdNode* H);

static inline
void freetable_if_different_manager(DdManager* man, camlidl_cudd_op_t* op)
{
  if (op->table!=NULL && op->table->manager != man){
    cuddHashTableQuit(op->table);
    op->table = NULL;
  }
}
static inline
void freetable_auto(camlidl_cudd_op_t* op)
{
  if (op->cachetype == 1 && op->table != NULL){
    cuddHashTableQuit(op->table);
    op->table=NULL;
  }
}
static inline
int alloctable(DdManager* man,
	       camlidl_cudd_op_t* op,
	       int arity)
{
  if (op->cachetype > 0 && op->table==NULL){
    op->table = cuddHashTableInit(man,arity,2);
    return (op->table!=NULL);
  }
  else
    return 1;
}

value camlidl_cudd_avdd_map_op(value v_op, value v_tno)
{
  CAMLparam2(v_op, v_tno); CAMLlocal2(v_no,v_res);
  int i,size;
  node__t no[3];
  node__t res;
  camlidl_cudd_op_t* op;
  camlidl_cudd_op_t* op2;
  camlidl_cudd_op_t* op1;

  camlidl_cudd_op_ml2c(v_op, &op);
  if (op->op2 != Val_unit)
    camlidl_cudd_op_ml2c(op->op2, &op2);
  else
    op2 = NULL;
  if (op->op1 != Val_unit)
    camlidl_cudd_op_ml2c(op->op1, &op1);
  else
    op1 = NULL;

  /* Initialize exception */
  camlidl_cudd_avdd_op_exn = Val_unit;
  /* Conversion from CAML */
  size = Wosize_val(v_tno);
  for (i=0; i<size; i++){
    v_no = Field(v_tno,i);
    camlidl_cudd_node_ml2c(v_no, &no[i]);
  }

  res.man = no[0].man;
  res.node = NULL;

  /* */
  freetable_if_different_manager(res.man->man, op);
  if (op2) freetable_if_different_manager(res.man->man, op2);
  if (op1) freetable_if_different_manager(res.man->man, op1);

  /* */
  op->man = no[0].man;
  if (op2) op2->man = no[0].man;
  if (op1) op1->man = no[0].man;

  switch (op->optype){
  case 0: /* op1 */
    if (size!=1) abort();
    if (alloctable(res.man->man,op,1)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    res.node = Cuddaux_addApply1(res.man->man,&op->table,op,camlidl_cudd_avdd_op1,no[0].node);
    break;
  case 1: /* op2 */
    if (size!=2) abort();
    if (no[0].man!=no[1].man){
      failwith("Dd: binary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    res.node = Cuddaux_addApply2(res.man->man, &op->table, op, op->commutative,
				 camlidl_cudd_avdd_op2, no[0].node, no[1].node);
    break;
  case 2: /* test2 */
    if (size!=2) abort();
    if (no[0].man!=no[1].man){
      failwith("Dd: binary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    i = Cuddaux_addTest2(res.man->man, &op->table, op, op->commutative,
			 camlidl_cudd_avdd_cmpop,no[0].node,no[1].node);
    break;
  case 3: /* op3 */
    if (size!=3) abort();
    if (no[0].man!=no[1].man || no[0].man!=no[2].man){
      failwith("Dd: ternary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,3)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    res.node = Cuddaux_addApply3(res.man->man,&op->table,op,camlidl_cudd_avdd_op3,no[0].node,no[1].node,no[2].node);
    break;
  case 4: /* exist */
    if (size!=2) abort();
    if (no[0].man!=no[1].man){
      failwith("Dd: binary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0 ||
	alloctable(res.man->man,op2,2)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    res.node = Cuddaux_addAbstract(res.man->man, &op->table, &op2->table, op2,
				   camlidl_cudd_avdd_op2,no[1].node,no[0].node);
    break;
  case 5: /* existop1 */
    if (size!=2) abort();
    if (no[0].man!=no[1].man){
      failwith("Dd: binary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0 ||
	alloctable(res.man->man,op2,2)==0 ||
	alloctable(res.man->man,op1,1)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    res.node = Cuddaux_addApplyAbstract(res.man->man, &op->table, &op2->table, &op1->table,
					op2, op1,
					camlidl_cudd_avdd_op2,camlidl_cudd_avdd_op1,
					no[1].node,no[0].node);
    break;
  case 6: /* existand */
    if (size!=3) abort();
    if (no[0].man!=no[1].man || no[0].man!=no[2].man){
      failwith("Dd: ternary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0 ||
	alloctable(res.man->man,op2,2)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    {
      cuddauxType type = Type_val(op->ddtype,op->closure);
      DdNode* background = cuddauxUniqueType(op->ddtype==2,res.man->man,&type);
      if (background==NULL)
	goto camlidl_cudd_avdd_mapop_end;
      cuddRef(background);
      res.node = Cuddaux_addBddAndAbstract(res.man->man, &op->table, &op2->table,
					   op2, camlidl_cudd_avdd_op2,
					   no[1].node,no[2].node,no[0].node,background);
      cuddDeref(background);
    }
    break;
  case 7: /* existandapply */
    if (size!=3) abort();
    if (no[0].man!=no[1].man || no[0].man!=no[2].man){
      failwith("Dd: ternary function called with nodes belonging to different managers !");
    }
    if (alloctable(res.man->man,op,2)==0 ||
	alloctable(res.man->man,op2,2)==0 ||
	alloctable(res.man->man,op1,1)==0){
      goto camlidl_cudd_avdd_mapop_end;
    }
    {
      cuddauxType type = Type_val(op->ddtype,op->closure);
      DdNode* background = cuddauxUniqueType(op->ddtype==2,res.man->man,&type);
      if (background==NULL)
	goto camlidl_cudd_avdd_mapop_end;
      cuddRef(background);
      res.node = Cuddaux_addApplyBddAndAbstract(res.man->man, &op->table, &op2->table, &op1->table,
						op2, op1,
						camlidl_cudd_avdd_op2,camlidl_cudd_avdd_op1,
						no[1].node,no[2].node,no[0].node,background);
      cuddDeref(background);
    }
    break;
  }
 camlidl_cudd_avdd_mapop_end:
  if (res.node) cuddRef(res.node);
  freetable_auto(op);
  if (op2 != NULL) freetable_auto(op2);
  if (op1 != NULL) freetable_auto(op1);
  if (res.node) cuddDeref(res.node);
  if (camlidl_cudd_avdd_op_exn!=Val_unit){
    Cudd_ClearErrorCode(res.man->man);
    assert(Is_exception_result(camlidl_cudd_avdd_op_exn));
    caml_raise(Extract_exception(camlidl_cudd_avdd_op_exn));
  }

  switch (op->optype){
  case 2:
    /* Force the exception if error */
    v_res = (i==-1) ? camlidl_cudd_node_c2ml(&res) : Val_bool(i);
      break;
  default:
    v_res = camlidl_cudd_node_c2ml(&res);
    break;
  }
  CAMLreturn(v_res);
}

/* ========================================================================= */
/* Terminal cases of user operations */
/* ========================================================================= */

static
DdNode* camlidl_cudd_mapop_result(int ddtype, DdManager* man, value _v_val)
{
  DdNode* res;

  if (Is_exception_result(_v_val)){
    camlidl_cudd_avdd_op_exn = _v_val;
    res = NULL;
  }
  else {
    cuddauxType type = Type_val(ddtype,_v_val);
    res = cuddauxUniqueType(ddtype==2,man,&type);
  }
  return res;
}

DdNode* camlidl_cudd_avdd_op1(DdManager* man, DDAUX_IDOP ptr, DdNode* f)
{
  CAMLparam0();
  CAMLlocal2(_v_f,_v_val);
  DdNode *res;

  assert (f->ref>=1);
  if (cuddIsConstant(f)){
    cuddauxType type;
    camlidl_cudd_op_t* op = (camlidl_cudd_op_t*)ptr;

    _v_f = Val_DdNode(op->ddtype,f);
    _v_val = caml_callback_exn(op->closure, _v_f);
    res = camlidl_cudd_mapop_result(op->ddtype,man,_v_val);
  }
  else {
    res = NULL;
  }
  CAMLreturnT(DdNode*,res);
}

DdNode* camlidl_cudd_avdd_op2(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G)
{
  CAMLparam0();
  CAMLlocal3(_v_F,_v_G,_v_val);
  DdNode *res;
  node__t noF,noG;
  camlidl_cudd_op_t* op = (camlidl_cudd_op_t*)ptr;

  res = NULL;
  if (op->idempotent && F==G) {
    res = F;
    goto camlidl_cudd_avdd_op2_exit;
  }
  if (!cuddIsConstant(F) && !cuddIsConstant(G)){
    goto camlidl_cudd_avdd_op2_exit;
  }
  if (op->special != Val_unit &&
      (cuddIsConstant(F) || cuddIsConstant(G))){
    noF.man = op->man; noF.node = F;
    noG.man = op->man; noG.node = G;
    _v_F = camlidl_cudd_node_c2ml(&noF);
    _v_G = camlidl_cudd_node_c2ml(&noG);
    _v_val = caml_callback2_exn(op->special,_v_F,_v_G);
    if (Is_exception_result(_v_val)){
      camlidl_cudd_avdd_op_exn = _v_val;
      goto camlidl_cudd_avdd_op2_exit;
    }
    else if (Is_block(_v_val)){
      node__t no;
      _v_val = Field(_v_val,0);
      camlidl_cudd_node_ml2c(_v_val,&no);
      res = no.node;
      if (op->man != no.man) caml_failwith("Custom.map_op2: the special function returned a diagram on a different manager !");
      goto camlidl_cudd_avdd_op2_exit;
    }
  }
  if (cuddIsConstant(F) && cuddIsConstant(G)){
    switch (op->ddtype){
    case 0:
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
      break;
    case 1:
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
      break;
    case 2:
      _v_F = cuddauxCamlV(F);
      _v_G = cuddauxCamlV(G);
      break;
    default: abort();
    }
    _v_val = caml_callback2_exn(op->closure, _v_F, _v_G);
  camlidl_cudd_avdd_op2_end:
    res = camlidl_cudd_mapop_result(op->ddtype,man,_v_val);
  }
  camlidl_cudd_avdd_op2_exit:
  CAMLreturnT(DdNode*,res);
}

DdNode* camlidl_cudd_avdd_cmpop(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G)
{
  CAMLparam0();
  CAMLlocal3(_v_F,_v_G,_v_val);
  DdNode *res;
  node__t noF,noG;
  camlidl_cudd_op_t* op = (camlidl_cudd_op_t*)ptr;

  res = NULL;
  if (op->idempotent && F==G) {
    res = DD_ONE(man);
    goto camlidl_cudd_avdd_cmp_exit;
  }
  if (!cuddIsConstant(F) && !cuddIsConstant(G)){
    goto camlidl_cudd_avdd_cmp_exit;
  }
  if (op->special != Val_unit &&
      (cuddIsConstant(F) || cuddIsConstant(G))){
    noF.man = op->man; noF.node = F;
    noG.man = op->man; noG.node = G;
    _v_F = camlidl_cudd_node_c2ml(&noF);
    _v_G = camlidl_cudd_node_c2ml(&noG);
    _v_val = caml_callback2_exn(op->special,_v_F,_v_G);
    if (Is_exception_result(_v_val)){
      goto camlidl_cudd_avdd_cmp_end;
    }
    else if (Is_block(_v_val)){
      _v_val = Field(_v_val,0);
      goto camlidl_cudd_avdd_cmp_end;
    }
  }
  if (cuddIsConstant(F) && cuddIsConstant(G)){
    switch (op->ddtype){
    case 0:
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
      break;
    case 1:
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
      break;
    case 2:
      _v_F = cuddauxCamlV(F);
      _v_G = cuddauxCamlV(G);
      break;
    default: abort();
    }
    _v_val = caml_callback2_exn(op->closure, _v_F, _v_G);
  camlidl_cudd_avdd_cmp_end:
    if (Is_exception_result(_v_val)){
      camlidl_cudd_avdd_op_exn = _v_val;
    }
    else {
      DdNode* one = DD_ONE(man);
      res = (Bool_val(_v_val) ? one : Cudd_Not(one));
    }
  }
 camlidl_cudd_avdd_cmp_exit:
  CAMLreturnT(DdNode*,res);
}

DdNode* camlidl_cudd_avdd_op3(DdManager* man, DDAUX_IDOP ptr, DdNode* F, DdNode* G, DdNode* H)
{
  CAMLparam0();
  CAMLlocal4(_v_F,_v_G,_v_H,_v_val);
  DdNode *res;
  node__t noF,noG,noH;
  camlidl_cudd_op_t* op = (camlidl_cudd_op_t*)ptr;

  res = NULL;
  if (!cuddIsConstant(F) && !cuddIsConstant(G) && !cuddIsConstant(H)){
    goto camlidl_cudd_avdd_op3_exit;
  }
  if (op->special != Val_unit &&
      (cuddIsConstant(F) || cuddIsConstant(G) || cuddIsConstant(H))){
    noF.man = op->man; noF.node = F;
    noG.man = op->man; noG.node = G;
    noH.man = op->man; noH.node = H;
    _v_F = camlidl_cudd_node_c2ml(&noF);
    _v_G = camlidl_cudd_node_c2ml(&noG);
    _v_H = camlidl_cudd_node_c2ml(&noH);
    _v_val = caml_callback3_exn(op->special,_v_F,_v_G,_v_H);
    if (Is_exception_result(_v_val)){
      camlidl_cudd_avdd_op_exn = _v_val;
      goto camlidl_cudd_avdd_op3_exit;
    }
    else if (Is_block(_v_val)){
      node__t no;
      _v_val = Field(_v_val,0);
      camlidl_cudd_node_ml2c(_v_val,&no);
      res = no.node;
      if (op->man != no.man) caml_failwith("Custom.map_op2: the special function returned a diagram on a different manager !");
      goto camlidl_cudd_avdd_op3_exit;
    }
  }
  if (cuddIsConstant(F) && cuddIsConstant(G) && cuddIsConstant(H)) {
    switch (op->ddtype){
    case 0:
      _v_F = copy_double(cuddV(F));
      _v_G = copy_double(cuddV(G));
      _v_H = copy_double(cuddV(H));
      break;
    case 1:
      _v_F = Val_int((int)(cuddV(F)));
      _v_G = Val_int((int)(cuddV(G)));
      _v_H = Val_int((int)(cuddV(H)));
      break;
    case 2:
      _v_F = cuddauxCamlV(F);
      _v_G = cuddauxCamlV(G);
      _v_H = cuddauxCamlV(H);
      break;
    default: abort();
    }
    _v_val = caml_callback3_exn(op->closure, _v_F, _v_G, _v_H);
  camlidl_cudd_avdd_op3_end:
    res = camlidl_cudd_mapop_result(op->ddtype,man,_v_val);
  }
 camlidl_cudd_avdd_op3_exit:
  CAMLreturnT(DdNode*,res);
}
