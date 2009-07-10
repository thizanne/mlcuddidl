(* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution  *)

(** Custom operations for ADDs (Internal) *)

(** This is used by modules {!Add} and {!Vdd} *)

(*  ********************************************************************** *)
(** {2 Types and values} *)
(*  ********************************************************************** *)

(** {3 Operations on leaves of MTBDDs} *)

type ('a,'b) op1
type ('a,'b,'c) op2
type ('a,'b) test2
type ('a,'b,'c,'d) op3
type ('a,'b) exist
type ('a,'b,'c,'d) existop1
type ('a,'b) existand
type ('a,'b,'c,'d) existandop1
type ('a,'b) vectorcomposeop1

(** {3 Caching policy} *)

type auto
type user
type hash
type cach

type ('a,'b) local
type global

type 'a cache = int

let global = 0
let autohash = 1
let userhash = 2

(** {3 Type of registered operations} *)

type ('a,'b) op

type ('a, 'add, 'b) mexist = [
  | `Fun of ('add -> 'add -> 'add option) option * ('a -> 'a -> 'a)
  | `Op of (('a, 'a, 'a) op2, 'b) op
]

type ('a, 'b, 'c) mop1 = [ 
  | `Fun of 'a -> 'b 
  | `Op of (('a, 'b) op1, 'c) op 
]

(*  ********************************************************************** *)
(** {2 Registering operations} *)
(*  ********************************************************************** *)

external _internal_register_op : ddtyp:int -> cachetyp:int -> optyp:int -> commutative:bool -> idempotent:bool -> op2:((('a,'b,'c) op2, 'd) op) -> op1:(('e,'f) op1, 'g) op -> special:('h -> 'i) ->
  ('j -> 'k) ->
  ('l,'m) op = "camlidl_cudd_avdd_register_op_byte" "camlidl_cudd_avdd_register_op"

let register_op1
    ~(ddtyp:int)
    ~(cachetyp:'c cache)
    (op : 'a -> 'b)
    :
    (('a,'b) op1, 'c) op
    =
  _internal_register_op
    ~ddtyp ~cachetyp ~optyp:0 ~commutative:false ~idempotent:false
    ~op2:(Obj.magic ()) ~op1:(Obj.magic ())
    ~special:(Obj.magic ())
    op

let register_op2
    ~(ddtyp:int)
    ~(cachetyp:'d cache)
    ?(commutative=false)
    ?(idempotent=false)
    ?(special:('add -> 'bdd -> 'cdd option) option)
    (op : 'a -> 'b -> 'c)
    :
    (('a,'b,'c) op2, 'd) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:1 ~commutative ~idempotent
    ~op2:(Obj.magic ()) ~op1:(Obj.magic ())
    ~special:(match special with None -> Obj.magic () | Some f -> f)
    op

let register_test2
    ~(ddtyp:int)
    ~(cachetyp:'c cache)
    ?(commutative=false)
    ?(reflexive=false)
    ?(special:('add -> 'bdd -> bool option) option)
    (op : 'a -> 'b -> bool)
    :
    (('a,'b) test2, 'c) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:2
    ~commutative ~idempotent:reflexive
    ~op2:(Obj.magic ()) ~op1:(Obj.magic ())
    ~special:(match special with None -> Obj.magic () | Some f -> f)
    op

let register_op3
    ~(ddtyp:int)
    ~(cachetyp:('e,'f) local cache)
    ?(special:('add -> 'bdd -> 'cdd -> 'ddd option) option)
    (op : 'a -> 'b -> 'c -> 'd)
    :
    (('a,'b,'c,'d) op3, ('e,'f) local) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:3 ~commutative:false ~idempotent:false
    ~op2:(Obj.magic ()) ~op1:(Obj.magic ())
    ~special:(match special with None -> Obj.magic () | Some f -> f)
    op

let register_exist
    ~(ddtyp:int)
    ~(cachetyp:'c cache)
    (op2 : (('a,'a,'a) op2,'b) op)
    :
    (('a,'b) exist, 'c) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:4 ~commutative:false ~idempotent:false
    ~op2:op2 ~op1:(Obj.magic ())
    ~special:(Obj.magic ())
    (Obj.magic ())

let register_existop1
    ~(ddtyp:int)
    ~(cachetyp:'e cache)
    (op1 : (('a,'b) op1,'c) op)
    (op2 : (('b,'b,'b) op2,'d) op)
    :
    (('a,'b,'c,'d) existop1, 'e) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:5 ~commutative:false ~idempotent:false
    ~op2:op2 ~op1:op1
    ~special:(Obj.magic ())
    (Obj.magic ())

let register_existand
    ~(ddtyp:int)
    ~(cachetyp:('c,'d) local cache)
    ~(bottom:'a)
    (op2 : (('a,'a,'a) op2,'b) op)
    :
    (('a,'b) existand, ('c,'d) local) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:6 ~commutative:false ~idempotent:false
    ~op2:op2 ~op1:(Obj.magic ())
    ~special:(Obj.magic ())
    (Obj.magic bottom)

let register_existandop1
    ~(ddtyp:int)
    ~(cachetyp:('e,'f) local cache)
    ~(bottom:'b)
    (op1 : (('a,'b) op1,'c) op)
    (op2 : (('b,'b,'b) op2,'d) op)
    :
    (('a,'b,'c,'d) existandop1, ('e,'f) local) op
    =
  _internal_register_op ~ddtyp ~cachetyp ~optyp:7 ~commutative:false ~idempotent:false
    ~op2:op2 ~op1:op1
    ~special:(Obj.magic ())
    (Obj.magic bottom)

(*  ********************************************************************** *)
(** {2 Inspecting operations and flushing caches} *)
(*  ********************************************************************** *)

external op2_of_exist : (('a,'b) exist, 'c) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existand : (('a,'b) existand, ('c,'d) local) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_avdd_op2_of_exist"

external op1_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_avdd_op1_of_existop1"
external op1_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_avdd_op1_of_existop1"

external flush_op : ('a, (user,'b) local) op -> unit = "camlidl_cudd_avdd_flush_op"
external flush_allop : unit -> unit = "camlidl_cudd_avdd_flush_allop"

(*  ********************************************************************** *)
(** {2 Applying operations} *)
(*  ********************************************************************** *)

external _internal_map_op : ('a,'b) op -> 'c array -> 'd = "camlidl_cudd_avdd_map_op"

let apply_op1 (op:(('a,'b) op1, 'c) op) (dd:'add) : 'bdd =
  _internal_map_op op [|dd|]

let apply_op2 (op:(('a,'b,'c) op2, 'd) op) (dd1:'add) (dd2:'bdd) : 'cdd =
  _internal_map_op op [|dd1; Obj.magic dd2|]

let apply_test2 (op:(('a,'b) test2, 'c) op) (dd1:'add) (dd2:'bdd) : bool =
  _internal_map_op op [|dd1; Obj.magic dd2|]

let apply_op3 (op:(('a,'b,'c,'d) op3, ('e,'f) local) op) (dd1:'add) (dd2:'bdd) (dd3:'cdd) : 'ddd =
  _internal_map_op op [|dd1;Obj.magic dd2;Obj.magic dd3|]

let apply_exist (op:(('a,'b) exist, 'c) op) ~(supp:'d Bdd.t) (dd:'add) : 'add =
  _internal_map_op op [|Obj.magic supp; dd|]

let apply_existop1 (op:(('a,'b,'c,'d) existop1, 'e) op) ~(supp:'f Bdd.t) (dd:'add) : 'bdd =
  _internal_map_op op [|Obj.magic supp; dd|]

let apply_existand (op:(('a,'b) existand, ('c,'d) local) op) ~(supp:'e Bdd.t) (bdd:'e Bdd.t) (dd:'add) : 'add =
  _internal_map_op op [|Obj.magic supp; Obj.magic bdd; dd|]

let apply_existandop1 (op:(('a,'b,'c,'d) existandop1, ('e,'f) local) op) ~(supp:'g Bdd.t) (bdd:'g Bdd.t) (dd:'add) : 'bdd =
  _internal_map_op op [|Obj.magic supp; Obj.magic bdd; dd|]

(*  ********************************************************************** *)
(** {2 Map operations (based on automatic local caches} *)
(*  ********************************************************************** *)

let map_op1 ~ddtyp op dd
    =
  let op = register_op1 ~ddtyp ~cachetyp:autohash op in
  apply_op1 op dd

let map_op2
    ~ddtyp
    ?(commutative=false)
    ?(idempotent=false)
    ?special
    op dd1 dd2
    =
  let op = register_op2 ~ddtyp ~cachetyp:autohash ~commutative ~idempotent ?special op in
  apply_op2 op dd1 dd2

let map_test2
    ~ddtyp
    ?(commutative=false)
    ?(reflexive=false)
    ?special
    op dd1 dd2
    =
  let op = register_test2 ~ddtyp ~cachetyp:autohash ~commutative ~reflexive ?special op in
  apply_test2 op dd1 dd2

let map_op3 ~ddtyp ?special op dd1 dd2 dd3
    =
  let op = register_op3 ~ddtyp ~cachetyp:autohash ?special op in
  apply_op3 op dd1 dd2 dd3

let map_exist
    ~ddtyp
    (mexist:('a,'add,'b) mexist)
    ~supp dd
    =
  let (op2:(('a,'a,'a) op2,'b) op) = match mexist with
    | `Op op -> op
    | `Fun(special,op) ->
	register_op2 ~ddtyp ~cachetyp:autohash ~commutative:true ~idempotent:true ?special op
  in
  let exist = register_exist ~ddtyp ~cachetyp:autohash op2 in
  apply_exist exist ~supp dd

let map_existop1
    ~ddtyp
    (mop1:('a,'b,'c) mop1)
    (mexist:('b,'bdd,'d) mexist)
    ~supp dd
    =
  let (op2:(('b,'b,'b) op2,'d) op) = match mexist with
    | `Op op -> op
    | `Fun(special,op) ->
	register_op2 ~ddtyp ~cachetyp:autohash ~commutative:true ~idempotent:true ?special op
  in
  let op1 = match mop1 with
    | `Op op -> op
    | `Fun op -> register_op1 ~ddtyp ~cachetyp:autohash op
  in
  let existop1 = register_existop1 ~ddtyp ~cachetyp:autohash op1 op2 in
  apply_existop1 existop1 ~supp dd

let map_existand
    ~ddtyp
    ~(bottom:'a)
    (mexist:('a,'add,'b) mexist)
    ~supp bdd dd
    =
  let (op2:(('a,'a,'a) op2,'b) op) = match mexist with
    | `Op op -> op
    | `Fun(special,op) ->
	register_op2 ~ddtyp ~cachetyp:autohash ~commutative:true ~idempotent:true ?special op
  in
  let existand = register_existand ~ddtyp ~cachetyp:autohash ~bottom op2 in
  apply_existand existand ~supp bdd dd

let map_existandop1
    ~ddtyp
    ~(bottom:'b)
    (mop1:('a,'b,'c) mop1)
    (mexist:('b,'bdd,'d) mexist)
    ~supp bdd dd
    =
  let (op2:(('b,'b,'b) op2,'d) op) = match mexist with
    | `Op op -> op
    | `Fun(special,op) ->
	register_op2 ~ddtyp ~cachetyp:autohash ~commutative:true ~idempotent:true ?special op
  in
  let op1 = match mop1 with
    | `Op op -> op
    | `Fun op -> register_op1 ~ddtyp ~cachetyp:autohash op
  in
  let existandop1 = register_existandop1 ~ddtyp ~cachetyp:autohash ~bottom op1 op2 in
  apply_existandop1 existandop1 ~supp bdd dd
