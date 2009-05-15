open Format;;
open Cudd;;

(*
For interactive session:
type on shour shell the command:
"make cuddtop"
and then
"cuddtop -I installation_path_of_lib" or
"cuddtop" if in source directory, after compilation
*)

let man = Man.make_v ~numVars:10 ();;

(* Identifiers of variables printed as "x", "y,", .... *)
let idx = 0;;
let idy = 1;;
let idz = 2;;
let idw = 3;;

(* Correspondance function *)
let print_id fmt id = 
  Format.pp_print_string fmt
    (match id with
    | 0 -> "x"
    | 1 -> "y"
    | 2 -> "z"
    | 3 -> "w"
    | _ as x -> string_of_int x
    )

(* Formula corresponding to litterals "x","y",... *)
let x = Bdd.ithvar man idx;;
let y = Bdd.ithvar man idy;;
let z = Bdd.ithvar man idz;;
let w = Bdd.ithvar man idw;;

(* Raw printing *)
Format.printf "Raw printing:@.x = %a@.y = %a@.z = %a@.w = %a@."
  (Bdd.print_minterm print_id) x
  (Bdd.print_minterm print_id) y
  (Bdd.print_minterm print_id) z
  (Bdd.print_minterm print_id) w
;;
(* Better printing *)
Format.printf "Better printing:@.x = %a@.y = %a@.z = %a@.w = %a@."
  (Bdd.print_minterm print_id) x
  (Bdd.print_minterm print_id) y
  (Bdd.print_minterm print_id) z
  (Bdd.print_minterm print_id) w
;;

let f = Bdd.dand (Bdd.dand x y) (Bdd.dand (Bdd.dnot z) (Bdd.dnot w));;
let g = Bdd.dand (Bdd.dand y z) (Bdd.dnot w);;
(* Raw printing *)
Format.printf "Raw printing:@.f = %a@.g = %a@."
  (Bdd.print_minterm print_id) f
  (Bdd.print_minterm print_id) g
;;
(* Better printing *)
Format.printf "Better printing:@.f = %a@.g = %a@."
  (Bdd.print_minterm print_id) f
  (Bdd.print_minterm print_id) g
;;

(* ********************************************************************** *)
(* ********************************************************************** *)
(* ********************************************************************** *)

let man = Man.make_d ~numVars:10 ();;

let var = Array.init 9 (fun i -> Bdd.ithvar man i);;
let cst = Array.init 30 (fun i -> Rdd.cst man (float_of_int i));;

let f = Array.init 7 (fun i -> Bdd.ite var.(i) var.(i+1) var.(i+2));;

(*
Array.iter
  (fun f -> printf "cst = %a@." Rdd.print__minterm f)
  cst
;;
Array.iter
  (fun f -> printf "f = %a@." Bdd.print__minterm f)
  f
;;
  *)
  
let rdd1 = Rdd.ite f.(0) cst.(0) cst.(1);;
let rdd2 = Rdd.ite f.(1) cst.(2) cst.(4);;

let rdda = Rdd.add rdd1 rdd2;;
let rddb = Rdd.map_op2 
  ~idempotent:false
  ~commutative:true 
  (fun x y -> Gc.compact (); x +. y) 
  rdd1 rdd2;;

let g = Array.init 6 (fun i -> Bdd.ite var.(i) var.(i+1) (Bdd.dor var.(i+2) var.(i+3)));;
let h = Array.init 6 (fun i -> Bdd.dand f.(i) f.(i+1));;

let make_rdd bdd index depth = 
  let res = ref (Rdd.ite bdd.(index) cst.(index) cst.(0)) in
  for i=0 to depth-1 do
    res := Rdd.ite bdd.(index+i) cst.(index+i) !res
  done;
  !res
;;
let testop bdd opa opb =
  for index=0 to (Array.length bdd) - 2 do
    for depth=0 to (Array.length bdd) - index do
      let rdd1 = make_rdd bdd index depth in
      let rdd2 = make_rdd bdd (index+1) (depth-1) in
      let rdda = opa rdd1 rdd2 in
      let rddb = opb rdd1 rdd2 in
      if rdda<>rddb then failwith "";
    done;
  done;
  ()
;;

printf "Here 2@.";;

testop f Rdd.add (Rdd.map_op2 ~commutative:true (+.));;
testop g Rdd.add (Rdd.map_op2 ~commutative:true (+.));;
testop h Rdd.add (Rdd.map_op2 ~commutative:true (+.));;
printf "Here 3@.";;
testop f Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); Man.garbage_collect man; x *. y));;
printf "Here 4@.";;
testop g Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); Man.garbage_collect man; x *. y));;
testop h Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); Man.garbage_collect man; x *. y));;
