
BUILD_DIR_XCODE?=$(PWD)/BuildXcode
BUILD_DIR_NINJA?=$(PWD)/BuildNinja

MULL_CMAKE_TOOLCHAIN?=$(PWD)/Mull.toolchain.OSX.cmake

MULL_UNITTESTS_DIR=$(BUILD_DIR_NINJA)/unittests
MULL=$(MULL_UNITTESTS_DIR)/MullUnitTests

# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

test: test_unit test_integration

test_unit: build_ninja ## Run Unit Tests (Builds Ninja cache first if it does not exist)
	cd $(BUILD_DIR_NINJA) && ninja MullUnitTests

	# TODO: A common but dirty solution, people should learn about rpath
	# http://stackoverflow.com/a/12399085/598057
	cd $(MULL_UNITTESTS_DIR) && LD_LIBRARY_PATH=$(BUILD_DIR_NINJA)/lib $(MULL)

test_integration:
	# TODO: also run unit tests using ninja
	cd $(BUILD_DIR_NINJA) && ninja check-mull


CMAKE_COMMAND_LINE_DEBUG_FLAGS=# --trace # --debug-output # --debug-output --trace --trace-expand # --trace # --debug-output #

.PHONY: build build_xcode build_ninja

build_xcode: ## Build Xcode project with CMake.
	mkdir -p $(BUILD_DIR_XCODE)

	rm -rfv $(BUILD_DIR_XCODE)/CMakeCache.txt
	cd $(BUILD_DIR_XCODE) && cmake ../ -G Xcode \
	  $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
	  -DCMAKE_TOOLCHAIN_FILE=$(MULL_CMAKE_TOOLCHAIN) \
	  -DMULL_SUPPORT_RUST=1

rebuild_xcode: build_xcode reopen ## Build Xcode project with CMake, kill Xcode, reopen the project in Xcode

build_ninja: ## Build Ninja project with CMake.
	mkdir -p $(BUILD_DIR_NINJA)

	rm -rfv $(BUILD_DIR_NINJA)/CMakeCache.txt
	cd $(BUILD_DIR_NINJA) && cmake ../ -G Ninja

## Xcode-specific tools.
## TODO: maybe extract to Makefile.Xcode?
open: ## Open Mull.xcodeproj in Xcode
	open BuildXcode/Mull.xcodeproj

# This reopen task is mostly needed to do a work that involves serious
# modifications of CMake's files: **/CMakeLists.txt and toolchain files.
# Xcode does not pickup all of the changes in CMake without being reopened.
reopen: ## Kill Xcode and open Mull.xcodeproj in Xcode.
	killall Xcode || true
	open BuildXcode/Mull.xcodeproj

clean: clean_ninja clean_xcode ## Delete CMake build caches: Xcode and Ninja.

clean_xcode: ## Delete Xcode CMake build cache.
	rm -rfv $(BUILD_DIR_XCODE)

clean_ninja: ## Delete Ninja CMake build cache.
	rm -rfv $(BUILD_DIR_NINJA)

