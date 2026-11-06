// clang-format off
int consumed = 0;
void consume(int value) {
  consumed = value;
  // CHECK: code.cpp:[[@LINE-1]]:12: warning: Killed: Replaced = with = 42; [cxx_assign_const]
}
int answer() {
  return 41;
}
bool positive(int a) {
  return a > 0;
  // CHECK: code.cpp:[[@LINE-1]]:12: warning: Killed: Replaced > with >= [cxx_gt_to_ge]
  // CHECK: code.cpp:[[@LINE-2]]:12: warning: Killed: Replaced > with <= [cxx_gt_to_le]
}

int arithmetic(int a, int b) {
  int x = a + b;
  // CHECK: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced a + b with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:13: warning: Killed: Replaced + with - [cxx_add_to_sub]
  x = x - a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced - with + [cxx_sub_to_add]
  x = x * b;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced * with / [cxx_mul_to_div]
  x = x / (b + 1);
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced / with * [cxx_div_to_mul]
  // CHECK: code.cpp:[[@LINE-3]]:14: warning: Killed: Replaced + with - [cxx_add_to_sub]
  x = x % (a + 1);
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced % with / [cxx_rem_to_div]
  // CHECK: code.cpp:[[@LINE-3]]:14: warning: Killed: Replaced + with - [cxx_add_to_sub]
  x += a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced += with -= [cxx_add_assign_to_sub_assign]
  x -= b;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced -= with += [cxx_sub_assign_to_add_assign]
  x *= a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced *= with /= [cxx_mul_assign_to_div_assign]
  x /= (b + 1);
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced /= with *= [cxx_div_assign_to_mul_assign]
  // CHECK: code.cpp:[[@LINE-2]]:11: warning: Killed: Replaced + with - [cxx_add_to_sub]
  x %= (a + 1);
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced %= with /= [cxx_rem_assign_to_div_assign]
  // CHECK: code.cpp:[[@LINE-2]]:11: warning: Killed: Replaced + with - [cxx_add_to_sub]
  return -x;
  // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced - with  [cxx_minus_to_noop]
}

int bitwise(int a, int b) {
  int x = (a & b) - (a ^ b);
  // CHECK: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced (a & b) - (a ^ b) with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:14: warning: Killed: Replaced & with | [cxx_and_to_or]
  // CHECK: code.cpp:[[@LINE-3]]:19: warning: Killed: Replaced - with + [cxx_sub_to_add]
  // CHECK: code.cpp:[[@LINE-4]]:24: warning: Killed: Replaced ^ with | [cxx_xor_to_or]
  x = x << 1;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced << with >> [cxx_lshift_to_rshift]
  x = x >> 1;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced >> with << [cxx_rshift_to_lshift]
  x &= a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced &= with |= [cxx_and_assign_to_or_assign]
  x |= b;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced |= with &= [cxx_or_assign_to_and_assign]
  x ^= a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced ^= with |= [cxx_xor_assign_to_or_assign]
  x <<= 1;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]
  x >>= 1;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced >>= with <<= [cxx_rshift_assign_to_lshift_assign]
  return ~x;
  // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ~ with  [cxx_bitwise_not_to_noop]
}

