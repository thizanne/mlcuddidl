(*
First, "make cuddtop"
Then, type on shour shell the command:
"cuddtop -I installation_path_of_lib" or
"cuddtop" if in source directory, after compilation
*)

let man = Man.make_d ~numVars:10 ();;
#install_printer Bdd.print__minterm;;
#install_printer Idd.print__minterm;;
let x = (Bdd.dnot (Bdd.ithvar man 1));;
let y = (Bdd.dnot (Bdd.ithvar man 2));;
let z = Bdd.ithvar man 3;;
let w = Bdd.ithvar man 4;;

let f = Bdd.dand (Bdd.dand x y) (Bdd.dand (Bdd.dnot z) (Bdd.dnot w));;
let g = Bdd.dand (Bdd.dand y z) (Bdd.dnot w);;

let idd1 = Idd.ite f (Idd.cst man 0) (Idd.cst man 1);;
let supp = Idd.support idd1;;
let h = Idd.guard_of_nonbackground idd1;;
Bdd.gendisjdecomp h;;
Idd.nbpaths idd1;;
Idd.nbnonzeropaths idd1;;
Idd.nbminterms 4 idd1;;
Idd.nbleaves idd1;;
Idd.leaves idd1;;

let res = Bdd.cube_union f g;;
let res = Bdd.cube_union (Bdd.dnot f) g;;

let f = Bdd.dand (Bdd.dand (Bdd.dnot x) (Bdd.dnot y)) (Bdd.dand z w);;
let g = Bdd.dand (Bdd.dand (Bdd.dnot y) z) w;;

let res = Bdd.support_inter f g;;
let res = Bdd.support_union f g;;
let z = Bdd.nxor x y;;
let z = Bdd.xor x y;;
Bdd.pick_cubes_on_support z (Bdd.cube_of_minterm man [|Manager.True;Manager.True;Manager.True;Manager.True|]) 2;;

(* essai des Vdd avec des entiers *)
Gc.set { (Gc.get()) with Gc.verbose = 0x11 };;

let man = Manager.make_v ~numVars:20 ();;
#install_printer Bdd.print__minterm;;
let p = Vdd.print__minterm Format.pp_print_int;;
#install_printer p;;

let _ =
  let tab = Array.init 5 (fun i -> Vdd.cst man i) in
  for i=1 to 4 do tab.(i) <- tab.(0) done;
  let res = ref (Bdd.dtrue man) in
  for i=0 to 9 do
    res := Bdd.dand !res (Bdd.xor (Bdd.ithvar man i) (Bdd.ithvar man (i+10)));
  done;
  ()
;;
Manager.print_info man;;
Gc.full_major();;
Manager.reduce_heap man Manager.REORDER_SIFT 1;;
Gc.full_major();;
Manager.reduce_heap man Manager.REORDER_NONE 0;;
Manager.print_info man;;

let x = (Bdd.dnot (Bdd.ithvar man 1));;
let y = (Bdd.dnot (Bdd.ithvar man 2));;
let z = Bdd.ithvar man 3;;
let w = Bdd.ithvar man 4;;

let f = Bdd.dand (Bdd.dand x y) (Bdd.dand (Bdd.dnot z) (Bdd.dnot w));;
let g = Bdd.dand (Bdd.dand y z) (Bdd.dnot w);;

let idd1 = Vdd.ite f (Vdd.cst man 0) (Vdd.cst man 1);;
let supp = Vdd.support idd1;;
let h = Vdd.guard_of_nonbackground idd1;;
Bdd.gendisjdecomp h;;
Vdd.nbpaths idd1;;
Vdd.nbnonzeropaths idd1;;
Vdd.nbminterms 4 idd1;;
Vdd.nbleaves idd1;;
Vdd.leaves idd1;;

let res = Bdd.cube_union f g;;
let res = Bdd.cube_union (Bdd.dnot f) g;;

let f = Bdd.dand (Bdd.dand (Bdd.dnot x) (Bdd.dnot y)) (Bdd.dand z w);;
let g = Bdd.dand (Bdd.dand (Bdd.dnot y) z) w;;

let res = Bdd.support_inter f g;;
let res = Bdd.support_union f g;;
let z = Bdd.nxor x y;;
let z = Bdd.xor x y;;
Bdd.pick_cubes_on_support z (Bdd.cube_of_minterm man [|Manager.True;Manager.True;Manager.True;Manager.True|]) 2;;

(* essai des Vdd avec des références sur des entiers *)
Gc.set { (Gc.get()) with Gc.verbose = 0x11 };;
let man = Manager.make_v ~numVars:20 ();;
#install_printer Bdd.print__minterm;;
let p = Vdd.print__minterm (fun fmt r -> Format.pp_print_int fmt !r);;
#install_printer p;;

let finalise_leaf r = Format.printf "finalizing %i@." !r
let make_leaf n = 
  let res = ref n in
  Gc.finalise finalise_leaf res;
  res
;;
let tab = Array.init 30 (fun i -> (i,i,i,Vdd.cst man (make_leaf i)));;
let tab = Array.map (fun (_,_,_,vdd) -> vdd) tab;;
let res = ref tab.(0);;
for i=0 to 9 do
  res := Vdd.ite (Bdd.ithvar man i) tab.(i) !res
