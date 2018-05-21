FIXTURE_DIRS=\
  ./google_test/google_test/ \
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

