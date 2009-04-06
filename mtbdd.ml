(** MTBDDs using a weak hashtable for unique constants *)

open Format


type 'a table = 'a Weakke.Custom.t

type 'a unique = 'a

let print_table = Weakke.Custom.print

let make_table
  ~(hash : 'leaf -> int)
  ~(equal : 'leaf -> 'leaf -> bool)
  :
  'leaf table
  =
  Weakke.Custom.create hash equal 23

external copy_shr : 'a -> 'a = "camlidl_cudd_custom_copy_shr"

let unique (table:'a table) (elt:'a) : 'a unique =
  Weakke.Custom.merge_map table elt copy_shr

let get (leaf:'a unique) : 'a = leaf

type 'a mtbdd =
  'a Vdd.vdd =
  | Leaf of 'a
  | Ite of int * 'a Vdd.t * 'a Vdd.t

include Mapleaf
include Vdd

let dval_u = dval
let is_eval_cst_u = is_eval_cst
let is_ite_cst_u = is_ite_cst
let iter_cube_u = iter_cube
let guard_of_leaf_u = guard_of_leaf
let guard_of_leaf table dd leaf = guard_of_leaf_u dd (unique table leaf)
let leaves_u = leaves
let pick_leaf_u = pick_leaf
let guardleafs_u = guardleafs

let cst_u = cst
let cst cudd table v = cst cudd (unique table v)
