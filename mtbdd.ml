(*i $Id: mtbdd.ml,v 1.2 2004/10/01 16:53:37 bjeannet Exp $ i*)

(* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution. *)

open Format

module type LeafType =
  sig
    type t
    val equal: t -> t -> bool
    val hash: t -> int
    val background: t
  end

module type S = 
  sig
    type leaf
    type t
    type id_unop
    type id_binop
    type id_combinop
    type mtbdd = 
      | Leaf of leaf
      | Ite of int * t * t

    val leafbackground: leaf
	
    (* internal operations, not meant to be used by casual user *)
    module Hash: (Hashtbl.S with type key = leaf)
    val hasht: int Hash.t
    module Assoc: (Map.S with type key = int)
    val assoct: (leaf Assoc.t) ref

    val leaf_of_id: int -> leaf
    val id_of_leaf: leaf -> int
    val remove_leaf: leaf -> unit
    val iter_leaf : (int -> leaf -> unit) -> unit
    external to_idd: t -> Idd.t = "%identity"
    external of_idd: Idd.t -> t = "%identity"

    (* Extractors *)
    external manager : t -> Manager.t = "camlidl_cudd_bdd_manager"
    external is_cst: t -> bool = "camlidl_cudd_bdd_is_cst"
    external topvar: t -> int = "camlidl_cudd_bdd_topvar"
    external dthen: t -> t = "camlidl_cudd_rdd_dthen"
    external delse: t -> t = "camlidl_cudd_rdd_delse"
    external cofactors: int -> t -> (t * t) = "camlidl_cudd_rdd_cofactors"
    external cofactor: t -> Bdd.t -> t = "camlidl_cudd_rdd_cofactor"
    val dval: t -> leaf
    val inspect: t -> mtbdd
    (* Supports *)
    external support: t -> Bdd.t = "camlidl_cudd_bdd_support"
    external supportsize: t -> int = "camlidl_cudd_bdd_supportsize"
    external is_var_in: int -> t -> bool = "camlidl_cudd_bdd_is_var_in"
    external vectorsupport : t array -> Bdd.t = "camlidl_cudd_bdd_vectorsupport"
    external vectorsupport2 : Bdd.t array -> t array -> Bdd.t = "camlidl_cudd_rdd_vectorsupport2"
    (* Classical operations *)
    val cst: Manager.t -> leaf -> t
    external ite: Bdd.t -> t -> t -> t = "camlidl_cudd_rdd_ite"
    external ite_cst: Bdd.t -> t -> t -> t = "camlidl_cudd_rdd_ite_cst"
    external eval_cst : t -> Bdd.t -> t option = "camlidl_cudd_rdd_eval_cst"
    external compose: int -> Bdd.t -> t -> t = "camlidl_cudd_rdd_compose"
    external vectorcompose: Bdd.t array -> t -> t = "camlidl_cudd_rdd_vectorcompose"
    (* Logical tests *)
    external is_equal : t -> t -> bool = "camlidl_cudd_bdd_is_equal"
    external is_equal_when: t -> t -> Bdd.t -> bool = "camlidl_cudd_bdd_is_equal_when"
    val is_eval_cst: t -> Bdd.t -> leaf option
    val is_ite_cst: Bdd.t -> t -> t -> leaf option
    (* Structural information *)
    external size : t -> int = "camlidl_cudd_bdd_size"
    external nbpaths : t -> float = "camlidl_cudd_bdd_nbpaths"
    external nbnonzeropaths : t -> float = "camlidl_cudd_rdd_nbnonzeropaths"
    external nbminterms : int -> t -> float = "camlidl_cudd_bdd_nbminterms"
    external density : int -> t -> float = "camlidl_cudd_bdd_density"
    external nbleaves : t -> int = "camlidl_cudd_rdd_nbleaves"
      
    (* Variable mapping *)
    external varmap : t -> t = "camlidl_cudd_rdd_varmap"  
    external permute : int array -> t -> t = "camlidl_cudd_rdd_permute"  

    (* Iterators *)
    external iter_node: (t -> unit) -> t -> unit = "camlidl_cudd_iter_node"
    val iter_cube: (Manager.tbool array -> leaf -> unit) -> t -> unit 
    (* Leaves and guards *)
    external guard_of_node: t -> t -> Bdd.t = "camlidl_cudd_rdd_guard_of_node"
    external guard_of_nonbackground : t -> Bdd.t = "camlidl_cudd_rdd_guard_of_nonbackground"
    external nodes_below_level: t -> int option -> t array = "camlidl_cudd_rdd_nodes_below_level"
    val leaves: t -> leaf array
    val guard_of_leaf: t -> leaf -> Bdd.t
    val guardleafs: t -> (Bdd.t * leaf) array
    (* Minimizations *)
    external constrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_constrain"
    external tdconstrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_tdconstrain"
    external restrict: t -> Bdd.t -> t = "camlidl_cudd_rdd_restrict"
    external tdrestrict : t -> Bdd.t -> t = "camlidl_cudd_rdd_tdrestrict"
    (* Conversions *)
    external to_bdd : t -> Bdd.t = "camlidl_cudd_rdd_to_bdd"
   (* User operations *)
    val mapleaf1 : (Bdd.t -> leaf -> leaf) -> t -> t
    val mapleaf2 : (Bdd.t -> leaf -> leaf -> leaf) -> t -> t -> t

    val mapunop : (leaf -> leaf) -> t -> t
    val mapbinop : commutative:bool -> (leaf -> leaf -> leaf) -> t -> t -> t
    val mapterop : (leaf -> leaf -> leaf -> leaf) -> t -> t -> t -> t

    val alloc_unop: (leaf -> leaf) -> id_unop
    val alloc_binop: (leaf -> leaf -> leaf) -> id_binop
    val alloc_combinop: (leaf -> leaf -> leaf) -> id_combinop
    external apply_unop: id_unop -> t -> t = "camlidl_cudd_idd_apply_unop"
    external apply_binop: id_binop -> t -> t -> t = "camlidl_cudd_idd_apply_binop"
    external apply_combinop: id_combinop -> t -> t -> t = "camlidl_cudd_idd_apply_combinop"
    (* Miscellaneous *) 
    external transfer : t -> Manager.t -> t = "camlidl_cudd_rdd_transfer"
    (* Printing *)
    external _print: t -> unit = "camlidl_cudd_print"
    val print__minterm: Format.formatter -> t -> unit
    val print_minterm: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
    val print: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
