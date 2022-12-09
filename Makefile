.PHONY: build
build:
	cd build && ninja

.PHONY: run
run:
	@ ./build/bin/kscope

.PHONY: clean
clean:
	cd build && ninja clean

.PHONY: remove
remove:
	rm -rf build
