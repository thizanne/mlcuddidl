(* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  *)

(** Custom operations for ADDs (Internal) *)

(** Internal module, used by modules {!Idd}, {!Rdd} and {!Vdd} *)

(*  ********************************************************************** *)
(** {2 Types of registered operations} *)
(*  ********************************************************************** *)

(*  ---------------------------------------------------------------------- *)
(** {3 Operations on leaves of MTBDDs} *)
(*  ---------------------------------------------------------------------- *)

type ('a,'b) op1
  (** Type of unary operations ['a -> 'b] *)
type ('a,'b,'c) op2
  (** Type of binary operations ['a -> 'b -> 'c] *)
type ('a,'b) test2
  (** Type of binary tests ['a -> 'b -> bool] *)
type ('a,'b,'c,'d) op3
  (** Type of ternary operations ['a -> 'b -> 'c -> 'd] *)
type ('a,'b) exist
  (** Type of quantification operation [supp -> 'a -> 'a].  The leaf
      operation [op:'a -> 'a -> 'a] is assumed to be commutative
      and idempotent ([op f f=f]).  When a Boolean variable in [supp] is
      quantified out, [op:'a -> 'a -> 'a] is used to combine the two branch of the diagram.
  *)
type ('a,'b,'c,'d) existop1
  (** Type of quantification and op1 operation.  The leaf
      operation [op:'b -> 'b -> 'b] is assumed to be commutative
      and idempotent ([op f f=f]).
      [existop1 op op1 supp bdd] is equivalent to
      [exist supp (op1 f)].
  *)
type ('a,'b) existand
  (** Type of combined quantification and and operation. The leaf
      operation [op:'a -> 'a -> 'a] is assumed to be commutative, idempotent ([op f f=f]).
      and also [op f bottom = f].
      [existand ~bottom op supp bdd f] is equivalent to [exist supp (ite bdd f bottom)].
  *)
type ('a,'b,'c,'d) existandop1
  (** Type of combined quantification, and and op1 operation. The leaf
      operation [op:'b -> 'b -> 'b] is assumed to be commutative, idempotent ([op f f=f]), and
      also [op f bottom = f].
      [existandop1 ~bottom op op1 supp bdd f] is equivalent to
      [exist supp (ite bdd (op1 f) bottom))]. *)
type ('a,'b) vectorcomposeop1

(*  ---------------------------------------------------------------------- *)
(** {3 Caching policy} *)
(*  ---------------------------------------------------------------------- *)

(** {4 Local cache} *)

type auto
  (** The local table is cleared automatically at the end on the
      operation on MTBDDs.  Hence, there is no reuse between two
      calls to the same MTBDD operation.

      Default option, as there is no danger to do tricky
      errors.
  *)
type user
  (** It is up to the user to clear regularly the local
      table. Forgetting to do so will prevent garbage collection
      of nodes stored in the table, which can only grow.

      The OCaml closure defining the function should not use free
      variables that may be modified and so impact its result:
      they would act as hidden parameters that are not taken into
      account in the cache.

      If such hidden parameters are modified, the cache should be cleared with {!flush_cache}
  *)

type 'a local
  (** Local cache (hashtable) policy, where ['a] is either [auto] or [user]. *)

(** {4 Global cache} *)

type global
  (** The operation on MTBDDs is memoized in the global cache.

      Same remark as for [user local] concerning free
      variables.acting as hidden parameters. If hidden parameters
      are modified, the global cache should be cleared with
      {!Man.flush_cache}.
  *)

(** {4 Caching policy} *)

type 'a cache
  (** Caching policy, where ['a] is either ['a local] or [global]. *)

(*  ---------------------------------------------------------------------- *)
(** {3 Type of registered operations} *)
(*  ---------------------------------------------------------------------- *)

type ('a,'b) op
  (** ['a] indicates the type and arity of the corresponding operation on leaves
      (one of [('a,'b) op1, ('a,'b,'c) op2, ...])

      ['b] indicates the caching policy.
  **)

type ('a, 'add, 'b) mexist = [
  | `Fun of ('add -> 'add -> 'add option) option * ('a -> 'a -> 'a)
  | `Op of (('a, 'a, 'a) op2, 'b) op
]

type ('a, 'b, 'c) mop1 = [ 
  | `Fun of 'a -> 'b 
  | `Op of (('a, 'b) op1, 'c) op 
]

(*  ********************************************************************** *)
(** {2 Registering and managing operations} *)
(*  ********************************************************************** *)

val global : global cache
val auto : auto local cache
val user : user local cache

val register_op1 :
  ddtyp:int -> cachetyp:'a cache -> ('b -> 'c) -> (('b, 'c) op1, 'a) op
val register_op2 :
  ddtyp:int ->
  cachetyp:'a cache ->
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('bdd -> 'cdd -> 'ddd option) ->
  ('b -> 'c -> 'd) -> (('b, 'c, 'd) op2, 'a) op
val register_test2 :
  ddtyp:int ->
  cachetyp:'a cache ->
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('bdd -> 'cdd -> bool option) ->
  ('b -> 'c -> bool) -> (('b, 'c) test2, 'a) op
val register_op3 :
  ddtyp:int ->
  cachetyp:'a local cache ->
  ?special:('bdd -> 'cdd -> 'ddd -> 'edd option) ->
  ('b -> 'c -> 'd -> 'e) -> (('b, 'c, 'd, 'e) op3, 'a local) op
val register_exist :
  ddtyp:int ->
  cachetyp:'a cache -> (('b, 'b, 'b) op2, 'c) op -> (('b, 'c) exist, 'a) op
val register_existop1 :
  ddtyp:int ->
  cachetyp:'a cache ->
  (('b, 'c) op1, 'd) op ->
  (('c, 'c, 'c) op2, 'e) op -> (('b, 'c, 'd, 'e) existop1, 'a) op
val register_existand :
  ddtyp:int ->
  cachetyp:'a local cache ->
  bottom:'b -> (('b, 'b, 'b) op2, 'c) op -> (('b, 'c) existand, 'a local) op
val register_existandop1 :
  ddtyp:int ->
  cachetyp:'a local cache ->
  bottom:'b ->
  (('c, 'b) op1, 'd) op ->
  (('b, 'b, 'b) op2, 'e) op -> (('c, 'b, 'd, 'e) existandop1, 'a local) op

external op2_of_exist : (('a,'b) exist, 'c) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_rivdd_op2_of_exist"
external op2_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_rivdd_op2_of_exist"
external op2_of_existand : (('a,'b) existand, 'c local) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_rivdd_op2_of_exist"
external op2_of_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_rivdd_op2_of_exist"

external op1_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_rivdd_op1_of_existop1"
external op1_of_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_rivdd_op1_of_existop1"

external flush_op : ('a, user local) op -> unit = "camlidl_cudd_rivdd_flush_op"
external flush_allop : unit -> unit = "camlidl_cudd_rivdd_flush_allop"

(*  ********************************************************************** *)
(** {2 Applying operations} *)
(*  ********************************************************************** *)

val apply_op1 : (('a, 'b) op1, 'c) op -> 'add -> 'bdd
val apply_op2 : (('a, 'b, 'c) op2, 'd) op -> 'add -> 'bdd -> 'cdd
val apply_test2 : (('a, 'b) test2, 'c) op -> 'add -> 'bdd -> bool
val apply_op3 :
  (('a, 'b, 'c, 'd) op3, 'e local) op -> 'add -> 'bdd -> 'cdd -> 'ddd
val apply_exist : (('a,'b) exist, 'c) op -> supp:(Man.v Bdd.t) -> 'add -> 'add
val apply_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> supp:(Man.v Bdd.t) -> 'add -> 'bdd
val apply_existand : (('a,'b) existand, 'c local) op -> supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'add -> 'add
val apply_existandop1 : (('a,'b,'c,'d) existandop1, 'e local) op -> supp:(Man.v Bdd.t) -> Man.v Bdd.t -> 'add -> 'bdd

(*  ********************************************************************** *)
(** {2 Map operations (based on automatic local caches} *)
(*  ********************************************************************** *)

val map_op1 : ddtyp:int -> ('a -> 'b) -> 'add -> 'bdd
val map_op2 :
  ddtyp:int ->
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('add -> 'bdd -> 'cdd option) ->
  ('a -> 'b -> 'c) ->
  'add -> 'bdd -> 'cdd
val map_test2 :
  ddtyp:int ->
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('add -> 'bdd -> bool option) ->
  ('a -> 'b -> bool) ->
  'add -> 'bdd -> bool
val map_op3 :
  ddtyp:int ->
  ?special:('add -> 'bdd -> 'cdd -> 'ddd option) ->
  ('a -> 'b -> 'c -> 'd) ->
  'add -> 'bdd -> 'cdd -> 'ddd
val map_exist :
  ddtyp:int -> ('a, 'add, 'b) mexist -> supp:Man.v Bdd.t -> 'add -> 'add
val map_existop1 :
  ddtyp:int ->
  ('a, 'b, 'c) mop1 -> ('b, 'bdd, 'd) mexist -> supp:Man.v Bdd.t -> 'add -> 'bdd
val map_existand :
  ddtyp:int ->
  bottom:'a ->
  ('a, 'add, 'b) mexist -> supp:Man.v Bdd.t -> Man.v Bdd.t -> 'add -> 'add
val map_existandop1 :
  ddtyp:int ->
  bottom:'b ->
  ('a, 'b, 'c) mop1 -> ('b, 'bdd, 'd) mexist ->
  supp:Man.v Bdd.t -> Man.v Bdd.t -> 'add -> 'bdd
