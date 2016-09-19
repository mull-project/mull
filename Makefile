
BUILD_NINJA ?= $(realpath ../../../BuildNinja)
MUTANG=$(BUILD_NINJA)/projects/mutang-project/unittests/MutangUnitTests 

test: test_unit test_integration

test_unit:
	cd $(BUILD_NINJA) && ninja MutangUnitTests

	# TODO: A common but dirty solution, people should learn about rpath 
	# http://stackoverflow.com/a/12399085/598057
	cd $(BUILD_NINJA) && LD_LIBRARY_PATH=$(BUILD_NINJA)/lib $(MUTANG)

test_integration:
	# TODO: also run unit tests using ninja
	cd $(BUILD_NINJA) && ninja check-mutang

