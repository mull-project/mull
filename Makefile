MAKEFLAGS += --warn-undefined-variables

BUILD_DIR_XCODE?=$(PWD)/BuildXcode
BUILD_DIR_NINJA?=$(PWD)/BuildNinja

MULL_UNIT_TESTS_DIR=$(BUILD_DIR_NINJA)/unittests
MULL_UNIT_TESTS=$(MULL_UNIT_TESTS_DIR)/MullUnitTests

INSTALL_DIR?=/usr/local

CMAKE_COMMAND_LINE_DEBUG_FLAGS=# --trace # --debug-output # --debug-output --trace --trace-expand # --trace # --debug-output #

OS?=$(shell uname -s)

ifeq ($(LLVM_ROOT),)
	ifeq ($(OS), Darwin)
		LLVM_ROOT=/opt/llvm-3.9
	else
		ifneq ($(wildcard /etc/debian_version),)
			LLVM_ROOT=/usr/lib/llvm-3.9
		endif
	endif
endif

ifeq ($(LLVM_ROOT),)
$(error Could not find/identify LLVM_ROOT)
endif

# Self-Documented Makefile
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help
help: ## Show this help message.
	@grep -E '^[a-zA-Z_\.-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-27s\033[0m %s\n", $$1, $$2}'

fixtures.generate: ## Generates fixtures for tests
	cd lab && make synchronize_fixtures

fixtures.clean: ## Clean fixtures
	find $(PWD)/lab | grep \.bc | xargs rm -v

clean: xcode.clean ninja.clean ## Remove all the build artefacts
	@true

test: ninja.run.unit-tests ## Run unit tests
	@true

install: ninja.install.mull-driver ## Install mull-driver into INSTALL_DIR (default: /usr/local/)
	@true

###
###  Ninja
###

ninja.init: $(BUILD_DIR_NINJA) ## Prepare Ninja project on macOS
	cd $(BUILD_DIR_NINJA) && cmake -G Ninja \
    -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
    $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
    -DLLVM_ROOT=$(LLVM_ROOT) \
    ../

ninja.build.mull-driver: ## Build mull-driver on macOS
	cd $(BUILD_DIR_NINJA) && ninja mull-driver
	@echo "Resulting binary:\n"$(BUILD_DIR_NINJA)/tools/driver/mull-driver

ninja.build.unit-tests: fixtures.generate ## Build unit-tests on macOS
	cd $(BUILD_DIR_NINJA) && ninja MullUnitTests

ninja.install.mull-driver: ninja.build.mull-driver ## Install mull driver
	cd $(BUILD_DIR_NINJA) && ninja install

ninja.build.example: ninja.install.mull-driver ## Build example on macOS
	cd Examples/HelloWorld && \
    export PATH=$(INSTALL_DIR)/bin:$(PATH) && \
    make example \
      MULL=mull-driver \
      MULL_CC=$(LLVM_ROOT)/bin/clang

ninja.run.unit-tests: ninja.build.unit-tests ## Run unit-tests on macOS
	cd $(MULL_UNIT_TESTS_DIR) && $(MULL_UNIT_TESTS)

ninja.run.example: ninja.build.example ## Run example on macOS
	cd Examples/HelloWorld && \
    export PATH=$(INSTALL_DIR)/bin:$(PATH) && \
    make run \
      MULL=mull-driver \
      MULL_CC=$(LLVM_ROOT)/bin/clang


ninja.clean:
	rm -rfv $(BUILD_DIR_NINJA)

$(BUILD_DIR_NINJA):
	mkdir $(BUILD_DIR_NINJA)

###
### Travis
###

travis.test: ninja.init ninja.run.unit-tests ninja.run.example

UBUNTU_URL_3.9=http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
UBUNTU_URL_4.0=http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
UBUNTU_URL_5.0=http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-linux-x86_64-ubuntu14.04.tar.xz
UBUNTU_URL_6.0=http://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
UBUNTU_URL=$(shell echo $(UBUNTU_URL_$(LLVM_VERSION)))
UBUNTU_ARCHIVE=$(shell echo $(UBUNTU_URL) | awk -F/ ' { print $$NF } ' | sed 's/.tar.xz//')

MACOS_URL_3.9=http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
MACOS_URL_4.0=http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-apple-darwin.tar.xz
MACOS_URL_5.0=http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-x86_64-apple-darwin.tar.xz
MACOS_URL_6.0=http://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-apple-darwin.tar.xz
MACOS_URL=$(shell echo $(MACOS_URL_$(LLVM_VERSION)))
MACOS_ARCHIVE=$(shell echo $(MACOS_URL) | awk -F/ ' { print $$NF } ' | sed 's/.tar.xz//')

travis.install.macos:
	brew update
	brew upgrade cmake
	brew install zlib
	brew install ncurses
	brew install sqlite
	brew install ninja
	cd /tmp && \
     wget $(MACOS_URL) && \
     tar xf $(MACOS_ARCHIVE).tar.xz && \
     sudo mv $(MACOS_ARCHIVE) /opt/llvm-$(LLVM_VERSION)

travis.install.ubuntu:
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo apt-get update
	sudo apt-get -y install wget git cmake make ninja-build \
     libz-dev sqlite3 libsqlite3-dev ncurses-dev \
     binutils-2.26 tree libstdc++6
	sudo update-alternatives --install /usr/bin/ld ld /usr/bin/ld-2.26 1
	cd /tmp && \
     wget $(UBUNTU_URL) && \
     tar xf $(UBUNTU_ARCHIVE).tar.xz && \
     sudo mv $(UBUNTU_ARCHIVE) /opt/llvm-$(LLVM_VERSION)

###
###  Xcode
###

xcode.init: $(BUILD_DIR_XCODE) ## Build Xcode project with CMake.
	cd $(BUILD_DIR_XCODE) && cmake ../ -G Xcode \
    $(CMAKE_COMMAND_LINE_DEBUG_FLAGS) \
    -DLLVM_ROOT=$(LLVM_ROOT) \

xcode.kill-and-rebuild: xcode.kill-and-reopen ## Build Xcode project with CMake, kill Xcode, reopen the project in Xcode

xcode.open: ## Open Mull.xcodeproj in Xcode
	open $(BUILD_DIR_XCODE)/Mull.xcodeproj

# This reopen task is mostly needed to do a work that involves serious
# modifications of CMake's files: **/CMakeLists.txt and toolchain files.
# Xcode does not pickup all of the changes in CMake without being reopened.
xcode.kill-and-reopen: ## Kill Xcode and open Mull.xcodeproj in Xcode.
	killall Xcode || true
	open $(BUILD_DIR_XCODE)/Mull.xcodeproj

xcode.clean: ## Delete Xcode CMake build cache.
	rm -rfv $(BUILD_DIR_XCODE)

$(BUILD_DIR_XCODE):
	mkdir $(BUILD_DIR_XCODE)

