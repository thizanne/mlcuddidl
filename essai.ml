open Format;;
(*
#install_printer Idd.print__minterm;;
*)
let man = Manager.make 4 4 0 0 0;;

let var = Array.init 4 (fun i -> Bdd.ithvar man i);;
let cst = Array.init 6 (fun i -> Idd.cst man i);;

let exprim1 () =
  let f = Bdd.dnot(Bdd.eq var.(0) var.(1)) in
  let g = Bdd.dand var.(2) var.(3) in
  let h = Bdd.dor f g in 
  let add1 = Idd.ite h cst.(2) cst.(3) in
  printf "add1 = %a@."
    Idd.print__minterm add1
  ;
  let add2 = Idd.mapunop (fun v -> v+10) add1 in
  printf "add2 = %a@."
    Idd.print__minterm add2
  ;
  add2
;;
let exprim2 () =
  let f = Bdd.dnot(Bdd.eq var.(0) var.(1)) in
  let g = Bdd.dand var.(2) var.(3) in
  let h = Bdd.dor f g in
  let add1 = Idd.ite h cst.(2) cst.(3) in
  let add2 = Idd.ite f cst.(4) cst.(5) in
  printf "add1 = %a@.add2 = %a@."
    Idd.print__minterm add1
    Idd.print__minterm add2
  ;
  let add3 = Idd.mapbinop ~commutative:true (fun x y -> x*y) add1 add2 in
  printf "add3 = %a@."
    Idd.print__minterm add3
  ;
  add3
;;

let add1 = exprim1();;
let add2 = exprim2();;
(*
Gc.compact();;
Manager.debugcheck man;;
Manager.check_keys man;;
Manager.reduce_heap man Manager.REORDER_SIFT 1;;
*)

(*
let gc b () =
  Format.pp_print_string Format.std_formatter 
    (if b then "reordering" else "gc");
  Format.printf "@.";
  Gc.full_major()

let f man size =
  let f = ref (Bdd.dtrue man) in
  for i=0 to size-1 do
    let g = Bdd.eq (Bdd.ithvar man i) (Bdd.ithvar man (i+2*size)) in
    f := Bdd.dand !f g;
  done;
  Format.printf "DONE1 %i@." (Bdd.size !f);
  Manager.reduce_heap man Manager.REORDER_GROUP_SIFT_CONV 10;
  Format.printf "DONE1 %i@." (Bdd.size !f);
  for i=0 to size-1 do
    let g = Bdd.xor (Bdd.ithvar man (i)) (Bdd.ithvar man (i+size)) in
    f := Bdd.dand !f g;
  done;
  Format.printf "DONE2 %i@." (Bdd.size !f);
  Manager.reduce_heap man Manager.REORDER_GROUP_SIFT_CONV 10;
  Format.printf "DONE2 %i@." (Bdd.size !f);
  for i=0 to size-1 do
    let g = Bdd.xor (Bdd.ithvar man (i+size)) (Bdd.ithvar man (i+3*size)) in
    f := Bdd.dand !f g;
  done;
  Format.printf "DONE3 %i@." (Bdd.size !f);
  Manager.reduce_heap man Manager.REORDER_GROUP_SIFT_CONV 10;
  Format.printf "DONE3 %i@." (Bdd.size !f);
  !f

let g () = 
  let _ = 
    let man = Manager.make 200 0 0 0 500000000 in
    Manager.set_gc 500000000 (gc false) (gc true);
    let bdd1 = f man 20 in
    let bdd2 = f man 20 in
    ()
  in
  Gc.full_major();
  Format.printf "@.";
  ()
;;
g ();
g ();
Format.printf "@.";;
*)
