out:
	../build/Compiler
	llvm-link -S llvm_ir.txt lib.ll -o link.ll
	llc link.ll
	clang link.s -o main

run: out
	./main

clean:
	rm link.ll link.s main