(** Lifting operation on leaves to operations on MTBDDs *)

val restrict : bool ref
val combineretractive : Man.v Bdd.t * 'a -> 'a Vdd.t -> 'a Vdd.t
val combineexpansive :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'b Vdd.t -> 'c Vdd.t) -> Man.v Bdd.t * 'a -> 'b Vdd.t -> 'c Vdd.t
val combineleaf1 :
  default:'c ->
  combine:('b -> 'c -> 'c) ->
  (Bdd.vt -> 'a -> 'b) -> 'a Vdd.t -> 'c
val retractivemapleaf1 :
  default:'a Vdd.t ->
  (Bdd.vt -> 'b -> Bdd.vt * 'a) -> 'b Vdd.t -> 'a Vdd.t
val expansivemapleaf1 :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'a Vdd.t -> 'a Vdd.t) ->
  (Bdd.vt -> 'b -> Bdd.vt * 'a) -> 'b Vdd.t -> 'a Vdd.t
val mapleaf1 : ('a -> 'b) -> 'a Vdd.t -> 'b Vdd.t
val combineleaf2 :
  default:'d ->
  combine:('c -> 'd -> 'd) ->
  (Bdd.vt -> 'a -> 'b -> 'c) ->
  'a Vdd.t -> 'b Vdd.t -> 'd
val retractivemapleaf2 :
  default:'a Vdd.t ->
  (Bdd.vt -> 'b -> 'c -> Bdd.vt * 'a) ->
  'b Vdd.t -> 'c Vdd.t -> 'a Vdd.t
val expansivemapleaf2 :
  default:'a Vdd.t ->
  merge:('a Vdd.t -> 'a Vdd.t -> 'a Vdd.t) ->
  (Bdd.vt -> 'b -> 'c -> Bdd.vt * 'a) ->
  'b Vdd.t -> 'c Vdd.t -> 'a Vdd.t
val mapleaf2 : ('a -> 'b -> 'c) -> 'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t
val combineleaf_array :
  default:'c ->
  combine:('b -> 'c -> 'c) ->
  tabsorbant:('a -> bool) option array ->
  (Bdd.vt -> 'a array -> 'b) -> 'a Vdd.t array -> 'c
val combineleaf1_array :
  default:'d ->
  combine:('c -> 'd -> 'd) ->
  ?absorbant:('a -> bool) ->
  tabsorbant:('b -> bool) option array ->
  (Bdd.vt -> 'a -> 'b array -> 'c) ->
  'a Vdd.t -> 'b Vdd.t array -> 'd
val combineleaf2_array :
  default:'e ->
  combine:('d -> 'e -> 'e) ->
  ?absorbant1:('a -> bool) ->
  ?absorbant2:('b -> bool) ->
  tabsorbant:('c -> bool) option array ->
  (Bdd.vt -> 'a -> 'b -> 'c array -> 'd) ->
  'a Vdd.t -> 'b Vdd.t -> 'c Vdd.t array -> 'e
