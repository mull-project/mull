
pub fn sum(a: i32, b: i32) -> i32 {
  return a + b;
}

pub fn just_print() {
  println!("I am just_print() function. I just say hello!");
}

#[test]
fn rusttest_foo_sum1() {
  assert!(sum(3, 4) == 7);
}

#[test]
fn rusttest_foo_sum2() {
  assert!(sum(4, 5) == 9);
}

#[test]
fn rusttest_foo_sum3() {
  assert!(sum(5, 6) == 11);
}

#[test]
fn rusttest_foo_sum4() {
  assert!(sum(5, 6) == 11);
}

