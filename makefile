OUTDIR=out
QEMU=qemu

ifndef VERBOSE
.SILENT:
endif

build : ${OUTDIR}/tmp
	@echo "ArcOS is being built"
	@echo "Build Bootloader"
	@make -C bootloader
	echo "Joining parts"
	cat $(OUTDIR)/tmp/boot.bin > $(OUTDIR)/os.bin
	echo "ArcOS has been built"

${OUTDIR}/tmp :
	@-mkdir -p $(OUTDIR)/tmp

${OUTDIR}/os.bin : build

boot : $(OUTDIR)/os.bin
	@echo "Booting ArcOS on QEMU..."
	@qemu $(OUTDIR)/os.bin

clean:
	-rm -r $(OUTDIR)
