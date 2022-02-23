.. |op0| replace:: cxx_add_assign_to_sub_assign
.. |desc0| replace:: Replaces += with -=
.. |op1| replace:: cxx_add_to_sub
.. |desc1| replace:: Replaces + with -
.. |op2| replace:: cxx_and_assign_to_or_assign
.. |desc2| replace:: Replaces &= with \|=
.. |op3| replace:: cxx_and_to_or
.. |desc3| replace:: Replaces & with |
.. |op4| replace:: cxx_assign_const
.. |desc4| replace:: Replaces 'a = b' with 'a = 42'
.. |op5| replace:: cxx_bitwise_not_to_noop
.. |desc5| replace:: Replaces ~x with x
.. |op6| replace:: cxx_div_assign_to_mul_assign
.. |desc6| replace:: Replaces /= with \*=
.. |op7| replace:: cxx_div_to_mul
.. |desc7| replace:: Replaces / with *
.. |op8| replace:: cxx_eq_to_ne
.. |desc8| replace:: Replaces == with !=
.. |op9| replace:: cxx_ge_to_gt
.. |desc9| replace:: Replaces >= with >
.. |op10| replace:: cxx_ge_to_lt
.. |desc10| replace:: Replaces >= with <
.. |op11| replace:: cxx_gt_to_ge
.. |desc11| replace:: Replaces > with >=
.. |op12| replace:: cxx_gt_to_le
.. |desc12| replace:: Replaces > with <=
.. |op13| replace:: cxx_init_const
.. |desc13| replace:: Replaces 'T a = b' with 'T a = 42'
.. |op14| replace:: cxx_le_to_gt
.. |desc14| replace:: Replaces <= with >
.. |op15| replace:: cxx_le_to_lt
.. |desc15| replace:: Replaces <= with <
.. |op16| replace:: cxx_logical_and_to_or
.. |desc16| replace:: Replaces && with ||
.. |op17| replace:: cxx_logical_or_to_and
.. |desc17| replace:: Replaces || with &&
.. |op18| replace:: cxx_lshift_assign_to_rshift_assign
.. |desc18| replace:: Replaces <<= with >>=
.. |op19| replace:: cxx_lshift_to_rshift
.. |desc19| replace:: Replaces << with >>
.. |op20| replace:: cxx_lt_to_ge
.. |desc20| replace:: Replaces < with >=
.. |op21| replace:: cxx_lt_to_le
.. |desc21| replace:: Replaces < with <=
.. |op22| replace:: cxx_minus_to_noop
.. |desc22| replace:: Replaces -x with x
.. |op23| replace:: cxx_mul_assign_to_div_assign
.. |desc23| replace:: Replaces \*= with /=
.. |op24| replace:: cxx_mul_to_div
.. |desc24| replace:: Replaces * with /
.. |op25| replace:: cxx_ne_to_eq
.. |desc25| replace:: Replaces != with ==
.. |op26| replace:: cxx_or_assign_to_and_assign
.. |desc26| replace:: Replaces \|= with &=
.. |op27| replace:: cxx_or_to_and
.. |desc27| replace:: Replaces | with &
.. |op28| replace:: cxx_post_dec_to_post_inc
.. |desc28| replace:: Replaces x-- with x++
.. |op29| replace:: cxx_post_inc_to_post_dec
.. |desc29| replace:: Replaces x++ with x--
.. |op30| replace:: cxx_pre_dec_to_pre_inc
.. |desc30| replace:: Replaces --x with ++x
.. |op31| replace:: cxx_pre_inc_to_pre_dec
.. |desc31| replace:: Replaces ++x with --x
.. |op32| replace:: cxx_rem_assign_to_div_assign
.. |desc32| replace:: Replaces %= with /=
.. |op33| replace:: cxx_rem_to_div
.. |desc33| replace:: Replaces % with /
.. |op34| replace:: cxx_remove_negation
.. |desc34| replace:: Replaces !a with a
.. |op35| replace:: cxx_remove_void_call
.. |desc35| replace:: Removes calls to a function returning void
.. |op36| replace:: cxx_replace_scalar_call
.. |desc36| replace:: Replaces call to a function with 42
.. |op37| replace:: cxx_rshift_assign_to_lshift_assign
.. |desc37| replace:: Replaces >>= with <<=
.. |op38| replace:: cxx_rshift_to_lshift
.. |desc38| replace:: Replaces << with >>
.. |op39| replace:: cxx_sub_assign_to_add_assign
.. |desc39| replace:: Replaces -= with +=
.. |op40| replace:: cxx_sub_to_add
.. |desc40| replace:: Replaces - with +
.. |op41| replace:: cxx_xor_assign_to_or_assign
.. |desc41| replace:: Replaces ^= with \|=
.. |op42| replace:: cxx_xor_to_or
.. |desc42| replace:: Replaces ^ with |
.. |op43| replace:: negate_mutator
.. |desc43| replace:: Negates conditionals !x to x and x to !x
.. |op44| replace:: scalar_value_mutator
.. |desc44| replace:: Replaces zeros with 42, and non-zeros with 0


