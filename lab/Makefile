FIXTURE_DIRS=\
  ./custom_test \
  ./google_test/google_test/ \
  ./simple_test/mutators/and_or_replacement \
  ./simple_test/mutators/and_or_replacement_cpp/ \
  ./simple_test/mutators/math_div/ \
  ./simple_test/mutators/math_mul/ \
  ./simple_test/mutators/math_sub/ \
  ./simple_test/mutators/negate_condition/ \
  ./simple_test/mutators/remove_void_function/ \
  ./simple_test/mutators/replace_assignment/ \
  ./simple_test/mutators/replace_call/ \
  ./simple_test/mutators/scalar_value/ \
  ./simple_test/count_letters \
  ./dylibs_and_objects \
  ./mutators/boundary \
  ./junk_detection/compdb \

synchronize_fixtures:
	@for path in ${FIXTURE_DIRS}; do \
		cd $$path && make synchronize_fixtures; cd -; \
	done

clean:
	@for path in ${FIXTURE_DIRS}; do \
		cd $$path && make clean; cd -; \
	done

