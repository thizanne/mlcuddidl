let man = Manager.make 10 10 0 0 0;;
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
Idd.guard_of_nonbackground idd1;;
Idd.nbpaths idd1;;
Idd.nbnonzeropaths idd1;;
Idd.nbminterms 4 idd1;;
Idd.nbleaves idd1;;
Idd.leaves idd1;;

let res = Bdd.cube_union f g;;
let res = Bdd.cube_union (Bdd.dnot f) g;;
let z = Bdd.nxor x y;;
let z = Bdd.xor x y;;



let _ = 
  let var = Array.init 1000 (fun i -> Bdd.ithvar Manager.dummy i) in
  let tab = Array.init 999 (fun i -> Bdd.dand var.(i) var.(i+1)) in
  ()