============= ==================
Operator Name Operator Semantics
============= ==================
|op0|         |desc0|
|op1|         |desc1|
|op2|         |desc2|
|op3|         |desc3|
|op4|         |desc4|
|op5|         |desc5|
|op6|         |desc6|
|op7|         |desc7|
|op8|         |desc8|
|op9|         |desc9|
|op10|        |desc10|
|op11|        |desc11|
|op12|        |desc12|
|op13|        |desc13|
|op14|        |desc14|
|op15|        |desc15|
|op16|        |desc16|
|op17|        |desc17|
|op18|        |desc18|
|op19|        |desc19|
|op20|        |desc20|
|op21|        |desc21|
|op22|        |desc22|
|op23|        |desc23|
|op24|        |desc24|
|op25|        |desc25|
|op26|        |desc26|
|op27|        |desc27|
|op28|        |desc28|
|op29|        |desc29|
|op30|        |desc30|
|op31|        |desc31|
|op32|        |desc32|
|op33|        |desc33|
|op34|        |desc34|
|op35|        |desc35|
|op36|        |desc36|
|op37|        |desc37|
|op38|        |desc38|
|op39|        |desc39|
|op40|        |desc40|
|op41|        |desc41|
|op42|        |desc42|
|op43|        |desc43|
|op44|        |desc44|
============= ==================


Groups:
    :all:	cxx_all, experimental

    :cxx_all:	cxx_assignment, cxx_increment, cxx_decrement, cxx_arithmetic, cxx_comparison, cxx_boundary, cxx_bitwise, cxx_calls

    :cxx_arithmetic:	cxx_minus_to_noop, cxx_add_to_sub, cxx_sub_to_add, cxx_mul_to_div, cxx_div_to_mul, cxx_rem_to_div

    :cxx_arithmetic_assignment:	cxx_add_assign_to_sub_assign, cxx_sub_assign_to_add_assign, cxx_mul_assign_to_div_assign, cxx_div_assign_to_mul_assign, cxx_rem_assign_to_div_assign

    :cxx_assignment:	cxx_bitwise_assignment, cxx_arithmetic_assignment, cxx_const_assignment

    :cxx_bitwise:	cxx_bitwise_not_to_noop, cxx_and_to_or, cxx_or_to_and, cxx_xor_to_or, cxx_lshift_to_rshift, cxx_rshift_to_lshift

    :cxx_bitwise_assignment:	cxx_and_assign_to_or_assign, cxx_or_assign_to_and_assign, cxx_xor_assign_to_or_assign, cxx_lshift_assign_to_rshift_assign, cxx_rshift_assign_to_lshift_assign

    :cxx_boundary:	cxx_le_to_lt, cxx_lt_to_le, cxx_ge_to_gt, cxx_gt_to_ge

    :cxx_calls:	cxx_remove_void_call, cxx_replace_scalar_call

    :cxx_comparison:	cxx_eq_to_ne, cxx_ne_to_eq, cxx_le_to_gt, cxx_lt_to_ge, cxx_ge_to_lt, cxx_gt_to_le

    :cxx_const_assignment:	cxx_assign_const, cxx_init_const

    :cxx_decrement:	cxx_pre_dec_to_pre_inc, cxx_post_dec_to_post_inc

    :cxx_default:	cxx_increment, cxx_arithmetic, cxx_comparison, cxx_boundary

    :cxx_increment:	cxx_pre_inc_to_pre_dec, cxx_post_inc_to_post_dec

    :cxx_logical:	cxx_logical_and_to_or, cxx_logical_or_to_and, cxx_remove_negation

    :experimental:	negate_mutator, scalar_value_mutator, cxx_logical

