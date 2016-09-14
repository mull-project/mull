SOURCES=main.c

CC=clang

all: main.ll main.bc config.yaml

config.yaml:
	echo "fork: true" > config.yaml
	echo "bitcode_files:" >> config.yaml
	echo "  - $(PWD)/main.bc" >> config.yaml

main.ll:
	$(CC) -S -emit-llvm -o main.ll $(SOURCES)

main.bc:
	$(CC) -c -emit-llvm -o main.bc $(SOURCES)

clean:
	rm -f  main.ll main.bc config.yaml

