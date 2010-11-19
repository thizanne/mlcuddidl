(** Custom operations for MTBDDs *)

(*  ********************************************************************** *)
(** {2 Types and values} *)
(*  ********************************************************************** *)

(*  ====================================================================== *)
(** {3 Caching policy for user operations} *)
(*  ====================================================================== *)

type auto = Custom.auto
type user = Custom.user
type hash = Custom.hash
type cach = Custom.cach
type ('a,'b) local = ('a,'b) Custom.local
  (** Indicates local caching policy, where ['a] is either [auto]
      or [user] and ['b] is either [hash] or [cach]. *)
type global = Custom.global
  (** Indicates global caching policy. *)

type 'a cache = 'a Custom.cache
  (** Indicates caching policy, where ['a] is either [('b,'c) local] or
      [global]. *)

val autohash : (auto,hash) local cache
  (** The local hashtable is cleared automatically at the end on the
      operation on MTBDDs ([apply_XX] function).  Hence, there is
      no reuse between two calls to the same MTBDD operation.

      Default option, as there is no danger to do tricky
      errors.
  *)
val userhash : (user,hash) local cache
  (** It is up to the user to clear regularly the local
      hashtable. Forgetting to do so will prevent garbage
      collection of nodes stored in the table, which can only
      grows.

      The OCaml closure defining the function should not use free
      variables that may be modified and so impact its result:
      they would act as hidden parameters that are not taken into
      account in the cache.

      If such hidden parameters are modified, the cache should be
      cleared with {!flush_op} or {!flush_allop} *)

val global : global cache
  (** The operation on MTBDDs is memoized in CUDD global (regular)
      cache.

      Same remark as for [user local] concerning free
      variables acting as hidden parameters. If hidden parameters
      are modified, the global cache should be cleared with
      {!Man.cache_flush}.
  *)

(*  ====================================================================== *)
(** {3 Type of registered operations} *)
(*  ====================================================================== *)

type ('a,'b) op
  (** ['a] indicates the type and arity of the corresponding
      operation on leaves (one of [('a,'b) op1, ('a,'b,'c) op2,
      ...] described below)

      ['b] indicates the caching policy.

      Objects of this type are externally seen as abstract
      identifiers of operations on MTBDDs.
      - They are created by [register_XX] functions described below.
      - The associated operation is applied to MTBDDs using
      [apply_XX] functions.

      The internal information attached to such an object
      is regularly garbage collected when it is no longer
      referenced.

      The use of such objects is transparent to the user when
      using the [map_XX] functions, which basically perform [let
      id = register_XX .. in apply_XX id ..].

      See the description of unary operation to concretize these
      principles.
  *)

(*  ********************************************************************** *)
(** {2 Flushing caches} *)
(*  ********************************************************************** *)

val flush_op : ('a, (user,'b) local) op -> unit
  (** Flush the (user local) cache associated to the given operation *)

val flush_allop : unit -> unit
  (** Flush all (user local) caches associated to registered
      operations. *)

(*  ********************************************************************** *)
(** {2 Unary operations} *)
(*  ********************************************************************** *)

type ('a,'b) op1
  (** Type of unary operations ['a -> 'b] *)

val register_op1 :
  cachetyp:'c cache ->
  ('a -> 'b) ->
  (('a,'b) op1, 'c) op
  (** Register an unary operation, with the given caching policy,
      and return the corresponding abstract identifier.

      The internal information attached to free operation
      identifier is regularly garbage collected when the
      identifier is no longer referenced. *)

val apply_op1 : (('a,'b) op1, 'c) op -> 'a Vdd.t -> 'b Vdd.t
  (** Apply the unary operation (identifier, as created by
      previous function) to every leaf of the argument MTBDD, and
      return the resulting MTBDD. *)

val map_op1 : ('a -> 'b) -> 'a Vdd.t -> 'b Vdd.t
  (** Combine the two previous operations (using an automatic
      local cache). *)

(** {4 Example:}

    Assuming [type t = bool Vdd.t], and corresponding diagrams
    [bdd:t] and [bdd2:t] (with type [bool], it is safe to use
    directly VDDs, and it simplifies the examples).

    We want to negate every leaf of [bdd1] and [bdd2].

    {ul
    {- We register the operation:

    [let op = register_op1 ~cachetyp:auto (fun b -> not b);;]
    }
    {- Later we apply it on [bdd1] and [bdd2] with

    [let res1 = apply_op1 op bdd1 and res2 = apply_op1 op bdd2;;]
    }
    {- As the cache is cleared between the two calls to
    [apply_op1], even if [bdd1] and [bdd2] share common nodes, it
    might be better to write instead:

{[let op = register_op1 ~cachetyp:user (fun b -> not b);;
let res1 = apply_op1 op bdd1 and res2 = apply_op1 op bdd2;;]}

    We can then flush the local cache with:

    {[flush_op op;;]}

    (especially if [op] cannot be garbage collected but is not used any more).
    }
    {- The third option is to use the CUDD global regular cache,
    which is automatically garbage collected when needed:

{[let op = register_op1 ~cachetyp:global (fun b -> not b);;
let res1 = apply_op1 op bdd1 and res2 = apply_op1 op bdd2;;]}
    }
    {- If the operation is applied only to one diagram, it is simpler to write

    [let res1 = map_op1 (fun b -> not b) bdd1;;]
    }}
*)

(*  ********************************************************************** *)
(** {2 Binary operations} *)
(*  ********************************************************************** *)

(** Similar to unary operations *)

type ('a,'b,'c) op2
  (** Type of binary operations ['a -> 'b -> 'c] *)

val register_op2 :
  cachetyp:'d cache ->
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t option) ->
  ('a -> 'b -> 'c) -> (('a,'b,'c) op2, 'd) op
  (** Register a binary operation, with the given caching policy,
      and return the corresponding abstract identifier.

      [commutative] (default: [false]), when [true], allows to
      optimize the cache usage (hence the speed) when the operation
      is commutative.

      [idempotent] (default: [false]) allows similarly some
      optimization when op is idempotent: [op x x = x].
      This makes sense only if ['a='b='c] (the case will never
      happens otherwise).

      [?special] (default: [None]), if equal to [Some
      specialcase], modifies [op] as follows: it is applied to
      every pair of node during the recursive descend, and if
      [specialcase vdda vddb = (Some vddc)], then [vddc] is
      assumed to be the result of [map_op2 op vdda vddb].  This
      allows not to perform a full recursive descend when a
      special case is encountered. See the example below.
  *)

val apply_op2 : (('a,'b,'c) op2, 'd) op -> 'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t
  (** Apply the binary operation (identifier, as created by
      previous function) to every pair of leafs of the argument MTBDDs, and
      return the resulting MTBDD. *)

val map_op2 :
  ?commutative:bool ->
  ?idempotent:bool ->
  ?special:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t option) ->
  ('a -> 'b -> 'c) ->
  'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t
  (** Combine the two previous operations (using an automatic
      local cache). *)

(** {4 Example:}

    Assuming as for unary operation example [type t = bool Vdd.t]
    and corresponding diagrams [bdd1:t] and [bdd2:t].

    We can compute their conjunction with

    {[let res = map_op2
  ~commutative:true ~idempotent:true
  ~special:(fun bdd1 bdd2 ->
    if Vdd.is_cst bdd1 && Vdd.dval bdd1 = false then Some(bdd1)
    else if Vdd.is_cst bdd2 && Vdd.dval bdd2 = false then Some(bdd2)
    else None
  (fun b1 b2 -> b1 && b2) bdd1 bdd2;;]}
*)

(*  ********************************************************************** *)
(** {2 Binary tests} *)
(*  ********************************************************************** *)

type ('a,'b) test2
  (** Type of binary tests ['a -> 'b -> bool] *)

val register_test2 :
  cachetyp:'c cache ->
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('a Vdd.t -> 'b Vdd.t -> bool option) ->
  ('a -> 'b -> bool) -> (('a,'b) test2, 'c) op
  (** Register a binary test, with the given caching policy,
      and return the corresponding abstract identifier.

      [commutative] (default: [false]), when [true], allows to
      optimize the cache usage (hence the speed) when the test is commutative.

      [reflexive] (default: [false]) allows similarly some
      optimization when test is reflexive: [test x x = true].
      This makes sense only if ['a='b] (the case will never
      happen otherwise).

      [?special] (default: [None]) has the same semantics as for
      binary operations above.
  *)

val apply_test2 : (('a,'b) test2, 'c) op -> 'a Vdd.t -> 'b Vdd.t -> bool
val map_test2 :
  ?commutative:bool ->
  ?reflexive:bool ->
  ?special:('a Vdd.t -> 'b Vdd.t -> bool option) ->
  ('a -> 'b -> bool) ->
  'a Vdd.t -> 'b Vdd.t -> bool
  (** See binary operations. *)

(** {4 Example:}

    Still assuming [type t = bool Vdd.t]
    and corresponding diagrams [bdd1:t] and [bdd2:t].

    We can test their implication with

{[let res = map_test2
  ~commutative:false ~idempotent:true
  ~special:(fun bdd1 bdd2 ->
    if Vdd.is_cst bdd1 && Vdd.dval bdd1 = false then Some(true)
    else if Vdd.is_cst bdd2 then Some(Vdd.dval bdd2)
    else None
  )
  (fun b1 b2 -> b2 || not b1) bdd1 bdd2]}
*)

(*  ********************************************************************** *)
(** {2 Ternary operations} *)
(*  ********************************************************************** *)

(** See binary operations. Only local caches are possible here. *)

type ('a,'b,'c,'d) op3
  (** Type of ternary operations ['a -> 'b -> 'c -> 'd] *)

val register_op3 :
  cachetyp:('e,'f) local cache ->
  ?special:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t -> 'd Vdd.t option) ->
  ('a -> 'b -> 'c -> 'd) ->
  (('a,'b,'c,'d) op3, ('e,'f) local) op

val apply_op3 :
  (('a,'b,'c,'d) op3, ('e,'f) local) op ->
  'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t -> 'd Vdd.t

val map_op3 :
  ?special:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t -> 'd Vdd.t option) ->
  ('a -> 'b -> 'c -> 'd) ->
  'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t -> 'd Vdd.t

(** {4 Example:}

    Still assuming [type t = bool Vdd.t]
    and corresponding diagrams [bdd1:t], [bdd2:t], [bdd3:t].

    We can define [if-then-else] with
{[let res = map_op3
  ~special:(fun bdd1 bdd2 bdd3 ->
    if Vdd.is_cst bdd1
    then Some(if Vdd.dval bdd1 (* = true *) then bdd2 else bdd3)
    else None
  )
  (fun b1 b2 b3 -> if b1 then b2 else b3) bdd1 bdd2 bdd3]}
*)

(*  ********************************************************************** *)
(** {2 Quantification} *)
(*  ********************************************************************** *)

type ('a,'b) exist
  (** Type of quantification operation [supp -> 'a -> 'a].  The underlying
      leaf operation [op:'a -> 'a -> 'a] is assumed to be
      commutative and idempotent ([op f f=f]).  When a Boolean
      variable in [supp] is quantified out, [map_op2 op] is
      used to combine the two branch of the diagram. *)

val register_exist :
  cachetyp:'c cache -> (('a,'a,'a) op2,'b) op -> (('a,'b) exist, 'c) op
  (** Register an existential quantification operation, with the
      given caching policy, and the given underlying binary
      operation (assumed to be commutative and idempotent), and
      return the corresponding abstract identifier.

      The underlying binary operation remains referenced by the
      quantification operation as long as the latter
      is alive (garbage collection remark). *)

val op2_of_exist : (('a,'b) exist, 'c) op -> (('a,'a,'a) op2, 'b) op
  (** Extract from a quantification operation its underlying
      binary operation. *)

val apply_exist : (('a,'b) exist, 'c) op -> supp:Bdd.vt -> 'a Vdd.t -> 'a Vdd.t
  (** Quantify out the variables in [supp] (assumed to be a
      positive cube) in the MTBDD. *)

type ('a, 'b) mexist = [
  | `Fun of 
      ('a Vdd.t -> 'a Vdd.t -> 'a Vdd.t option) option * 
      ('a -> 'a -> 'a)
  | `Op of (('a, 'a, 'a) op2, 'b) op
]
  (** Used by [map_exist] and other functions below.

      If we have defined 

      [let op2 = register_op2 ?special ~commutative:true ~idempotent:true f;;]

      then [`Fun (?special,f)] is equivalent to [`Op op2] (excluding
      cache options).
  *)

val map_exist :
  ('a, 'b) mexist ->
  supp:Bdd.vt -> 'a Vdd.t -> 'a Vdd.t
  (** Quantify out the variables in [supp] (assumed to be a
      positive cube) in the MTBDD. *)

(** {4 Example:}

    Still assuming [type t = bool Vdd.t]
    and corresponding diagrams [bdd:t]

    We define ordinary existential quantification with

{[let dor = register_op2 ~cache:auto ~commutative:true ~idempotent:true ( || );;
let exist = register_exist ~cache:auto dor;;
let res = apply_exist exist ~supp bdd;;
let res = map_exist (`Op dor) ~supp bdd;; (* equivalent result *)
let res = map_exist (`Fun (None,( || ))) ~supp bdd;; (* equivalent result *)]}

    We can define ordinary universal quantification by replacing
    [||] with [&&].
*)

(*  ********************************************************************** *)
(** {2 Unary operation and quantification} *)
(*  ********************************************************************** *)

(** Similar to quantification *)

type ('a,'b,'c,'d) existop1
  (** Type of op1 and quantification operation.

      [existop1 op2 op1 supp bdd] is equivalent to [exist op2 supp (op1
      f)] (with [op1:'a -> 'b] and [op2:'b -> 'b ->'b].

      ['c] indicates the cache policy of [op1]

      ['d] indicates the cache policy of [op2]

      The binary operation [op2:'b -> 'b -> 'b] is assumed to be
      commutative and idempotent ([op f f=f]). *)

val register_existop1 :
  cachetyp:'e cache ->
  (('a, 'b) op1, 'd) op ->
  (('b, 'b, 'b) op2, 'c) op ->
  (('a,'b,'d,'c) existop1, 'e) op

val op1_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('a,'b) op1, 'c) op
val op2_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('b,'b,'b) op2, 'd) op  
  (** Extract from the operation its underlying
      unary and binary operations. *)

val apply_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> supp:Bdd.vt -> 'a Vdd.t -> 'b Vdd.t

type ('a, 'b, 'c) mop1 = [
  | `Fun of 'a -> 'b
  | `Op of (('a, 'b) op1, 'c) op
]
val map_existop1 :
  ('a,'b,'c) mop1 -> ('b,'d) mexist ->
  supp:Bdd.vt -> 'a Vdd.t -> 'b Vdd.t

(*  ********************************************************************** *)
(** {2 Conjunction and quantification} *)
(*  ********************************************************************** *)

type ('a,'b) existand
  (** Type of combined quantification and and operation.

      [existand ~bottom op2 supp bdd f] is equivalent to [exist
      op2 supp (ite bdd f bottom)]. 

      The leaf
      operation [op2:'a -> 'a -> 'a] is assumed to be commutative,
      idempotent ([op2 f f=f]), and also [op2 f bottom = f].
  *)

val register_existand :
  cachetyp:('c,'d) local cache ->
  bottom:'a ->
  (('a, 'a, 'a) op2, 'b) op -> (('a,'b) existand, ('c,'d) local) op

val op2_of_existand : (('a,'b) existand, ('c,'d) local) op -> (('a,'a,'a) op2, 'b) op

val apply_existand : (('a,'b) existand, ('c,'d) local) op -> supp:Bdd.vt -> Bdd.vt -> 'a Vdd.t -> 'a Vdd.t

val map_existand :
  bottom:'a ->
  ('a, 'b) mexist ->
  supp:Bdd.vt -> Bdd.vt -> 'a Vdd.t -> 'a Vdd.t

(*  ********************************************************************** *)
(** {2 Unary operation, conjunction and quantification} *)
(*  ********************************************************************** *)

type ('a,'b,'c,'d) existandop1
  (** Type of unary operation, conjunction and quantification  

      [existandop1 ~bottom op op1 supp bdd f] is equivalent to
      [exist op2 supp (ite bdd (op1 f) bottom))].

      The leaf operation [op2:'b -> 'b -> 'b] is assumed to be
      commutative, idempotent ([op2 f f=f]), and also [op2 f bottom
      = f].  
  *)

val register_existandop1 :
  cachetyp:('e,'f) local cache ->
  bottom:'b ->
  (('a, 'b) op1, 'd) op ->
  (('b, 'b, 'b) op2, 'c) op ->
  (('a,'b,'d,'c) existandop1, ('e,'f) local) op

val op2_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('b,'b,'b) op2, 'd) op
val op1_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('a,'b) op1, 'c) op

val apply_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> supp:Bdd.vt -> Bdd.vt -> 'a Vdd.t -> 'b Vdd.t

val map_existandop1 :
  bottom:'b ->
  ('a,'b,'c) mop1 -> ('b,'d) mexist ->
  supp:Bdd.vt -> Bdd.vt -> 'a Vdd.t -> 'b Vdd.t
