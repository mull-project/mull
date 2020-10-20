#include "Testee.h"

#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// We simulate a Boost.Test test suite and test cases without actually including any Boost headers
// in order to avoid adding Boost as a dependency when nothing else in the project uses Boost.

// NB: There will be a whole lot of weird constructs in this code because it was adapted from a
// preprocessed real Boost.Test test case

/* Boost fake begins */
namespace boost {
namespace unit_test {
typedef unsigned long counter_t;
using const_string = std::string_view; // <-- not what Boost uses

enum test_unit_type { TUT_CASE = 0x01, TUT_SUITE = 0x10, TUT_ANY = 0x11 };

enum assertion_result { AR_FAILED, AR_PASSED, AR_TRIGGERED };

class test_unit {
public:
  test_unit_type const p_type;
  const_string const p_type_name;
  const_string const p_file_name;
  std::size_t const p_line_num;
  // omitting some properties for brevity
  std::string p_name;

protected:
  ~test_unit() = default;

  test_unit(const_string name, const_string file_name, std::size_t line_num, test_unit_type t)
      : p_type(t), p_type_name(t == TUT_CASE ? "case" : "suite"), p_file_name(file_name),
        p_line_num(line_num), p_name(name) {}
};

class test_case : public test_unit {
public:
  enum { type = TUT_CASE };

  //  test_case(const_string tc_name, std::function<void()> const &test_func);
  test_case(const_string tc_name, const_string tc_file, std::size_t tc_line,
            std::function<void()> const &test_func)
      : test_unit{ tc_name, tc_file, tc_line, test_unit_type(type) }, p_test_func{ test_func } {}

  std::function<void()> p_test_func;

  ~test_case() = default;
};

class test_suite : public test_unit {
public:
  enum { type = TUT_SUITE };

  explicit test_suite(const_string ts_name, const_string ts_file, std::size_t ts_line)
      : test_unit{ ts_name, ts_file, ts_line, test_unit_type(type) } {}

  ~test_suite() = default;
};

namespace decorator {

class __attribute__((__visibility__("default"))) collector_t {
  // Not the same as Boost
  // Not thread-safe
public:
  static collector_t &instance();
  std::vector<std::shared_ptr<test_unit>> test_units;

private:
  collector_t(collector_t const &) = delete;
  collector_t &operator=(collector_t const &) = delete;
  ~collector_t() = default;

private:
  collector_t() = default;

};

collector_t &collector_t::instance() {
  static collector_t s_instance;
  return s_instance;
}
} // namespace decorator

namespace ut_detail {

struct nil_t {};

struct __attribute__((__visibility__("default"))) auto_test_unit_registrar {

  // Test case registration
  auto_test_unit_registrar(test_case *tc, decorator::collector_t &decorators,
                           counter_t exp_fail = 0);
  // Test suite opening registration
  explicit auto_test_unit_registrar(const_string ts_name, const_string ts_file, std::size_t ts_line,
                                    decorator::collector_t &decorators);
  // Test suite closing registration
  explicit auto_test_unit_registrar(int);

  // Other registrar variants which are used for templated test cases or possibly other special
  // situations which we don't need in this simulation
  // explicit auto_test_unit_registrar( test_unit_generator const& tc_gen, decorator::collector_t&
  // decorators ); explicit auto_test_unit_registrar( boost::shared_ptr<test_unit_generator> tc_gen,
  // decorator::collector_t& decorators );
};

// not the same as Boost
auto_test_unit_registrar::auto_test_unit_registrar(test_case *tc, decorator::collector_t &collector,
                                                   counter_t) {
  collector.test_units.push_back(std::shared_ptr<test_unit>(tc));
}
auto_test_unit_registrar::auto_test_unit_registrar(const_string ts_name, const_string ts_file,
                                                   std::size_t ts_line,
                                                   decorator::collector_t &collector) {
  collector.test_units.push_back(std::make_shared<test_suite>(ts_name, ts_file, ts_line));
}
auto_test_unit_registrar::auto_test_unit_registrar(int) {}


} // namespace ut_detail

inline test_case *make_test_case(std::function<void()> const &test_func, const_string tc_name,
                                 const_string tc_file, std::size_t tc_line) {
  return new test_case(tc_name, tc_file, tc_line, test_func);
}
} // namespace unit_test
} // namespace boost

typedef boost::unit_test::ut_detail::nil_t BOOST_AUTO_TEST_CASE_FIXTURE;

int main() {

  auto &collector = boost::unit_test::decorator::collector_t::instance();
  for (auto &&unit : collector.test_units) {
    std::cout << unit->p_type_name << ": " << unit->p_name << std::endl;
    if (unit->p_type == boost::unit_test::TUT_CASE) {
      auto &tc = *static_cast<boost::unit_test::test_case *>(unit.get());
      try {
        tc.p_test_func();
      } catch (const std::exception &e) {
        std::cerr << "Failed test " << unit->p_name << std::endl;
        std::cerr << "  " << unit->p_file_name << ":" << unit->p_line_num << ": " << e.what() << std::endl;
        std::terminate();
      }
    }
  }
  std::cout << "All tests pass" << std::endl;

  return 0;
}

/* Boost fake ends */

namespace mull::boosttest_tests {

namespace SampleTestSuite {
static boost::unit_test::ut_detail::auto_test_unit_registrar SampleTestSuite_registrar70
    __attribute__((__unused__)) (boost::unit_test::const_string{ "SampleTestSuite" },
                                 boost::unit_test::const_string{
                                     "../tests/fixtures/boosttest_test/boosttest_test/Test.cpp" },
                                 42, boost::unit_test::decorator::collector_t::instance());

struct sampleTestCase : public BOOST_AUTO_TEST_CASE_FIXTURE {
  void test_method();
};

static void sampleTestCase_invoker() {
  sampleTestCase t;
  //  boost::unit_test::setup_conditional(t);
  t.test_method();
  //  boost::unit_test::teardown_conditional(t);
}
struct sampleTestCase_id {};
static boost::unit_test::ut_detail::auto_test_unit_registrar sampleTestCase_registrar91
    __attribute__((__unused__)) (boost::unit_test::make_test_case(
                                     &sampleTestCase_invoker, "sampleTestCase",
                                     "../spotify/player_builder/cpp/tests/player_builder_test.cpp",
                                     9),
                                 boost::unit_test::decorator::collector_t::instance());
void sampleTestCase::test_method() {
  Testee testeeClass;

  int result = testeeClass.sum(2, 3);

  // Not really what Boost does
  if (result != 5) {
    throw std::logic_error{ "Failed check result != 5" };
  }
}

static boost::unit_test::ut_detail::auto_test_unit_registrar end_suite_registrar172
    __attribute__((__unused__)) (1);
} // namespace SampleTestSuite

} // namespace mull::boosttest_tests
