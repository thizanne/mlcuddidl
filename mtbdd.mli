(* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  *)

(** MTBDDs using a weak hashtable for unique constants *)

type 'a unique
  (** Type of unique representants of MTBDD leaves *)

type 'a t = 'a unique Vdd.t
  (** Type of MTBDDs.

      Objects of this type contains both the top node of the MTBDD
      and the manager to which the node belongs. The manager can
      be retrieved with {!manager}. Objects of this type are
      automatically garbage collected.  *)

type 'a table = 'a Weakke.Custom.t
  (** Hashtable to manage unique constants *)

val print_table :
  ?first:(unit, Format.formatter, unit) format ->
  ?sep:(unit, Format.formatter, unit) format ->
  ?last:(unit, Format.formatter, unit) format ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter -> 'a table -> unit

val make_table : hash:('a -> int) -> equal:('a -> 'a -> bool) -> 'a table
  (** Building a table *)

val unique : 'a table -> 'a -> 'a unique
  (** Building a unique constant *)
val get : 'a unique -> 'a 
  (** Type conversion (no computation) *)

(** Public type for exploring the abstract type [t] *)
type 'a mtbdd =
  | Leaf of 'a unique      (** Terminal value *)
  | Ite of int * 'a t * 'a t (** Decision on CUDD variable *)

(** We refer to the modules {!Rdd} and {!Vdd} for the description
    of the interface. *)

(* ====================================================== *)
(** {2 Extractors} *)
(* ====================================================== *)

external manager : 'a t -> Man.v Man.t = "camlidl_cudd_bdd_manager"
  (** Returns the manager associated to the MTBDD *)

external is_cst : 'a t -> bool = "camlidl_cudd_bdd_is_cst"
  (** Is the MTBDD constant ? *)

external topvar : 'a t -> int = "camlidl_cudd_bdd_topvar"
  (** Returns the index of the top node of the MTBDD (65535 for a
      constant MTBDD) *)

external dthen : 'a t -> 'a t = "camlidl_cudd_rdd_dthen"
  (** Returns the positive subnode of the MTBDD *)

external delse : 'a t -> 'a t = "camlidl_cudd_rdd_delse"
  (** Returns the negative subnode of the MTBDD *)

external cofactors : int -> 'a t -> 'a t * 'a t = "camlidl_cudd_rdd_cofactors"
  (** Returns the positive and negative cofactor of the MTBDD wrt
      the variable *)

external cofactor : 'a t -> Man.v Bdd.t -> 'a t = "camlidl_cudd_rdd_cofactor"
  (** [cofactor mtbdd cube] evaluates [mtbbdd] on the cube [cube] *)

val dval_u : 'a t -> 'a unique
val dval : 'a t -> 'a
  (** Returns the value of the assumed constant MTBDD *)

val inspect : 'a t -> 'a mtbdd
  (** Decompose the MTBDD *)

(* ====================================================== *)
(** {2 Supports} *)
(* ====================================================== *)

external support : 'a t -> Man.v Bdd.t = "camlidl_cudd_bdd_support"
external supportsize : 'a t -> int = "camlidl_cudd_bdd_supportsize"
external is_var_in : int -> 'a t -> bool = "camlidl_cudd_bdd_is_var_in"
external vectorsupport : 'a t array -> Man.v Bdd.t = "camlidl_cudd_bdd_vectorsupport"
external vectorsupport2 : Man.v Bdd.t array -> 'a t array -> Man.v Bdd.t = "camlidl_cudd_rdd_vectorsupport2"

(* ====================================================== *)
(** {2 Classical operations} *)
(* ====================================================== *)

val cst_u : Man.v Man.t -> 'a unique -> 'a t
val cst : Man.v Man.t -> 'a table -> 'a -> 'a t

external ite : Man.v Bdd.t -> 'a t -> 'a t -> 'a t = "camlidl_cudd_rdd_ite"
external ite_cst : Man.v Bdd.t -> 'a t -> 'a t -> 'a t option = "camlidl_cudd_rdd_ite_cst"
external eval_cst : 'a t -> Man.v Bdd.t -> 'a t option = "camlidl_cudd_rdd_eval_cst"
external compose : int -> Man.v Bdd.t -> 'a t -> 'a t = "camlidl_cudd_rdd_compose"
external vectorcompose: Man.v Bdd.t array -> 'a t -> 'a t = "camlidl_cudd_rdd_vectorcompose"

(* ====================================================== *)
(** {2 Logical tests} *)
(* ====================================================== *)

external is_equal : 'a t -> 'a t -> bool = "camlidl_cudd_bdd_is_equal"
external is_equal_when : 'a t -> 'a t -> Man.v Bdd.t -> bool = "camlidl_cudd_bdd_is_equal_when"


val is_eval_cst_u : 'a t -> Man.v Bdd.t -> 'a unique option
val is_ite_cst_u : Man.v Bdd.t -> 'a t -> 'a t -> 'a unique option
val is_eval_cst : 'a t -> Man.v Bdd.t -> 'a option
val is_ite_cst : Man.v Bdd.t -> 'a t -> 'a t -> 'a option

(* ====================================================== *)
(** {2 Structural information} *)
(* ====================================================== *)

external size : 'a t -> int = "camlidl_cudd_bdd_size"
external nbpaths : 'a t -> float = "camlidl_cudd_bdd_nbpaths"
external nbnonzeropaths : 'a t -> float = "camlidl_cudd_bdd_nbtruepaths"
external nbminterms : int -> 'a t -> float = "camlidl_cudd_bdd_nbminterms"
external density : int -> 'a t -> float = "camlidl_cudd_bdd_density"
external nbleaves : 'a t -> int = "camlidl_cudd_rdd_nbleaves"

(* ====================================================== *)
(** {2 Variable mapping} *)
(* ====================================================== *)

external varmap : 'a t -> 'a t = "camlidl_cudd_rdd_varmap"
external permute : 'a t -> int array -> 'a t = "camlidl_cudd_rdd_permute"

(* ====================================================== *)
(** {2 Iterators} *)
(* ====================================================== *)

val iter_cube_u : (Man.tbool array -> 'a unique -> unit) -> 'a t -> unit
val iter_cube : (Man.tbool array -> 'a -> unit) -> 'a t -> unit


external iter_node: ('a t -> unit) -> 'a t -> unit = "camlidl_cudd_iter_node"

(* ====================================================== *)
(** {2 Leaves and guards} *)
(* ====================================================== *)

external guard_of_node : 'a t -> 'a t -> Man.v Bdd.t = "camlidl_cudd_rdd_guard_of_node"
external guard_of_nonbackground : 'a t -> Man.v Bdd.t = "camlidl_cudd_rdd_guard_of_nonbackground"
val nodes_below_level: ?max:int -> 'a t -> int option -> 'a t array 

(** Guard of the given leaf *)
val guard_of_leaf_u : 'a t -> 'a unique -> Man.v Bdd.t
val guard_of_leaf : 'a table -> 'a t -> 'a -> Man.v Bdd.t

(** Returns the set of leaf values (excluding the background value) *)
val leaves_u: 'a t -> 'a unique array
val leaves: 'a t -> 'a array

(** Picks (but not randomly) a non background leaf. Return [None] if the only leaf is the background leaf. *)
val pick_leaf_u : 'a t -> 'a unique
val pick_leaf : 'a t -> 'a

(** Returns the set of leaf values together with their guard in the RDD *)
val guardleafs_u : 'a t -> (Man.v Bdd.t * 'a unique) array
val guardleafs : 'a t -> (Man.v Bdd.t * 'a) array

(* ====================================================== *)
(** {2 Minimizations} *)
(* ====================================================== *)

external constrain: 'a t -> Man.v Bdd.t -> 'a t = "camlidl_cudd_rdd_constrain"
external tdconstrain: 'a t -> Man.v Bdd.t -> 'a t = "camlidl_cudd_rdd_tdconstrain"
external restrict: 'a t -> Man.v Bdd.t -> 'a t = "camlidl_cudd_rdd_restrict"
external tdrestrict : 'a t -> Man.v Bdd.t -> 'a t = "camlidl_cudd_rdd_tdrestrict"

(* ====================================================== *)
(** {2 Conversions} *)
(* ====================================================== *)
(* ====================================================== *)
(** {2 User operations} *)
(* ====================================================== *)
(* ====================================================== *)
(** {3 By decomposition into guards and leafs} *)
(* ====================================================== *)
(**
   Be cautious: here the background leaf is used as a special value,
   and should not be used for ordinary purpose.
*)

val combineretractive : Man.v Bdd.t * 'a unique -> 'a t -> 'a t
val combineexpansive :
  default:'a t ->
  merge:('a t -> 'b t -> 'c t) -> Man.v Bdd.t * 'a unique -> 'b t -> 'c t
val combineleaf1 :
  default:'a t ->
  combine:(Man.v Bdd.t * 'c -> 'a t -> 'a t) ->
  (Man.v Bdd.t -> 'b unique -> Man.v Bdd.t * 'c) -> 'b t -> 'a t
val retractivemapleaf1 :
  default:'a t ->
  (Man.v Bdd.t -> 'b unique -> Man.v Bdd.t * 'a unique) -> 'b t -> 'a t
val expansivemapleaf1 :
  default:'a t ->
  merge:('a t -> 'a t -> 'a t) ->
  (Man.v Bdd.t -> 'b unique -> Man.v Bdd.t * 'a unique) -> 'b t -> 'a t
val mapleaf1 : ('a unique -> 'b unique) -> 'a t -> 'b t
val combineleaf2 :
  default:'a t ->
  combine:(Man.v Bdd.t * 'd -> 'a t -> 'a t) ->
  (Man.v Bdd.t -> 'b unique -> 'c unique -> Man.v Bdd.t * 'd) ->
  'b t -> 'c t -> 'a t
val retractivemapleaf2 :
  default:'a t ->
  (Man.v Bdd.t -> 'b unique -> 'c unique -> Man.v Bdd.t * 'a unique) ->
  'b t -> 'c t -> 'a t
val expansivemapleaf2 :
  default:'a t ->
  merge:('a t -> 'a t -> 'a t) ->
  (Man.v Bdd.t -> 'b unique -> 'c unique -> Man.v Bdd.t * 'a unique) ->
  'b t -> 'c t -> 'a t
val mapleaf2 : ('a unique -> 'b unique -> 'c unique) -> 'a t -> 'b t -> 'c t
val combineleaf_array :
  default:'a t ->
  combine:(Man.v Bdd.t * 'c -> 'a t -> 'a t) ->
  tabsorbant:('b -> bool) option array ->
  (Man.v Bdd.t -> 'b unique array -> Man.v Bdd.t * 'c) -> 'b t array -> 'a t
val combineleaf1_array :
  default:'a t ->
  combine:(Man.v Bdd.t * 'd -> 'a t -> 'a t) ->
  ?absorbant:('b -> bool) ->
  tabsorbant:('c -> bool) option array ->
  (Man.v Bdd.t -> 'b unique -> 'c unique array -> Man.v Bdd.t * 'd) ->
  'b t -> 'c t array -> 'a t
val combineleaf2_array :
  default:'a t ->
  combine:(Man.v Bdd.t * 'e -> 'a t -> 'a t) ->
  ?absorbant1:('b -> bool) ->
  ?absorbant2:('c -> bool) ->
  tabsorbant:('d -> bool) option array ->
  (Man.v Bdd.t -> 'b unique -> 'c unique -> 'd unique array -> Man.v Bdd.t * 'e) ->
  'b t -> 'c t -> 'd t array -> 'a t

(* ====================================================== *)
(** {3 By using CUDD cache} *)
(* ====================================================== *)

(** {4 Types} *)

type ('a, 'b) op1 = ('a unique, 'b unique) Vdd.op1
type ('a, 'b, 'c) op2 = ('a unique, 'b unique, 'c unique) Vdd.op2
type ('a, 'b) test2 = ('a unique, 'b unique) Vdd.test2
type ('a, 'b, 'c, 'd) op3 = ('a unique, 'b unique, 'c unique, 'd unique) Vdd.op3
type ('a, 'b) exist = ('a unique, 'b) Vdd.exist
type ('a, 'b, 'c, 'd) existop1 = ('a unique, 'b unique, 'c, 'd) Vdd.existop1
type ('a, 'b) existand = ('a unique, 'b) Vdd.existand
type ('a, 'b, 'c, 'd) existandop1 = ('a unique, 'b unique, 'c, 'd) Vdd.existandop1
type ('a, 'b) vectorcomposeop1 = ('a unique, 'b) Vdd.vectorcomposeop1
type auto = Vdd.auto
type user = Vdd.user
type 'a local = 'a Vdd.local
type global = Vdd.global
type 'a cache = 'a Vdd.cache
type ('a, 'b) op = ('a, 'b) Vdd.op
type ('a, 'b) mexist =
    [ `Fun of ('a t -> 'a t -> 'a t option) option * ('a unique -> 'a unique -> 'a unique)
    | `Op of (('a, 'a, 'a) op2, 'b) op ]
type ('a, 'b, 'c) mop1 = [ `Fun of 'a unique -> 'b unique | `Op of (('a, 'b) op1, 'c) op ]
val global : global cache
val auto : auto local cache
val user : user local cache

(** {4 Registering operations} *)

val register_op1 :
  cachetyp:'c cache -> ('a unique -> 'b unique) -> (('a,'b) op1, 'c) op
val register_op2 :
  cachetyp:'d cache ->
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('a t -> 'b t -> 'c t option) ->
  ('a unique -> 'b unique -> 'c unique) -> (('a,'b,'c) op2, 'd) op
val register_test2 :
  cachetyp:'c cache ->
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('a t -> 'b t -> bool option) ->
 ('a unique -> 'b unique -> bool) -> (('a,'b) test2, 'c) op
val register_op3 :
  cachetyp:'e  local cache ->
  ?special:('a t -> 'b t -> 'c t -> 'd t option) ->
  ('a unique -> 'b unique -> 'c unique -> 'd unique) -> (('a,'b,'c,'d) op3, 'e local) op
val register_exist :
  cachetyp:'c cache -> (('a,'a,'a) op2,'b) op -> (('a,'b) exist, 'c) op
val register_existop1 :
  cachetyp:'e cache ->
  (('a, 'b) op1, 'd) op ->
  (('b, 'b, 'b) op2, 'c) op ->
  (('a,'b,'d,'c) existop1, 'e) op
val register_existand :
  cachetyp:'c local cache ->
  bottom:'a unique ->
  (('a, 'a, 'a) op2, 'b) op -> (('a,'b) existand, 'c local) op
val register_existandop1 :
  cachetyp:'e local cache ->
  bottom:'b unique ->
  (('a, 'b) op1, 'd) op ->
  (('b, 'b, 'b) op2, 'c) op ->
  (('a,'b,'d,'c) existandop1, 'e local) op

(** {4 Flushing cache} *)

val op2_of_exist : (('a,'b) exist, 'c) op -> (('a,'a,'a) op2, 'b) op
val op2_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('b,'b,'b) op2, 'd) op
val op2_of_existand : (('a,'b) existand, 'c local) op -> (('a,'a,'a) op2, 'b) op
val op2_of_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> (('b,'b,'b) op2, 'd) op
val op1_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('a,'b) op1, 'c) op
val op1_of_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> (('a,'b) op1, 'c) op

val flush_op : ('a, user local) op -> unit
val flush_allop : unit -> unit

(** {4 Applying operations} *)

val apply_op1 : (('a,'b) op1, 'c) op -> 'a t -> 'b t
val apply_op2 : (('a,'b,'c) op2, 'd) op -> 'a t -> 'b t -> 'c t
val apply_test2 : (('a,'b) test2, 'c) op -> 'a t -> 'b t -> bool
val apply_op3 :
  (('a,'b,'c,'d) op3, 'e local) op ->
  'a t -> 'b t -> 'c t -> 'd t
val apply_exist : (('a,'b) exist, 'c) op -> supp:(Man.v Bdd.t) -> 'a t -> 'a t
val apply_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> supp:(Man.v Bdd.t) -> 'a t -> 'b t
val apply_existand : (('a,'b) existand, 'c local) op -> supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'a t -> 'a t
val apply_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'a t -> 'b t

(** {4 Map functions (based on automatic user caches)} *)

val map_op1 :
  ('a unique -> 'b unique) -> 'a t -> 'b t
val map_op2 :
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('a t -> 'b t -> 'c t option) ->
  ('a unique -> 'b unique -> 'c unique) ->
  'a t -> 'b t -> 'c t
val map_test2 :
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('a t -> 'b t -> bool option) ->
  ('a unique -> 'b unique -> bool) ->
  'a t -> 'b t -> bool
val map_op3 :
  ?special:('a t -> 'b t -> 'c t -> 'd t option) ->
  ('a unique -> 'b unique -> 'c unique -> 'd unique) ->
  'a t -> 'b t -> 'c t -> 'd t

val map_exist :
  ('a, 'b) mexist ->
  supp:(Man.v Bdd.t) -> 'a t -> 'a t
val map_existop1 :
  ('a,'b,'c) mop1 -> ('b,'d) mexist ->
  supp:(Man.v Bdd.t) -> 'a t -> 'b t
val map_existand :
  bottom:'a unique ->
  ('a, 'b) mexist ->
  supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'a t -> 'a t
val map_existandop1 :
  bottom:'b unique ->
  ('a,'b,'c) mop1 -> ('b,'d) mexist ->
  supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'a t -> 'b t

(* ====================================================== *)
(** {2 Miscellaneous} *)
(* ====================================================== *)

external transfer : 'a t -> Man.v Man.t -> 'a t = "camlidl_cudd_rdd_transfer"

(* ====================================================== *)
(** {2 Printing} *)
(* ====================================================== *)

val print__minterm:
  (Format.formatter -> 'a -> unit) ->
  Format.formatter -> 'a t -> unit
val print_minterm:
  (Format.formatter -> int -> unit) ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter -> 'a t -> unit
val print:
  (Format.formatter -> Man.v Bdd.t -> unit) ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter -> 'a t -> unit

