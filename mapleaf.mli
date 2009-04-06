(** Lifting operation on leaves to operations on MTBDDs *)

val restrict : bool ref
val combineretractive : Man.v Bdd.t * 'a -> 'a Vdd.t -> 'a Vdd.t
val combineexpansive :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t) -> Man.v Bdd.t * 'a -> 'b Vdd.t -> 'c Vdd.t
val combineleaf1 :
  default:'a Vdd.t ->
  combine:(Man.v Bdd.t * 'c -> 'a Vdd.t -> 'a Vdd.t) ->
  (Man.v Bdd.t -> 'b -> Man.v Bdd.t * 'c) -> 'b Vdd.t -> 'a Vdd.t
val retractivemapleaf1 :
  default:'a Vdd.t ->
  (Man.v Bdd.t -> 'b -> Man.v Bdd.t * 'a) -> 'b Vdd.t -> 'a Vdd.t
val expansivemapleaf1 :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'a Vdd.t -> 'a Vdd.t) ->
  (Man.v Bdd.t -> 'b -> Man.v Bdd.t * 'a) -> 'b Vdd.t -> 'a Vdd.t
val mapleaf1 : ('a -> 'b) -> 'a Vdd.t -> 'b Vdd.t
val combineleaf2 :
  default:'a Vdd.t ->
  combine:(Man.v Bdd.t * 'd -> 'a Vdd.t -> 'a Vdd.t) ->
  (Man.v Bdd.t -> 'b -> 'c -> Man.v Bdd.t * 'd) ->
  'b Vdd.t -> 'c Vdd.t -> 'a Vdd.t
val retractivemapleaf2 :
  default:'a Vdd.t ->
  (Man.v Bdd.t -> 'b -> 'c -> Man.v Bdd.t * 'a) ->
  'b Vdd.t -> 'c Vdd.t -> 'a Vdd.t
val expansivemapleaf2 :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'a Vdd.t -> 'a Vdd.t) ->
  (Man.v Bdd.t -> 'b -> 'c -> Man.v Bdd.t * 'a) ->
  'b Vdd.t -> 'c Vdd.t -> 'a Vdd.t
val mapleaf2 : ('a -> 'b -> 'c) -> 'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t
val combineleaf_array :
  default:'a Vdd.t ->
  combine:(Man.v Bdd.t * 'c -> 'a Vdd.t -> 'a Vdd.t) ->
  tabsorbant:('b -> bool) option array ->
  (Man.v Bdd.t -> 'b array -> Man.v Bdd.t * 'c) -> 'b Vdd.t array -> 'a Vdd.t
val combineleaf1_array :
  default:'a Vdd.t ->
  combine:(Man.v Bdd.t * 'd -> 'a Vdd.t -> 'a Vdd.t) ->
  ?absorbant:('b -> bool) ->
  tabsorbant:('c -> bool) option array ->
  (Man.v Bdd.t -> 'b -> 'c array -> Man.v Bdd.t * 'd) ->
  'b Vdd.t -> 'c Vdd.t array -> 'a Vdd.t
val combineleaf2_array :
  default:'a Vdd.t ->
  combine:(Man.v Bdd.t * 'e -> 'a Vdd.t -> 'a Vdd.t) ->
  ?absorbant1:('b -> bool) ->
  ?absorbant2:('c -> bool) ->
  tabsorbant:('d -> bool) option array ->
  (Man.v Bdd.t -> 'b -> 'c -> 'd array -> Man.v Bdd.t * 'e) ->
  'b Vdd.t -> 'c Vdd.t -> 'd Vdd.t array -> 'a Vdd.t
