/* Conversion of datatypes and common functions */

/* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  */

#ifndef __CUDD_CAML_H__
#define __CUDD_CAML_H__

#include <stdio.h>
#include <assert.h>

#include "cuddInt.h"
#include "cuddauxInt.h"

#include "caml/mlvalues.h"

extern int camlidl_cudd_custom_hook(DdManager* dd, const char* s, void* data);
extern int camlidl_cudd_garbage(DdManager* dd, const char* s, void* data);
extern int camlidl_cudd_reordering(DdManager* dd, const char* s, void* data);
value camlidl_cudd_set_gc(value _v_heap, value _v_gc, value _v_reordering);

struct man__t {
  DdManager* man;
  size_t count;
};
typedef struct man__t* man__t;
typedef struct man__t* man__dt;
typedef struct man__t* man__vt;

struct node__t {
  struct man__t* man; 
  DdNode* node;
};
typedef struct node__t bdd__t;
typedef struct node__t bdd__dt;
typedef struct node__t bdd__vt;
typedef struct node__t node__t;
typedef struct node__t rdd__t;
typedef struct node__t idd__t;
typedef struct node__t vdd__t;
typedef value vdd__leaf;
typedef struct node__t mtbdd__t;

typedef DdNode*(*unop_t)(DdManager *,DdNode *);
typedef DdNode*(*binop_t)(DdManager *,DdNode **,DdNode **);

value camlidl_cudd_man_c2ml(struct man__t** man);
void camlidl_cudd_man_ml2c(value val, struct man__t** man);
value camlidl_cudd_node_c2ml(struct node__t* no);
void camlidl_cudd_node_ml2c(value val, struct node__t *node);
value camlidl_cudd_bdd_c2ml(struct node__t* bdd);
#define camlidl_cudd_leaf_ml2c(val, leaf) do { *leaf = val; } while(0);
#define camlidl_cudd_leaf_c2ml(leaf) (*leaf)

#define managerRef(x) do { if((x)->count<SIZE_MAX) (x)->count++; } while(0)

#define man_of_vmanager(x) (*(man__t*)(Data_custom_val(x)))
#define DdManager_of_vmanager(x) (*(man__t*)(Data_custom_val(x)))->man

#define node_of_vnode(x) ((node__t*)(Data_custom_val(x)))
#define DdManager_of_vnode(x) ((node__t*)(Data_custom_val(x)))->man->man
#define DdNode_of_vnode(x) ((node__t*)(Data_custom_val(x)))->node

cuddauxType Type_val(int ddtype, value val);
value Val_type(int ddtype, cuddauxType* type);
value Val_DdNode(int ddtype, DdNode* node);

#endif
