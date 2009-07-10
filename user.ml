(** Custom operations for MTBDDs *)

(*  ====================================================== *)
(** {2 Types and values} *)
(*  ====================================================== *)

type 'a t = 'a Vdd.t

(** {3 Operations on leaves of MTBDDs} *)

type ('a, 'b) op1 = ('a, 'b) Custom.op1
type ('a, 'b, 'c) op2 = ('a, 'b, 'c) Custom.op2
type ('a, 'b) test2 = ('a, 'b) Custom.test2
type ('a, 'b, 'c, 'd) op3 = ('a, 'b, 'c, 'd) Custom.op3
type ('a, 'b) exist = ('a, 'b) Custom.exist
type ('a, 'b, 'c, 'd) existop1 = ('a, 'b, 'c, 'd) Custom.existop1
type ('a, 'b) existand = ('a, 'b) Custom.existand
type ('a, 'b, 'c, 'd) existandop1 = ('a, 'b, 'c, 'd) Custom.existandop1
type ('a, 'b) vectorcomposeop1 = ('a, 'b) Custom.vectorcomposeop1

(** {3 Caching policy} *)

type auto = Custom.auto
type user = Custom.user
type hash = Custom.hash
type cach = Custom.cach

type ('a,'b) local = ('a,'b) Custom.local
type global = Custom.global

type 'a cache = 'a Custom.cache

let global = Custom.global
let autohash = Custom.autohash
let userhash = Custom.userhash

(** {3 Type of registered operations} *)

type ('a, 'b) op = ('a, 'b) Custom.op

type ('a, 'b) mexist = [ 
  | `Fun of ('a t -> 'a t -> 'a t option) option * ('a -> 'a -> 'a)
  | `Op of (('a, 'a, 'a) op2, 'b) op 
]
type ('a, 'b, 'c) mop1 = [
  | `Fun of 'a -> 'b
  | `Op of (('a, 'b) op1, 'c) op
]

(*  ********************************************************************** *)
(** {2 Registering operations} *)
(*  ********************************************************************** *)

type _ddtyp = int (* 0:ADD, 1:IDD, 2:VDD *)
let _ddtyp = 2

let register_op1 ~cachetyp = Custom.register_op1 ~ddtyp:_ddtyp ~cachetyp
let register_op2 ~cachetyp = Custom.register_op2 ~ddtyp:_ddtyp ~cachetyp
let register_test2  ~cachetyp = Custom.register_test2 ~ddtyp:_ddtyp ~cachetyp
let register_op3 ~cachetyp = Custom.register_op3 ~ddtyp:_ddtyp ~cachetyp
let register_exist ~cachetyp (op:(('a,'a,'a) Custom.op2,'b) Custom.op) = Custom.register_exist ~ddtyp:_ddtyp ~cachetyp op
let register_existop1 ~cachetyp (op1:(('a,'b) Custom.op1,'d) Custom.op) (op:(('b,'b,'b) Custom.op2,'c) Custom.op) = Custom.register_existop1 ~ddtyp:_ddtyp ~cachetyp op1 op
let register_existand ~cachetyp ~(bottom:'a) (op:(('a,'a,'a) Custom.op2,'b) Custom.op) = Custom.register_existand ~ddtyp:_ddtyp ~cachetyp ~bottom op
let register_existandop1 ~cachetyp ~(bottom:'b) (op1:(('a,'b) Custom.op1,'d) Custom.op) (op:(('b,'b,'b) Custom.op2,'c) Custom.op) = Custom.register_existandop1 ~ddtyp:_ddtyp ~cachetyp ~bottom op1 op

(*  ********************************************************************** *)
(** {2 Inspecting operations and flushing caches} *)
(*  ********************************************************************** *)

external op2_of_exist : (('a,'b) exist, 'c) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existand : (('a,'b) existand, ('c,'d) local) op -> (('a,'a,'a) op2, 'b) op = "camlidl_cudd_avdd_op2_of_exist"
external op2_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('b,'b,'b) op2, 'd) op = "camlidl_cudd_avdd_op2_of_exist"

external op1_of_existop1 : (('a,'b,'c,'d) existop1, 'e) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_avdd_op1_of_existop1"
external op1_of_existandop1 : (('a,'b,'c,'d) existandop1, ('e,'f) local) op -> (('a,'b) op1, 'c) op = "camlidl_cudd_avdd_op1_of_existop1"

external flush_op : ('a,(user,'b) local) op -> unit = "camlidl_cudd_avdd_flush_op"
external flush_allop : unit -> unit = "camlidl_cudd_avdd_flush_allop"

(*  ********************************************************************** *)
(** {2 Applying operations} *)
(*  ********************************************************************** *)

let (apply_op1 : (('a,'b) op1,'c) op -> 'a t -> 'b t) = Custom.apply_op1
let (apply_op2 : (('a,'b,'c) op2,'d) op -> 'a t -> 'b t -> 'c t) = Custom.apply_op2
let (apply_test2 : (('a,'b) test2,'c) op -> 'a t -> 'b t -> bool) = Custom.apply_test2
let (apply_op3 : (('a,'b,'c,'d) op3,('e,'f) local) op -> 'a t -> 'b t -> 'c t -> 'd t) = Custom.apply_op3
let (apply_exist : (('a,'b) exist,'c) op -> supp:(Bdd.vt) -> 'a t -> 'a t) = Custom.apply_exist
let (apply_existop1 : (('a,'b,'c,'d) existop1,'e) op -> supp:(Bdd.vt) -> 'a t -> 'b t) = Custom.apply_existop1
let (apply_existand : (('a,'b) existand,('c,'d) local) op -> supp:(Bdd.vt) -> Bdd.vt -> 'a t -> 'a t) = Custom.apply_existand
let (apply_existandop1 : (('a,'b,'c,'d) existandop1,('e,'f) local) op -> supp:(Bdd.vt) -> Bdd.vt -> 'a t -> 'b t) = Custom.apply_existandop1

(*  ********************************************************************** *)
(** {2 Map operations (based on automatic local caches} *)
(*  ********************************************************************** *)

let map_op1 f dd = Custom.map_op1 ~ddtyp:_ddtyp f dd
let map_op2 ?commutative ?idempotent ?special f dd1 dd2 = Custom.map_op2 ~ddtyp:_ddtyp ?commutative ?idempotent ?special f dd1 dd2
let map_test2 ?commutative ?reflexive ?special f dd1 dd2 = Custom.map_test2 ~ddtyp:_ddtyp ?commutative ?reflexive ?special f dd1 dd2
let map_op3 ?special f dd1 dd2 dd3 = Custom.map_op3 ~ddtyp:_ddtyp ?special f dd1 dd2 dd3
let map_exist mexist ~supp dd = Custom.map_exist ~ddtyp:_ddtyp mexist ~supp dd
let map_existop1 mop1 mexist ~supp dd = Custom.map_existop1 ~ddtyp:_ddtyp mop1 mexist ~supp dd
let map_existand ~bottom mexist ~supp bdd dd = Custom.map_existand ~ddtyp:_ddtyp ~bottom mexist ~supp bdd dd
let map_existandop1 ~bottom mop1 mexist ~supp bdd dd = Custom.map_existandop1 ~ddtyp:_ddtyp ~bottom mop1 mexist ~supp bdd dd

