OUTDIR = "../out"

build : bootloader.asm boot_sect_print.asm 32bit-switch.asm \
				32bit-print.asm 32bit-gdt.asm
	echo "Bootloader is being built"
	@nasm -f bin bootloader.asm -o $(OUTDIR)/tmp/boot.bin
	echo "Bootloader has been built"
