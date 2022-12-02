.PHONY: build
build:
	cd build && ninja

.PHONY: clean
clean:
	cd build && ninja clean

.PHONY: remove
remove:
	rm -rf build
