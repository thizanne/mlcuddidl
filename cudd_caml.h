/* $Id: cudd_caml.h,v 1.3 2005/06/14 14:41:05 bjeannet Exp $ */

/* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  */

#ifndef __DDCOMMON_CAML_H__
#define __DDCOMMON_CAML_H__

#include <stdio.h>
#include <assert.h>

#include "cuddInt.h"
#include "cuddaux.h"

#include "caml/mlvalues.h"

extern int camlidl_cudd_garbage(DdManager* dd, const char* s, void* data);
extern int camlidl_cudd_reordering(DdManager* dd, const char* s, void* data);
value camlidl_cudd_set_gc(value _v_heap, value _v_gc, value _v_reordering);

struct manager__t {
  DdManager* man;
  size_t count;
};
typedef struct manager__t* manager__t;

struct node__t {
  struct manager__t* man; 
  DdNode* node;
};
typedef struct node__t bdd__t;
typedef struct node__t node__t;
typedef struct node__t rdd__t;
typedef struct node__t idd__t;
typedef struct node__t mtbdd__t;

typedef DdNode*(*unop_t)(DdManager *,DdNode *);
typedef DdNode*(*binop_t)(DdManager *,DdNode **,DdNode **);

value camlidl_cudd_manager_c2ml(struct manager__t** man);
void camlidl_cudd_manager_ml2c(value val, struct manager__t** man);
value camlidl_cudd_node_c2ml(struct node__t* no);
void camlidl_cudd_node_ml2c(value val, struct node__t *node);
value camlidl_cudd_bdd_c2ml(struct node__t* bdd);

#define managerRef(x) do { if((x)->count<SIZE_MAX) (x)->count++; } while(0)

#define manager_of_vmanager(x) (*(manager__t*)(Data_custom_val(x)))
#define DdManager_of_vmanager(x) (*(manager__t*)(Data_custom_val(x)))->man

#define node_of_vnode(x) ((node__t*)(Data_custom_val(x)))
#define DdManager_of_vnode(x) ((node__t*)(Data_custom_val(x)))->man->man
#define DdNode_of_vnode(x) ((node__t*)(Data_custom_val(x)))->node

#endif