int comparison(int a, int b) {
  int count = 0;
  // CHECK: code.cpp:[[@LINE-1]]:15: warning: Killed: Replaced 0 with 1 [cxx_init_const]
  if (a == b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced == with != [cxx_eq_to_ne]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  if (a != b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced != with == [cxx_ne_to_eq]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  if (a < b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced < with >= [cxx_lt_to_ge]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced < with <= [cxx_lt_to_le]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  if (a <= b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced <= with > [cxx_le_to_gt]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced <= with < [cxx_le_to_lt]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  if (a > b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced > with >= [cxx_gt_to_ge]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced > with <= [cxx_gt_to_le]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  if (a >= b)
  // CHECK: code.cpp:[[@LINE-1]]:9: warning: Killed: Replaced >= with > [cxx_ge_to_gt]
  // CHECK: code.cpp:[[@LINE-2]]:9: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
    count++;
    // CHECK: code.cpp:[[@LINE-1]]:10: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  return count;
}

int increments(int a) {
  int x = a;
  // CHECK: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced a with 42 [cxx_init_const]
  x++;
  // CHECK: code.cpp:[[@LINE-1]]:4: warning: Killed: Replaced ++ with -- [cxx_post_inc_to_post_dec]
  x--;
  // CHECK: code.cpp:[[@LINE-1]]:4: warning: Killed: Replaced -- with ++ [cxx_post_dec_to_post_inc]
  ++x;
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced ++ with -- [cxx_pre_inc_to_pre_dec]
  --x;
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced -- with ++ [cxx_pre_dec_to_pre_inc]
  return x;
}

bool logical(int a, int b) {
  if (!a)
  // CHECK: code.cpp:[[@LINE-1]]:7: warning: Killed: Replaced ! with  [negate_mutator]
    return b;
  return a && b || !b;
  // CHECK: code.cpp:[[@LINE-1]]:20: warning: Killed: Replaced ! with  [cxx_remove_negation]
  // CHECK: code.cpp:[[@LINE-2]]:20: warning: Killed: Replaced ! with  [negate_mutator]
}

int negation(int a) {
  if (!a)
  // CHECK: code.cpp:[[@LINE-1]]:7: warning: Killed: Replaced ! with  [negate_mutator]
    return 1;
  return 0;
}

int calls(int a) {
  consume(a);
  // CHECK: code.cpp:[[@LINE-1]]:3: warning: Killed: Replaced consume(a) with  [cxx_remove_void_call]
  int x = answer();
  // CHECK: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced answer() with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:11: warning: Killed: Replaced answer() with 42 [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-3]]:11: warning: Killed: Replaced answer() with 0 [cxx_replace_scalar_call_zr]
  bool flag = positive(a);
  // CHECK: code.cpp:[[@LINE-1]]:15: warning: Killed: Replaced positive(a) with true [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:15: warning: Killed: Replaced positive(a) with true [cxx_replace_scalar_call_nz]
  // CHECK: code.cpp:[[@LINE-3]]:15: warning: Killed: Replaced positive(a) with false [cxx_replace_scalar_call_zr]
  return flag ? x : a;
}

int constants(int a) {
  int x = 5;
  // CHECK: code.cpp:[[@LINE-1]]:11: warning: Killed: Replaced 5 with 4 [cxx_init_const]
  double d = 2.5;
  // CHECK: code.cpp:[[@LINE-1]]:14: warning: Killed: Replaced 2.5 with -2.5 [cxx_init_const]
  bool flag = true;
  // CHECK: code.cpp:[[@LINE-1]]:15: warning: Killed: Replaced true with 0 [cxx_init_const]
  int initial = x + (int)d + flag;
  // CHECK: code.cpp:[[@LINE-1]]:17: warning: Killed: Replaced x + (int)d + flag with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:19: warning: Killed: Replaced + with - [cxx_add_to_sub]
  // CHECK: code.cpp:[[@LINE-3]]:28: warning: Killed: Replaced + with - [cxx_add_to_sub]
  x = 7;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 6; [cxx_assign_const]
  d = 3.5;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = -3.5; [cxx_assign_const]
  flag = false;
  // CHECK: code.cpp:[[@LINE-1]]:8: warning: Killed: Replaced = with = 1; [cxx_assign_const]
  int assigned = x * (int)d * (flag ? 2 : 3);
  // CHECK: code.cpp:[[@LINE-1]]:18: warning: Killed: Replaced x * (int)d * (flag ? 2 : 3) with 42 [cxx_init_const]
  // CHECK: code.cpp:[[@LINE-2]]:20: warning: Killed: Replaced * with / [cxx_mul_to_div]
  // CHECK: code.cpp:[[@LINE-3]]:29: warning: Killed: Replaced * with / [cxx_mul_to_div]
  x = a;
  // CHECK: code.cpp:[[@LINE-1]]:5: warning: Killed: Replaced = with = 42; [cxx_assign_const]
  return initial * 100 + assigned + x;
  // CHECK: code.cpp:[[@LINE-1]]:18: warning: Killed: Replaced * with / [cxx_mul_to_div]
  // CHECK: code.cpp:[[@LINE-2]]:24: warning: Killed: Replaced + with - [cxx_add_to_sub]
  // CHECK: code.cpp:[[@LINE-3]]:35: warning: Killed: Replaced + with - [cxx_add_to_sub]
}
