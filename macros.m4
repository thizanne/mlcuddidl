dnl ***************************************************************************
dnl Number of user operations of each type (unary, binary, commutative binary)
dnl ***************************************************************************
dnl
define(NB_USEROPS,5)dnl
dnl
dnl ***************************************************************************
dnl Macros to wrap the transformation DdNode* en node__t
dnl ***************************************************************************
dnl
dnl Unary operations: name of the arg. of type node__t is no,
dnl Binary operations: no1, no2
dnl Ternary operations: no1,no2,no3
dnl
dnl ===========================================================================
dnl Check identity of the managers
dnl ===========================================================================
dnl
define(`CHECK_MAN2',
`if (no1.man!=no2.man){ \
  failwith(\"Dd: binary function called with nodes belonging to different managers !\"); \
}')dnl
dnl
define(`CHECK_MAN3',
`if (no1.man!=no2.man || no1.man!=no3.man){ \
  failwith(\"Dd: ternary function called with nodes belonging to different managers !\"); \
}')dnl
dnl
dnl ===========================================================================
dnl Macros for calling sequences
dnl ===========================================================================
dnl
dnl The number(s) indicates in which order the arguments are applied
dnl
define(`VAL_OF_MAN',
`quote(call, "
Begin_roots1(_v_man);
_res = $1(man);
End_roots();
")')dnl
define(`UNIT_OF_MAN_VAL',
`quote(call, "
Begin_roots1(_v_man);
$1(man,v);
End_roots();
")')dnl
define(`NO_OF_NO',
`quote(call, "
Begin_roots1(_v_no);
_res.man = no.man;
_res.node = $1(no.node);
End_roots();
")')dnl
define(`NO_OF_MAN_NO',
`quote(call, "
Begin_roots1(_v_no);
_res.man = no.man;
_res.node = $1(no.man,no.node);
End_roots();
")')dnl
define(`NO_OF_MAN_NO12',
`quote(call,
"CHECK_MAN2();
Begin_roots2(_v_no1,_v_no2);
_res.man = no1.man;
_res.node = $1(no1.man,no1.node,no2.node);
End_roots();
")')dnl
define(`NO_OF_MAN_NO21',
`quote(call,
"CHECK_MAN2();
Begin_roots2(_v_no1,_v_no2);
_res.man = no1.man;
_res.node = $1(no1.man,no2.node,no1.node);
End_roots();
")')dnl
define(`NO_OF_MAN_NO123',
`quote(call,
"CHECK_MAN3();
Begin_roots3(_v_no1,_v_no2,_v_no3);
_res.man = no1.man;
_res.node = $1(no1.man,no1.node,no2.node,no3.node);
End_roots();
")')dnl
define(`NO_OF_MAN_NO231',
`quote(call,
"CHECK_MAN3();
Begin_roots3(_v_no1,_v_no2,_v_no3);
_res.man = no1.man;
_res.node = $1(no1.man,no2.node,no3.node,no1.node);
End_roots();
")')dnl
dnl
dnl ===========================================================================
dnl Macro for subset or superset approximation functions on BDDs
dnl ===========================================================================
dnl
define(`SUBSUPERSET',
`quote(call,"
Begin_roots1(_v_no);
_res.man=no.man; _res.node = $1(no.man,no.node,nvars,threshold);
End_roots();
")')dnl
dnl
dnl ===========================================================================
dnl Macro for decomposition functions on BDDs
dnl ===========================================================================
dnl
define(`DECOMP',`
quote(MLMLI,"external $1: t -> (t*t) option = \"camlidl_bdd_$1\"")
quote(C,"
value camlidl_bdd_$1(value _v_no)
{
  CAMLparam1(_v_no); CAMLlocal4(_v_res,_v_a,_v_b,_v_pair);
  bdd__t no;
  int res;
  DdNode** tab;
  bdd__t a;
  bdd__t b;

  camlidl_cudd_node_ml2c(_v_no,&no);
  res = $2(no.man,no.node,&tab);
  switch(res){
  case 0:
    failwith(\"Bdd.$1: decomposition function failed (probably CUDD_OUT_OF_MEM)\");
    break;
  case 1:
    _v_res = Val_int(0);
    cuddDeref(tab[0]);		
    free(tab);
    break;
  case 2:
    a.man = b.man = no.man;
    a.node = tab[0];
    b.node = tab[1];
    cuddDeref(a.node);
    _v_a = camlidl_cudd_bdd_c2ml(&a);
    cuddDeref(b.node);
    _v_b = camlidl_cudd_bdd_c2ml(&b);
    _v_pair = alloc_small(0,2);
    Field(_v_pair,0) = _v_a;
    Field(_v_pair,1) = _v_b;
    _v_res = alloc_small(0,1);
    Field(_v_res,0) = _v_pair;
    free(tab);	
    break;
  }
  CAMLreturn(_v_res);
}")
')dnl
dnl
dnl ===========================================================================
dnl Macro for applying unary and binary operations on ADDs
dnl ===========================================================================
dnl
define(`APPLYBINOP',`quote(call,
"CHECK_MAN2();
Begin_roots2(_v_no1,_v_no2);
_res.man = no1.man;
_res.node = Cudd_addApply(no1.man,$1,no1.node,no2.node);
End_roots();
")')dnl
dnl
define(`APPLYUNOP',`quote(call,"
Begin_roots1(_v_no);
_res.man = no.man;
_res.node = Cudd_addMonadicApply(no.man,$1,no.node);
End_roots();
")')dnl
dnl
dnl ***************************************************************************
dnl Macros for user operations on RDDs
dnl ***************************************************************************
dnl
dnl ===========================================================================
dnl Looping macros, taken from GNU m4 manual
dnl ===========================================================================
dnl
dnl example: FORLOOP(`i',1,4,`i, ')
dnl
define(`FORLOOP',`pushdef(`$1', `$2')_FORLOOP(`$1', `$2', `$3', `$4')popdef(`$1')')dnl
define(`_FORLOOP',
       `$4`'ifelse($1, `$3', ,
		   `define(`$1', incr($1))_FORLOOP(`$1', `$2', `$3', `$4')')')dnl
dnl
dnl ===========================================================================
dnl Template for user operations
dnl ===========================================================================
dnl
dnl DEF_USEROP(name of the module,
dnl            macro to convert double into value,
dnl            macro to convert value into double,
dnl            index of the copy)
dnl
define(`DEF_USEROP',`
static DdNode* camlidl_$1_unop_$4(DdManager* man, DdNode* f)
{
  CAMLparam0(); CAMLlocal2(_v_f,_v_val);
  DdNode *res;
  double val;

  if (cuddIsConstant(f)){
    _v_f = $2(cuddV(f));
    _v_val = callback(camlidl_$1_unclosures[$4], _v_f);
    val = $3(_v_val);
    res = cuddUniqueConst(man,val);
  }
  else {
    res = NULL;
  }
  CAMLreturn(res);
}
static DdNode* camlidl_$1_binop_$4(DdManager* man, DdNode** f, DdNode** g)
{
  CAMLparam0(); CAMLlocal3(_v_F,_v_G,_v_val);
  DdNode *F, *G, *res;
  double val;

  F = *f; G = *g;
  if (cuddIsConstant(F) && cuddIsConstant(G)) {
    _v_F = $2(cuddV(F));
    _v_G = $2(cuddV(G));
    _v_val = callback2(camlidl_$1_binclosures[$4], _v_F, _v_G);
    val = $3(_v_val);
    res = cuddUniqueConst(man,val);
  }
  else {
    res = NULL;
  }
  CAMLreturn(res);
}
static DdNode* camlidl_$1_combinop_$4(DdManager* man, DdNode** f, DdNode** g)
{
  CAMLparam0(); CAMLlocal3(_v_F,_v_G,_v_val);
  DdNode *F, *G, *res;
  double val;

  F = *f; G = *g;
  if (cuddIsConstant(F) && cuddIsConstant(G)) {
    _v_F = $2(cuddV(F));
    _v_G = $2(cuddV(G));
    _v_val = callback2(camlidl_$1_combinclosures[$4], _v_F, _v_G);
    val = $3(_v_val);
    res = cuddUniqueConst(man,val);
  }
  else {
    if (F > G) {
      *f = G;
      *g = F;
    }
    res = NULL;
  }
  CAMLreturn(res);
}')dnl
dnl
dnl ===========================================================================
dnl C part of user operation for a module
dnl ===========================================================================
dnl
dnl ALLOCAPPLY_USEROP(name of the module,
dnl            macro to convert double into value,
dnl            macro to convert value into double,
dnl
define(`ALLOCAPPLY_USEROP',`
value camlidl_$1_unclosures[NB_USEROPS];
value camlidl_$1_binclosures[NB_USEROPS];
value camlidl_$1_combinclosures[NB_USEROPS];

FORLOOP(`INDEX',0,decr(NB_USEROPS),`DEF_USEROP($1,$2,$3,INDEX)')

unop_t camlidl_$1_unops[NB_USEROPS] = {
  FORLOOP(`INDEX',0,decr(NB_USEROPS),`&camlidl_$1_unop_`'INDEX, ')
};
binop_t camlidl_$1_binops[NB_USEROPS] = {
  FORLOOP(`INDEX',0,decr(NB_USEROPS),`&camlidl_$1_binop_`'INDEX, ')
};
binop_t camlidl_$1_combinops[NB_USEROPS] = {
  FORLOOP(`INDEX',0,decr(NB_USEROPS),`&camlidl_$1_combinop_`'INDEX, ')
};

int camlidl_$1_nb_unop = 0;
int camlidl_$1_nb_binop = 0;
int camlidl_$1_nb_combinop = 0;

value camlidl_$1_alloc_unop(value _v_closure)
{
  CAMLparam1(_v_closure); CAMLlocal1(_v_res);
  if (camlidl_$1_nb_unop < NB_USEROPS){
    register_global_root(&(camlidl_$1_unclosures[camlidl_$1_nb_unop]));
    camlidl_$1_unclosures[camlidl_$1_nb_unop] = _v_closure;
    _v_res = Val_int(camlidl_$1_nb_unop);
    camlidl_$1_nb_unop++;
  }
  else {
    failwith(\"$1.alloc_unop: maximum number of ops already allocated\");
  }
  CAMLreturn(_v_res);
}
value camlidl_$1_alloc_binop(value _v_closure)
{
  CAMLparam1(_v_closure); CAMLlocal1(_v_res);
  if (camlidl_$1_nb_binop < NB_USEROPS){
    register_global_root(&(camlidl_$1_binclosures[camlidl_$1_nb_binop]));
    camlidl_$1_binclosures[camlidl_$1_nb_binop] = _v_closure;
    _v_res = Val_int(camlidl_$1_nb_binop);
    camlidl_$1_nb_binop++;
  }
  else {
    failwith(\"$1.alloc_binop: maximum number of ops already allocated\");
  }
  CAMLreturn(_v_res);
}
value camlidl_$1_alloc_combinop(value _v_closure)
{
  CAMLparam1(_v_closure); CAMLlocal1(_v_res);
  if (camlidl_$1_nb_combinop < NB_USEROPS){
    register_global_root(&(camlidl_$1_combinclosures[camlidl_$1_nb_combinop]));
    camlidl_$1_combinclosures[camlidl_$1_nb_combinop] = _v_closure;
    _v_res = Val_int(camlidl_$1_nb_combinop);
    camlidl_$1_nb_combinop++;
  }
  else {
    failwith(\"$1.alloc_combinop: maximum number of ops already allocated\");
  }
  CAMLreturn(_v_res);
}
value camlidl_$1_apply_unop(value _v_id, value _v_no)
{
  CAMLparam2(_v_id,_v_no); CAMLlocal1(_v_res);
  node__t no,_res;
  int id;

  id = Int_val(_v_id);
  camlidl_cudd_node_ml2c(_v_no,&no);

  _res.man = no.man;
  _res.node = Cudd_addMonadicApply(no.man,camlidl_$1_unops[id], no.node);
  _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_$1_apply_binop(value _v_id, value _v_no1, value _v_no2)
{
  CAMLparam3(_v_id,_v_no1,_v_no2); CAMLlocal1(_v_res);
  node__t no1,no2,_res;
  int id;

  id = Int_val(_v_id);
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  CHECK_MAN2;
  _res.man = no1.man;
  _res.node = Cudd_addApply(no1.man,camlidl_$1_binops[id], no1.node, no2.node);
  _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}
value camlidl_$1_apply_combinop(value _v_id, value _v_no1, value _v_no2)
{
  CAMLparam3(_v_id,_v_no1,_v_no2); CAMLlocal1(_v_res);
  node__t no1,no2,_res;
  int id;

  id = Int_val(_v_id);
  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  CHECK_MAN2;
  _res.man = no1.man;
  _res.node = Cudd_addApply(no1.man,camlidl_$1_combinops[Int_val(_v_id)],no1.node,no2.node);
  _v_res = camlidl_cudd_node_c2ml(&_res);
  CAMLreturn(_v_res);
}')dnl
dnl
dnl ***************************************************************************
dnl Macros for matrix multiplication on ADDs
dnl ***************************************************************************
dnl
define(`MATMUL',`
value $1(value _v_array, value _v_no1, value _v_no2)
{
  CAMLparam3(_v_array,_v_no1,_v_no2); CAMLlocal1(_v_res);
  int i,size;
  DdNode** array;
  node__t no,no1,no2;

  camlidl_cudd_node_ml2c(_v_no1,&no1);
  camlidl_cudd_node_ml2c(_v_no2,&no2);
  CHECK_MAN2();
  size = Wosize_val(_v_array);
  array = malloc(size * sizeof(DdNode*));
  for (i=0; i<size; i++){
    value _v_index = Field(_v_array,i);
    int index = Int_val(_v_index);
    array[i] = Cudd_bddIthVar(no1.man, index);
  }
  no.man = no1.man;
  no.node = $2(no1.man,no1.node,no2.node,array,size);
  _v_res = camlidl_cudd_node_c2ml(&no);
  free(array);
  CAMLreturn(_v_res);
}')dnl
dnl
