# %.riscv: %.c lib.c Makefile
# 	./gcc -march=rv32im -mabi=ilp32 -fno-tree-loop-distribute-patterns -mno-relax -O1 $< lib.c -static -nostartfiles -nostdlib -o $@

# %.riscv: %.s Makefile
# 	./gcc -march=rv32im -mabi=ilp32 -fno-tree-loop-distribute-patterns -mno-relax -O1 $< -static -nostartfiles -nostdlib -o $@

# %.dis: %.riscv Makefile
# 	./objdump -s -w $< > $@
# 	./objdump -S $< >> $@

# FOR M1
%.riscv: %.c lib.c Makefile
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-gcc  -march=rv32im -mabi=ilp32 -fno-tree-loop-distribute-patterns -mno-relax -O1 $< lib.c -static -nostartfiles -nostdlib -o $@

%.riscv: %.s Makefile
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-gcc  -march=rv32im -mabi=ilp32 -fno-tree-loop-distribute-patterns -mno-relax -O1 $< -static -nostartfiles -nostdlib -o $@

%.dis: %.riscv Makefile
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-objdump -s -w $< > $@
	/usr/local/opt/riscv-gnu-toolchain/bin/riscv64-unknown-elf-objdump -S $< >> $@