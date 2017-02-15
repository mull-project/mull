
BUILD_NINJA ?= $(realpath ../../../BuildNinja)
MULL=$(BUILD_NINJA)/projects/mull-project/unittests/MullUnitTests 

test: test_unit test_integration

test_unit: ## Run Unit Tests
	cd $(BUILD_NINJA) && ninja MullUnitTests

	# TODO: A common but dirty solution, people should learn about rpath 
	# http://stackoverflow.com/a/12399085/598057
	cd $(BUILD_NINJA) && LD_LIBRARY_PATH=$(BUILD_NINJA)/lib $(MULL)

test_integration:
	# TODO: also run unit tests using ninja
	cd $(BUILD_NINJA) && ninja check-mull


DEBUG_FLAGS= # --trace # --debug-output # --debug-output --trace --trace-expand # --trace # --debug-output #
CMAKE_FLAGS=-DBUILD_SHARED_LIBS=true

# CMAKE_FLAGS=-G Xcode CMAKE_BUILD_TYPE="Debug"

.PHONY: build build_xcode build_ninja

TOOLCHAIN_PATH=$(PWD)/Mutang.toolchain.cmake
build: build_xcode build_ninja

rebuild_xcode: build_xcode reopen ## Build Xcode project with CMake, kill Xcode, reopen the project in Xcode

build_xcode: ## Build Xcode project with CMake.
	mkdir -p BuildXcode

	rm -rfv BuildXcode/CMakeCache.txt
	cd BuildXcode && cmake ../ -G Xcode $(CMAKE_FLAGS) $(DEBUG_FLAGS) -DMULL_SUPPORT_RUST=1

open: ## Open Mull.xcodeproj in Xcode
	open BuildXcode/Mull.xcodeproj

reopen: ## Kill Xcode and open Mull.xcodeproj in Xcode.
	killall Xcode || true
	open BuildXcode/Mull.xcodeproj

clean: ## Full clean of CMake build directories.
	rm -rfv BuildXcode

# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

