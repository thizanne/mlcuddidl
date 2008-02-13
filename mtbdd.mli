(*i $Id: mtbdd.mli,v 1.2 2004/10/01 16:53:37 bjeannet Exp $ i*)

(* This file is part of the MLCUDDIDL Library, released under LGPL license.
   Please read the COPYING file packaged in the distribution. *)

(** Multi-Terminal Binary Decision Diagrams *)

(*  ====================================================================== *)
(** {2 Type of the parameter module} *)
(*  ====================================================================== *)

module type LeafType =
  sig
    type t
      (** Type of a leaf *)
    val equal: t -> t -> bool
      (** Equality function on leaves *)
    val hash: t -> int
      (** Hashing function on leaves *)
    val background: t
      (** Background leaf value 
	(typically, dummy value not encountered for normal use *)
  end

(*  ====================================================================== *)
(** {2 Type of generated module} *)
(*  ====================================================================== *)

module type S = 
  sig
    type leaf
      (** Type of a leaf *)

    type t
      (** Type of MTBDD with leaves of type [leaf] *)

    type id_unop
    type id_binop
    type id_combinop
      (** For user-defined operations *)

    type mtbdd = 
      | Leaf of leaf
      | Ite of int * t * t

    val leafbackground: leaf
	
    module Hash: (Hashtbl.S with type key = leaf)
    val hasht: int Hash.t
    module Assoc: (Map.S with type key = int)
    val assoct: (leaf Assoc.t) ref
      (** Internal, not meant to be used by casual user *)

    val leaf_of_id: int -> leaf
    val id_of_leaf: leaf -> int
    val remove_leaf: leaf -> unit
    val iter_leaf : (int -> leaf -> unit) -> unit
    external to_idd: t -> Idd.t = "%identity"
    external of_idd: Idd.t -> t = "%identity"

    (** {3 Extractors} *)

    external manager : t -> Manager.t = "camlidl_cudd_bdd_manager"
    external is_cst: t -> bool = "camlidl_cudd_bdd_is_cst"
    external topvar: t -> int = "camlidl_cudd_bdd_topvar"
    external dthen: t -> t = "camlidl_cudd_rdd_dthen"
    external delse: t -> t = "camlidl_cudd_rdd_delse"
    external cofactors: int -> t -> (t * t) = "camlidl_cudd_rdd_cofactors"
    external cofactor: t -> Bdd.t -> t = "camlidl_cudd_rdd_cofactor"
    val dval: t -> leaf
    val inspect: t -> mtbdd

    (** {3 Supports} *)

    external support: t -> Bdd.t = "camlidl_cudd_bdd_support"
    external supportsize: t -> int = "camlidl_cudd_bdd_supportsize"
    external is_var_in: int -> t -> bool = "camlidl_cudd_bdd_is_var_in"
    external vectorsupport : t array -> Bdd.t = "camlidl_cudd_bdd_vectorsupport"
    external vectorsupport2 : Bdd.t array -> t array -> Bdd.t = "camlidl_cudd_rdd_vectorsupport2"

    (** {3 Classical operations} *)

    val cst: Manager.t -> leaf -> t
    external ite: Bdd.t -> t -> t -> t = "camlidl_cudd_rdd_ite"
    external compose: int -> Bdd.t -> t -> t = "camlidl_cudd_rdd_compose"
    external vectorcompose: Bdd.t array -> t -> t = "camlidl_cudd_rdd_vectorcompose"

    (** {3 Logical tests} *)

    external is_equal : t -> t -> bool = "camlidl_cudd_bdd_is_equal"
    external is_equal_when: t -> t -> Bdd.t -> bool = "camlidl_cudd_bdd_is_equal_when"
    val is_eval_cst: t -> Bdd.t -> leaf option
    val is_ite_cst: Bdd.t -> t -> t -> leaf option

    (** {3 Structural information} *)

    external size : t -> int = "camlidl_cudd_bdd_size"
    external nbpaths : t -> float = "camlidl_cudd_bdd_nbpaths"
    external nbnonzeropaths : t -> float = "camlidl_cudd_rdd_nbnonzeropaths"
    external nbminterms : int -> t -> float = "camlidl_cudd_bdd_nbminterms"
    external density : int -> t -> float = "camlidl_cudd_bdd_density"
    external nbleaves : t -> int = "camlidl_cudd_rdd_nbleaves"
      
    (** {3 Variable mapping} *)

    external varmap : t -> t = "camlidl_cudd_rdd_varmap"  
    external permute : int array -> t -> t = "camlidl_cudd_rdd_permute"  

    (** {3 Iterators} *)

    external iter_node: (t -> unit) -> t -> unit = "camlidl_cudd_iter_node"
    val iter_cube: (Manager.tbool array -> leaf -> unit) -> t -> unit 

    (** {3 Leaves and guards} *)

    external guard_of_node: t -> t -> Bdd.t = "camlidl_cudd_rdd_guard_of_node"
    external guard_of_nonbackground : t -> Bdd.t = "camlidl_cudd_rdd_guard_of_nonbackground"
    external nodes_below_level: t -> int option -> t array = "camlidl_cudd_rdd_nodes_below_level"
    val leaves: t -> leaf array
    val guard_of_leaf: t -> leaf -> Bdd.t
    val guardleafs: t -> (Bdd.t * leaf) array

    (** {3 Minimizations} *)

    external constrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_constrain"
    external tdconstrain: t -> Bdd.t -> t = "camlidl_cudd_rdd_tdconstrain"
    external restrict: t -> Bdd.t -> t = "camlidl_cudd_rdd_restrict"
    external tdrestrict : t -> Bdd.t -> t = "camlidl_cudd_rdd_tdrestrict"

    (** {3 Conversions} *)

    external to_bdd : t -> Bdd.t = "camlidl_cudd_rdd_to_bdd"

    (** {3 User operations} *)

    val mapleaf1 : (Bdd.t -> leaf -> leaf) -> t -> t
    val mapleaf2 : (Bdd.t -> leaf -> leaf -> leaf) -> t -> t -> t

    val alloc_unop: (leaf -> leaf) -> id_unop
    val alloc_binop: (leaf -> leaf -> leaf) -> id_binop
    val alloc_combinop: (leaf -> leaf -> leaf) -> id_combinop
    external apply_unop: id_unop -> t -> t = "camlidl_cudd_idd_apply_unop"
    external apply_binop: id_binop -> t -> t -> t = "camlidl_cudd_idd_apply_binop"
    external apply_combinop: id_combinop -> t -> t -> t = "camlidl_cudd_idd_apply_combinop"

    (** {3 Miscellaneous} *) 

    external transfer : t -> Manager.t -> t = "camlidl_cudd_rdd_transfer"

    (** {3 Printing} *)

    external _print: t -> unit = "camlidl_cudd_print"
    val print__minterm: Format.formatter -> t -> unit
    val print_minterm: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
    val print: (int -> string) -> (leaf -> string) -> Format.formatter -> t -> unit
  end

(*  ====================================================================== *)
(** {2 Functor} *)
(*  ====================================================================== *)

module Make : functor (Leaf: LeafType) -> (S with type leaf = Leaf.t)