end

module Make (Leaf : LeafType) = 
  struct
    type leaf = Leaf.t
    type t = Idd.t
    type id_unop = Idd.id_unop
    type id_binop = Idd.id_binop
    type id_combinop = Idd.id_combinop
    type mtbdd = 
      | Leaf of leaf
      | Ite of int * t * t

    (* Here we define the correspondance between leaves and integers,
       the inverse correspondance and the counter of registered
       leaves. *) 
    module Hash = Hashtbl.Make(Leaf)
	
    let (hasht : int Hash.t) = Hash.create 101
    module Assoc = Map.Make(
      struct
	type t = int
	let compare = (-)
      end
     )
    let (assoct : Leaf.t Assoc.t ref) = ref Assoc.empty

    let leafbackground = Leaf.background
    let _ =
      Hash.add hasht leafbackground 0;
      assoct := Assoc.add 0 leafbackground !assoct
	
    let index = ref 1
	
    (* We now define the conversion functions. *)
    let leaf_of_id id = (*r no catching of [Not_found] here *)
      try 
	Assoc.find id !assoct
      with Not_found ->
	failwith ("module Mtbdd.Make().leaf_of_id "
		  ^(string_of_int id)^": no associated object !")
    let id_of_leaf l = (*r We register the leaf if it doesn not exist *)
      try
	Hash.find hasht l
      with Not_found ->
	Hash.add hasht l !index;
	assoct := Assoc.add !index l !assoct;
	incr index;
	(!index - 1)
    let remove_leaf leaf =
      let id = id_of_leaf leaf in
      Hash.remove hasht leaf;
      assoct := Assoc.remove id !assoct
    let iter_leaf f = Assoc.iter f !assoct

    external to_idd: t -> Idd.t = "%identity"
    external of_idd: Idd.t -> t = "%identity"

    (* We are now ready to adapt needed operations. *)

    (* structural tests and extractors *)
    external manager : t -> Manager.t = "camlidl_cudd_bdd_manager"
    external is_cst: t -> bool = "camlidl_cudd_bdd_is_cst"
    external topvar: t -> int = "camlidl_cudd_bdd_topvar"
    external dthen: t -> t = "camlidl_cudd_rdd_dthen"
    external delse: t -> t = "camlidl_cudd_rdd_delse"
    external cofactors: int -> t -> (t * t) = "camlidl_cudd_rdd_cofactors"
    external cofactor: t -> Bdd.t -> t = "camlidl_cudd_rdd_cofactor"
    let dval t = leaf_of_id (Idd.dval t)
    let inspect t = 
      match (Idd.inspect t) with
      | Idd.Leaf(id) -> Leaf (leaf_of_id id)
      | Idd.Ite(i,t,e) -> Ite(i,t,e)
    (* variables and support *)
    external support: t -> Bdd.t = "camlidl_cudd_bdd_support"
    external supportsize: t -> int = "camlidl_cudd_bdd_supportsize"
    external is_var_in: int -> t -> bool = "camlidl_cudd_bdd_is_var_in"
    external vectorsupport : t array -> Bdd.t = "camlidl_cudd_bdd_vectorsupport"
    external vectorsupport2 : Bdd.t array -> t array -> Bdd.t = "camlidl_cudd_rdd_vectorsupport2"
    (* classical operations *)
    let cst man l = Idd.cst man (id_of_leaf l)
    external ite: Bdd.t -> t -> t -> t = "camlidl_cudd_rdd_ite"
    external ite_cst: Bdd.t -> t -> t -> t = "camlidl_cudd_rdd_ite_cst"
    external eval_cst : t -> Bdd.t -> t option = "camlidl_cudd_rdd_eval_cst"
    external compose: int -> Bdd.t -> t -> t = "camlidl_cudd_rdd_compose"
    external vectorcompose: Bdd.t array -> t -> t = "camlidl_cudd_rdd_vectorcompose"
    (* tests *)
    external is_equal : t -> t -> bool = "camlidl_cudd_bdd_is_equal"
    external is_equal_when: t -> t -> Bdd.t -> bool = "camlidl_cudd_bdd_is_equal_when"
    let is_eval_cst t bdd =
      match Idd.is_eval_cst t bdd with
      | None -> None
      | Some(id) -> Some (leaf_of_id id)
    let is_ite_cst f g h =
      match Idd.is_ite_cst f g h with
      | None -> None
      | Some(id) -> Some (leaf_of_id id)
    external size : t -> int = "camlidl_cudd_bdd_size"
    external nbpaths : t -> float = "camlidl_cudd_bdd_nbpaths"
    external nbnonzeropaths : t -> float = "camlidl_cudd_rdd_nbnonzeropaths"
    external nbminterms : int -> t -> float = "camlidl_cudd_bdd_nbminterms"
    external density : int -> t -> float = "camlidl_cudd_bdd_density"
    external nbleaves : t -> int = "camlidl_cudd_rdd_nbleaves"
      
    (* variable mapping *)
    external varmap : t -> t = "camlidl_cudd_rdd_varmap"  
    external permute : int array -> t -> t = "camlidl_cudd_rdd_permute"  

    (* Iterators *)
    external iter_node: (t -> unit) -> t -> unit = "camlidl_cudd_iter_node"
    let iter_cube f t =
      Idd.iter_cube (fun cube id -> f cube (leaf_of_id id)) t
    (* Leaves and guards *)
    external guard_of_node: t -> t -> Bdd.t = "camlidl_cudd_rdd_guard_of_node"
    external guard_of_nonbackground : t -> Bdd.t = "camlidl_cudd_rdd_guard_of_nonbackground"
    external nodes_below_level: t -> int option -> t array = "camlidl_cudd_rdd_nodes_below_level"
    let leaves t = Array.map leaf_of_id (Idd.leaves t)
    let guard_of_leaf t l = Idd.guard_of_leaf t (id_of_leaf l)
    let guardleafs rdd =
      let tab = Idd.leaves rdd in
      Array.map (fun id -> (Idd.guard_of_leaf rdd id, leaf_of_id id)) tab
    (* Minimizations *)
    external constrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_constrain"
    external tdconstrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_tdconstrain"
    external restrict: t -> Bdd.t -> t = "camlidl_cudd_rdd_restrict"
    external tdrestrict : t -> Bdd.t -> t = "camlidl_cudd_rdd_tdrestrict"
    (* User operations *)
    let mapleaf1 f t =
      Idd.mapleaf1
	(fun bdd id -> id_of_leaf (f bdd (leaf_of_id id)))
	t
    let mapleaf2 f t1 t2 =
      Idd.mapleaf2
	(fun bdd id1 id2 -> id_of_leaf (f bdd (leaf_of_id id1) (leaf_of_id id2)))
	t1 t2

    let wrap_binop leaf_op =
      let id_op = 
	fun idx idy ->
	  id_of_leaf (leaf_op (leaf_of_id idx) (leaf_of_id idy))
      in
      id_op

    let mapunop lop f = 
      let op = 
	fun id -> 
	  id_of_leaf (lop (leaf_of_id id))
      in
      Idd.mapunop op f

    let mapbinop ~commutative lop f g = 
      let op = wrap_binop lop in
      Idd.mapbinop ~commutative op f g 

    let mapterop lop f g h = 
      let op = fun idx idy idz ->
	  id_of_leaf (lop (leaf_of_id idx) (leaf_of_id idy) (leaf_of_id idz))
      in
      Idd.mapterop op f g h 

    let alloc_unop lop =
      let op = 
	fun id -> 
	  id_of_leaf (lop (leaf_of_id id)) 
      in
      Idd.alloc_unop op
    let alloc_binop lop =
      let op = wrap_binop lop in
      Idd.alloc_binop op
    let alloc_combinop lop =
      let op = wrap_binop lop in
      Idd.alloc_combinop op

    external apply_unop: id_unop -> t -> t = "camlidl_cudd_idd_apply_unop"
    external apply_binop: id_binop -> t -> t -> t = "camlidl_cudd_idd_apply_binop"
    external apply_combinop: id_combinop -> t -> t -> t = "camlidl_cudd_idd_apply_combinop"
    (* Miscellaneous *) 
    external to_bdd : t -> Bdd.t = "camlidl_cudd_rdd_to_bdd"
    external transfer : t -> Manager.t -> t = "camlidl_cudd_rdd_transfer"

    (* Printing *)
    external _print: t -> unit = "camlidl_cudd_print"
    let print__minterm = Idd.print__minterm
    let print_minterm bassoc lassoc formatter rdd =
      Idd.print_minterm 
	bassoc
	(fun id -> lassoc (leaf_of_id id))
	formatter
	rdd
	
    let print bassoc lassoc formatter rdd = 
      Idd.print
	bassoc
	(fun id -> lassoc (leaf_of_id id))
	formatter
	rdd
end

