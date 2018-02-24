FIXTURE_DIRS=\
  ./custom_test \
  ./google_test/google_test/ \
  ./simple_test/mutation_operators/and_or_replacement \
  ./simple_test/mutation_operators/and_or_replacement_cpp/ \
  ./simple_test/mutation_operators/math_div/ \
  ./simple_test/mutation_operators/math_mul/ \
  ./simple_test/mutation_operators/math_sub/ \
  ./simple_test/mutation_operators/negate_condition/ \
  ./simple_test/mutation_operators/remove_void_function/ \
  ./simple_test/mutation_operators/replace_assignment/ \
  ./simple_test/mutation_operators/replace_call/ \
  ./simple_test/mutation_operators/scalar_value/ \
  ./simple_test/count_letters \
  ./dylibs_and_objects \
  ./mutators/boundary \

synchronize_fixtures:
	@for path in ${FIXTURE_DIRS}; do \
		cd $$path && make synchronize_fixtures; cd -; \
	done

clean:
	@for path in ${FIXTURE_DIRS}; do \
		cd $$path && make clean; cd -; \
	done

