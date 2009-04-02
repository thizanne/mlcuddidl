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