done;;
Gc.compact();;
Manager.garbage_collect man;;
for i=0 to 9 do
  res := Vdd.ite (Bdd.ithvar man (i+10)) tab.(i) !res
done;;


let x = (Bdd.dnot (Bdd.ithvar man 1));;
let y = (Bdd.dnot (Bdd.ithvar man 2));;
let z = Bdd.ithvar man 3;;
let w = Bdd.ithvar man 4;;

let f = Bdd.dand (Bdd.dand x y) (Bdd.dand (Bdd.dnot z) (Bdd.dnot w));;
let g = Bdd.dand (Bdd.dand y z) (Bdd.dnot w);;


let leaf0 = ref 0 and leaf1 = ref 1;;
let idd1 = Vdd.ite f (Vdd.cst man leaf0) (Vdd.cst man leaf1);;
let supp = Vdd.support idd1;;
let h = Vdd.guard_of_nonbackground idd1;;
let h = Vdd.guard_of_leaf idd1 leaf0;;
let h = Vdd.guard_of_leaf idd1 leaf1;;
let h = Vdd.guard_of_leaf idd1 (ref 0);;
let h = Vdd.guard_of_leaf idd1 (ref 1);;
Bdd.gendisjdecomp h;;
Vdd.nbpaths idd1;;
Vdd.nbnonzeropaths idd1;;
Vdd.nbminterms 4 idd1;;
Vdd.nbleaves idd1;;
Vdd.leaves idd1;;

let res = Bdd.cube_union f g;;
let res = Bdd.cube_union (Bdd.dnot f) g;;

let f = Bdd.dand (Bdd.dand (Bdd.dnot x) (Bdd.dnot y)) (Bdd.dand z w);;
let g = Bdd.dand (Bdd.dand (Bdd.dnot y) z) w;;

let res = Bdd.support_inter f g;;
let res = Bdd.support_union f g;;
let z = Bdd.nxor x y;;
let z = Bdd.xor x y;;
Bdd.pick_cubes_on_support z (Bdd.cube_of_minterm man [|Manager.True;Manager.True;Manager.True;Manager.True|]) 2;;



(* essai des Vdd avec des flottants *)

let p = Vdd.print__minterm Format.pp_print_float;;
#install_printer p;;
let x = (Bdd.dnot (Bdd.ithvar man 1));;
let y = (Bdd.dnot (Bdd.ithvar man 2));;
let z = Bdd.ithvar man 3;;
let w = Bdd.ithvar man 4;;

let f = Bdd.dand (Bdd.dand x y) (Bdd.dand (Bdd.dnot z) (Bdd.dnot w));;
let g = Bdd.dand (Bdd.dand y z) (Bdd.dnot w);;

let idd1 = Vdd.ite f (Vdd.cst man 0.0) (Vdd.cst man 1.0);;
let supp = Vdd.support idd1;;
let h = Vdd.guard_of_nonbackground idd1;;
Bdd.gendisjdecomp h;;
Vdd.nbpaths idd1;;
Vdd.nbnonzeropaths idd1;;
Vdd.nbminterms 4 idd1;;
Vdd.nbleaves idd1;;
Vdd.leaves idd1;;

let res = Bdd.cube_union f g;;
let res = Bdd.cube_union (Bdd.dnot f) g;;

let f = Bdd.dand (Bdd.dand (Bdd.dnot x) (Bdd.dnot y)) (Bdd.dand z w);;
let g = Bdd.dand (Bdd.dand (Bdd.dnot y) z) w;;

let res = Bdd.support_inter f g;;
let res = Bdd.support_union f g;;
let z = Bdd.nxor x y;;
let z = Bdd.xor x y;;
Bdd.pick_cubes_on_support z (Bdd.cube_of_minterm man [|Manager.True;Manager.True;Manager.True;Manager.True|]) 2;;


(* essai des Rdd *)

let man = Manager.make_d ~numVars:10 ();;
#install_printer Bdd.print__minterm;;
#install_printer Rdd.print__minterm;;

let var = Array.init 9 (fun i -> Bdd.ithvar man i);;
let cst = Array.init 30 (fun i -> Rdd.cst man (float_of_int i));;

let f = Array.init 7 (fun i -> Bdd.ite var.(i) var.(i+1) var.(i+2));;

let rdd1 = Rdd.ite f.(0) cst.(0) cst.(1);;
let rdd2 = Rdd.ite f.(1) cst.(2) cst.(4);;

let rdda = Rdd.add rdd1 rdd2;;
let rddb = Rdd.map_op2 
  ~bottom1:(fun x -> None)
  ~bottom2:(fun x -> None) 
  ~neutral1:(fun x -> false) 
  ~neutral2:(fun x -> false) 
  ~commutative:true (fun x y -> Gc.compact (); x +. y)  rdd1 rdd2;;

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

testop f Rdd.add (Rdd.map_op2 ~bottom1:(fun x -> None) ~commutative:true (+.));;
testop g Rdd.add (Rdd.map_op2 ~commutative:true (+.));;
testop h Rdd.add (Rdd.map_op2 ~commutative:true (+.));;
testop f Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); x *. y));;
testop g Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); x *. y));;
testop h Rdd.mul (Rdd.map_op2 ~commutative:true (fun x y -> Gc.compact (); x *. y));;
