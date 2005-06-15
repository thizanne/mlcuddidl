(*i $Id: mtbdd.mli,v 1.2 2004/10/01 16:53:37 bjeannet Exp $ i*)

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
    external manager : t -> Manager.t = "camlidl_bdd_manager"
    external is_cst: t -> bool = "camlidl_bdd_is_cst"
    external topvar: t -> int = "camlidl_bdd_topvar"
    external dthen: t -> t = "camlidl_rdd_dthen"
    external delse: t -> t = "camlidl_rdd_delse"
    external cofactors: int -> t -> (t * t) = "camlidl_rdd_cofactors"
    external cofactor: t -> Bdd.t -> t = "camlidl_rdd_cofactor"
    val dval: t -> leaf
    val inspect: t -> mtbdd
    (* Supports *)
    external support: t -> Bdd.t = "camlidl_bdd_support"
    external supportsize: t -> int = "camlidl_bdd_supportsize"
    external is_var_in: int -> t -> bool = "camlidl_bdd_is_var_in"
    external vectorsupport : t array -> Bdd.t = "camlidl_bdd_vectorsupport"
    external vectorsupport2 : Bdd.t array -> t array -> Bdd.t = "camlidl_rdd_vectorsupport2"
    (* Classical operations *)
    val cst: Manager.t -> leaf -> t
    external ite: Bdd.t -> t -> t -> t = "camlidl_rdd_ite"
    external compose: int -> Bdd.t -> t -> t = "camlidl_rdd_compose"
    external vectorcompose: Bdd.t array -> t -> t = "camlidl_rdd_vectorcompose"
    (* Logical tests *)
    external is_equal : t -> t -> bool = "camlidl_bdd_is_equal"
    external is_equal_when: t -> t -> Bdd.t -> bool = "camlidl_bdd_is_equal_when"
    val is_eval_cst: t -> Bdd.t -> leaf option
    val is_ite_cst: Bdd.t -> t -> t -> leaf option
    (* Structural information *)
    external size : t -> int = "camlidl_bdd_size"
    external nbpaths : t -> float = "camlidl_bdd_nbpaths"
    external nbnonzeropaths : t -> float = "camlidl_rdd_nbnonzeropaths"
    external nbminterms : int -> t -> float = "camlidl_bdd_nbminterms"
    external density : int -> t -> float = "camlidl_bdd_density"
    external nbleaves : t -> int = "camlidl_rdd_nbleaves"
      
    (* Variable mapping *)
    external varmap : t -> t = "camlidl_rdd_varmap"  
    external permute : int array -> t -> t = "camlidl_rdd_permute"  

    (* Iterators *)
    external iter_node: (t -> unit) -> t -> unit = "camlidl_dd_iter_node"
    val iter_cube: (Manager.tbool array -> leaf -> unit) -> t -> unit 
    (* Leaves and guards *)
    external guard_of_node: t -> t -> Bdd.t = "camlidl_rdd_guard_of_node"
    external guard_of_nonbackground : t -> Bdd.t = "camlidl_rdd_guard_of_nonbackground"
    external nodes_below_level: t -> int option -> t array = "camlidl_rdd_nodes_below_level"
    val leaves: t -> leaf array
    val guard_of_leaf: t -> leaf -> Bdd.t
    val guardleafs: t -> (Bdd.t * leaf) array
    (* Minimizations *)
    external constrain: t -> Bdd.t -> t = "camlidl_rdd_constrain"
    external tdconstrain: t -> Bdd.t -> t = "camlidl_rdd_tdconstrain"
    external restrict: t -> Bdd.t -> t = "camlidl_rdd_restrict"
    external tdrestrict : t -> Bdd.t -> t = "camlidl_rdd_tdrestrict"
    (* Conversions *)
    external to_bdd : t -> Bdd.t = "camlidl_rdd_to_bdd"
   (* User operations *)
    val alloc_unop: (leaf -> leaf) -> id_unop
    val alloc_binop: (leaf -> leaf -> leaf) -> id_binop
    val alloc_combinop: (leaf -> leaf -> leaf) -> id_combinop
    external apply_unop: id_binop -> t -> t = "camlidl_idd_apply_unop"
    external apply_binop: id_binop -> t -> t -> t = "camlidl_idd_apply_binop"
    external apply_combinop: id_combinop -> t -> t -> t = "camlidl_idd_apply_combinop"
    (* Miscellaneous *) 
    external transfer : t -> Manager.t -> t = "camlidl_rdd_transfer"
    (* Printing *)
    external _print: t -> unit = "camlidl_dd_print"
    val print__minterm: Format.formatter -> t -> unit
    val print_minterm: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
    val print: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
  end

module Make : functor (Leaf: LeafType) -> (S with type leaf = Leaf.t)
