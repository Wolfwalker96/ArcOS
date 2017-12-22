From https://github.com/cfenollosa/os-tutorial

00-environment
===================


*Concepts you may want to Google beforehand: linux, mac, terminal, compiler, emulator, nasm, qemu*

**Goal: Install the software required to run this tutorial**

I'm working on a Mac, though Linux is better because it will have all the standard tools already
available for you.

On a mac, [install Homebrew](http://brew.sh) and then `brew install qemu nasm`

Don't use the Xcode developer tools `nasm` if you have them installed, they won't work for the most cases. Always use `/usr/local/bin/nasm`

On some systems qemu is split into multiple binaries. You may want
to call `qemu-system-x86_64 binfile`


01-bootsector-barebones
===================


*Concepts you may want to Google beforehand: assembler, BIOS*

**Goal: Create a file which the BIOS interprets as a bootable disk**

This is very exciting, we're going to create our own boot sector!

Theory
------

When the computer boots, the BIOS doesn't know how to load the OS, so it
delegates that task to the boot sector. Thus, the boot sector must be
placed in a known, standard location. That location is the first sector
of the disk (cylinder 0, head 0, sector 0) and it takes 512 bytes.

To make sure that the "disk is bootable", the BIOS checks that bytes
511 and 512 of the alleged boot sector are bytes `0xAA55`.

This is the simplest boot sector ever:

```
e9 fd ff 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
[ 29 more lines with sixteen zero-bytes each ]
00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 aa
```

It is basically all zeros, ending with the 16-bit value
`0xAA55` (beware of indianness, x86 is little-endian). 
The first three bytes perform an infinite jump

Simplest boot sector ever
-------------------------

You can either write the above 512 bytes
with a binary editor, or just write a very
simple assembler code:

```nasm
; Infinite loop (e9 fd ff)
loop:
    jmp loop 

; Fill with 510 zeros minus the size of the previous code
times 510-($-$$) db 0
; Magic number
dw 0xaa55 
```

To compile:
`nasm -f bin boot_sect_simple.asm -o boot_sect_simple.bin`

> OSX warning: if this drops an error, read chapter 00 again

I know you're anxious to try it out (I am!), so let's do it:

`qemu boot_sect_simple.bin`

You will see a window open which says "Booting from Hard Disk..." and
nothing else. When was the last time you were so excited to see an infinite
loop? ;-)


02-bootsector-print
===================


*Concepts you may want to Google beforehand: interrupts, CPU
registers*

**Goal: Make our previously silent boot sector print some text**

We will improve a bit on our infinite-loop boot sector and print
something on the screen. We will raise an interrupt for this.

On this example we are going to write each character of the "Hello"
word into the register `al` (lower part of `ax`), the bytes `0x0e`
into `ah` (the higher part of `ax`) and raise interrupt `0x10` which
is a general interrupt for video services.

`0x0e` on `ah` tells the video interrupt that the actual function
we want to run is to 'write the contents of `al` in tty mode'.

We will set tty mode only once though in the real world we 
cannot be sure that the contents of `ah` are constant. Some other
process may run on the CPU while we are sleeping, not clean
up properly and leave garbage data on `ah`.

For this example we don't need to take care of that since we are
the only thing running on the CPU.

Our new boot sector looks like this:
```nasm
mov ah, 0x0e ; tty mode
mov al, 'H'
int 0x10
mov al, 'e'
int 0x10
mov al, 'l'
int 0x10
int 0x10 ; 'l' is still on al, remember?
mov al, 'o'
int 0x10

jmp $ ; jump to current address = infinite loop

; padding and magic number
times 510 - ($-$$) db 0
dw 0xaa55 
```

You can examine the binary data with `xxd file.bin`

Anyway, you know the drill:

`nasm -fbin boot_sect_hello.asm -o boot_sect_hello.bin`

`qemu boot_sect_hello.bin`

Your boot sector will say 'Hello' and hang on an infinite loop


03-bootsector-memory
===================


*Concepts you may want to Google beforehand: memory offsets, pointers*

**Goal: Learn how the computer memory is organized**

Please open page 14 [of this document](
http://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf)<sup>1</sup>
and look at the figure with the memory layout.

The only goal of this lesson is to learn where the boot sector is stored

I could just bluntly tell you that the BIOS places it at `0x7C00` and
get it done with, but an example with wrong solutions will make things clearer.

We want to print an X on screen. We will try 4 different strategies
and see which ones work and why.

**Open the file `boot_sect_memory.asm`**

First, we will define the X as data, with a label:
```nasm
the_secret:
    db "X"
```

Then we will try to access `the_secret` in many different ways:

1. `mov al, the_secret`
2. `mov al, [the_secret]`
3. `mov al, the_secret + 0x7C00`
4. `mov al, 2d + 0x7C00`, where `2d` is the actual position of the 'X' byte in the binary

Take a look at the code and read the comments.

Compile and run the code. You should see a string similar to `1[2¢3X4X`, where
the bytes following 1 and 2 are just random garbage.

If you add or remove instructions, remember to compute the new offset of the X
by counting the bytes, and replace `0x2d` with the new one.

Please don't continue onto the next section unless you have 100% understood
the boot sector offset and memory addressing.


The global offset
-----------------

Now, since offsetting `0x7c00` everywhere is very inconvenient, assemblers let
us define a "global offset" for every memory location, with the `org` command:

```nasm
[org 0x7c00]
```

Go ahead and **open `boot_sect_memory_org.asm`** and you will see the canonical
way to print data with the boot sector, which is now attempt 2. Compile the code
and run it, and you will see how the `org` command affects each previous solution.

Read the comments for a full explanation of the changes with and without `org`

-----

[1] This whole tutorial is heavily inspired on that document. Please read the
root-level README for more information on that.


04-bootsector-stack
===================


*Concepts you may want to Google beforehand: stack*

**Goal: Learn how to use the stack**

The usage of the stack is important, so we'll write yet another boot sector
with an example.

Remember that the `bp` register stores the base address (i.e. bottom) of the stack,
and `sp` stores the top, and that the stack grows downwards from `bp` (i.e. `sp` gets
decremented)

This lesson is quite straightforward, so jump ahead to the code.

I suggest that you try accessing in-stack memory addresses by yourself, 
at different points in the code, and see what happens.


05-bootsector-functions-strings
===================


*Concepts you may want to Google beforehand: control structures,
function calling, strings*

**Goal: Learn how to code basic stuff (loops, functions) with the assembler**

We are close to our definitive boot sector.

In lesson 7 we will start reading from the disk, which is the last step before
loading a kernel. But first, we will write some code with control structures,
function calling, and full strings usage. We really need to be comfortable with
those concepts before jumping to the disk and the kernel.


Strings
-------

Define strings like bytes, but terminate them with a null-byte (yes, like C)
to be able to determine their end.

```nasm
mystring:
    db 'Hello, World', 0
```

Notice that text surrounded with quotes is converted to ASCII by the assembler,
while that lone zero will be passed as byte `0x00` (null byte)


Control structures
------------------

We have already used one: `jmp $` for the infinite loop.

Assembler jumps are defined by the *previous* instruction result. For example:

```nasm
cmp ax, 4      ; if ax = 4
je ax_is_four  ; do something (by jumping to that label)
jmp else       ; else, do another thing
jmp endif      ; finally, resume the normal flow

ax_is_four:
    .....
    jmp endif

else:
    .....
    jmp endif  ; not actually necessary but printed here for completeness

endif:
```

Think in your head in high level, then convert it to assembler in this fashion.

There are many `jmp` conditions: if equal, if less than, etc. They are pretty 
intuitive but you can always Google them


Calling functions
-----------------

As you may suppose, calling a function is just a jump to a label.

The tricky part are the parameters. There are two steps to working with parameters:

1. The programmer knows they share a specific register or memory address
2. Write a bit more code and make function calls generic and without side effects

Step 1 is easy. Let's just agree that we will use `al` (actually, `ax`) for the parameters.

```nasm
mov al, 'X'
jmp print
endprint:

...

print:
    mov ah, 0x0e  ; tty code
    int 0x10      ; I assume that 'al' already has the character
    jmp endprint  ; this label is also pre-agreed
```

You can see that this approach will quickly grow into spaghetti code. The current
`print` function will only return to `endprint`. What if some other function
wants to call it? We are killing code reusage.

The correct solution offers two improvements:

- We will store the return address so that it may vary
- We will save the current registers to allow subfunctions to modify them
  without any side effects

To store the return address, the CPU will help us. Instead of using a couple of
`jmp` to call subroutines, use `call` and `ret`.

To save the register data, there is also a special command which uses the stack: `pusha`
and its brother `popa`, which pushes all registers to the stack automatically and
recovers them afterwards.


Including external files
------------------------

I assume you are a programmer and don't need to convince you why this is
a good idea.

The syntax is
```nasm
%include "file.asm"
```


Printing hex values
-------------------

In the next lesson we will start reading from disk, so we need some way
to make sure that we are reading the correct data. File `boot_sect_print_hex.asm`
extends `boot_sect_print.asm` to print hex bytes, not just ASCII chars.


Code! 
-----

Let's jump to the code. File `boot_sect_print.asm` is the subroutine which will
get `%include`d in the main file. It uses a loop to print bytes on screen.
It also includes a function to print a newline. The familiar `'\n'` is
actually two bytes, the newline char `0x0A` and a carriage return `0x0D`. Please
experiment by removing the carriage return char and see its effect.

As stated above, `boot_sect_print_hex.asm` allows for printing of bytes.

The main file `boot_sect_main.asm` loads a couple strings and bytes,
calls `print` and `print_hex` and hangs. If you understood
the previous sections, it's quite straightforward.


06-bootsector-segmentation
===================


*Concepts you may want to Google beforehand: segmentation*

**Goal: learn how to address memory with 16-bit real mode segmentation**

If you are comfortable with segmentation, skip this lesson.

We did segmentation
with `[org]` on lesson 3. Segmentation means that you can specify
an offset to all the data you refer to.

This is done by using special registers: `cs`, `ds`, `ss` and `es`, for
Code, Data, Stack and Extra (i.e. user-defined)

Beware: they are *implicitly* used by the CPU, so once you set some
value for, say, `ds`, then all your memory access will be offset by `ds`.
[Read more here](http://wiki.osdev.org/Segmentation)

Furthermore, to compute the real address we don't just join the two
addresses, but we *overlap* them: `segment << 4 + address`. For example,
if `ds` is `0x4d`, then `[0x20]` actually refers to `0x4d0 + 0x20 = 0x4f0`

Enough theory. Have a look at the code and play with it a bit.

Hint: We cannot `mov` literals to those registers, we have to
use a general purpose register before.


07-bootsector-disk
===================


*Concepts you may want to Google beforehand: hard disk, cylinder, head, sector, 
carry bit*

**Goal: Let the bootsector load data from disk in order to boot the kernel**

Our OS won't fit inside the bootsector 512 bytes, so we need to read data from
a disk in order to run the kernel.

Thankfully, we don't have to deal with turning spinning platters on and off,
we can just call some BIOS routines, like we did to print characters on the screen.
To do so, we set `al` to `0x02` (and other registers with the required cylinder, head
and sector) and raise `int 0x13`

You can access [a detailed int 13h guide here](http://stanislavs.org/helppc/int_13-2.html)

On this lesson we will use for the first time the *carry bit*, which is an extra bit
present on each register which stores when an operation has overflowed its current
capacity:

```nasm
mov ax, 0xFFFF
add ax, 1 ; ax = 0x0000 and carry = 1
```

The carry isn't accessed directly but used as a control structure by other operators,
like `jc` (jump if the carry bit is set)

The BIOS also sets `al` to the number of sectors read, so always compare it
to the expected number.


Code
----

Open and examine `boot_sect_disk.asm` for the complete routine that
reads from disk.

`boot_sect_main.asm` prepares the parameters for disk read and calls `disk_load`.
Notice how we write some extra data which does not actually belong to the boot
sector, since it is outside the 512 bits mark.

The boot sector is actually sector 1 (the first one, sectors start at 1)
of cylinder 0 of head 0 of hdd 0.

Thus, any bytes after byte 512 correspond to sector 2 of cylinder 0 of head 0 of hdd 0

The main routine will fill it with sample data and then let the bootsector
read it.

**Note: if you keep getting errors and your code seems fine, make sure that qemu
is booting from the right drive and set the drive on `dl` accordingly**

The BIOS sets `dl` to the drive number before calling the bootloader. However,
I found some problems with qemu when booting from the hdd.

There are two quick options:

1. Try the flag `-fda` for example, `qemu -fda boot_sect_main.bin` which will set `dl`
as `0x00`, it seems to work fine then.
2. Explicitly use the flag `-boot`, e.g. `qemu boot_sect_main.bin -boot c` which 
automatically sets `dl` as `0x80` and lets the bootloader read data




08-32bit-print
===================


*Concepts you may want to Google beforehand: 32-bit protected mode, VGA, video 
memory*

**Goal: Print on the screen when on 32-bit protected mode**

32-bit mode allows us to use 32 bit registers and memory addressing, 
protected memory, virtual memory and other advantages, but we will lose
BIOS interrupts and we'll need to code the GDT (more on this later)

In this lesson we will write a new print string routine which works in
32-bit mode, where we don't have BIOS interrupts, by directly manipulating
the VGA video memory instead of calling `int 0x10`. The VGA memory starts
at address `0xb8000` and it has a text mode which is useful to avoid
manipulating direct pixels.


The formula for accessing a specific character on the 80x25 grid is:

`0xb8000 + 2 * (row * 80 + col)`

That is, every character uses 2 bytes (one for the ASCII, another for 
color and such), and we see that the structure of the memory concatenates
rows.

Open `32bit-print.asm` to see the code. It will always print the string
on the top left of the screen, but soon we'll write higher level routines
to replace it.

Unfortunately we cannot yet call this routine from the bootloader, because
we still don't know how to write the GDT and enter protected mode. Once
you have understood the code, jump to the next lesson.


09-32bit-gdt
===================


*Concepts you may want to Google beforehand: GDT*

**Goal: program the GDT**

Remember segmentation from lesson 6? The offset was left shifted
to address an extra level of indirection.

In 32-bit mode, segmentation works differently. Now, the offset becomes an
index to a segment descriptor (SD) in the GDT. This descriptor defines
the base address (32 bits), the size (20 bits) and some flags, like
readonly, permissions, etc. To add confusion, the data structures are split,
so open the os-dev.pdf file and check out the figure on page 34 or the 
Wikipedia page for the GDT.

The easiest way to program the GDT is to define two segments, one for code
and another for data. These can overlap which means there is no memory protection,
but it's good enough to boot, we'll fix this later with a higher language.

As a curiosity, the first GDT entry must be `0x00` to make sure that the
programmer didn't make any mistakes managing addresses.

Furthermore, the CPU can't directly load the GDT address, but it requires
a meta structure called the "GDT descriptor" with the size (16b) and address
(32b) of our actual GDT. It is loaded with the `lgdt` operation.

Let's directly jump to the GDT code in assembly. Again, to understand
all the segment flags, refer to the os-dev.pdf document. The theory for
this lesson is quite complex.

In the next lesson we will make the switch to 32-bit protected mode
and test our code from these lessons.


10-32bit-enter
===================


*Concepts you may want to Google beforehand: interrupts, pipelining*

**Goal: Enter 32-bit protected mode and test our code from previous lessons**

To jump into 32-bit mode:

1. Disable interrupts
2. Load our GDT
3. Set a bit on the CPU control register `cr0`
4. Flush the CPU pipeline by issuing a carefully crafted far jump
5. Update all the segment registers
6. Update the stack
7. Call to a well-known label which contains the first useful code in 32 bits

We will encapsulate this process on the file `32bit-switch.asm`. Open it
and take a look at the code.

After entering 32-bit mode, we will call `BEGIN_PM` which is the entry point
for our actual useful code (e.g. kernel code, etc). You can read the code
at `32bit-main.asm`. Compile and run this last file and you will see the two 
messages on the screen.

Congratulations! Our next step will be to write a simple kernel


11-kernel-crosscompiler
===================


*Concepts you may want to Google beforehand: cross-compiler*

**Goal: Create a development environment to build your kernel**

If you're using a Mac, you will need to do this process right away. Otherwise, it could have waited
for a few more lessons. Anyway, you will need a cross-compiler once we jump to developing in a higher
language, that is, C. [Read why](http://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F)

I'll be adapting the instructions [at the OSDev wiki](http://wiki.osdev.org/GCC_Cross-Compiler). 


Required packages
-----------------

First, install the required packages. On linux, use your package distribution. On a Mac, [install brew](http://brew.sh/) if
you didn't do it on lesson 00, and get those packages with `brew install`

- gmp
- mpfr
- libmpc
- gcc

Yes, we will need `gcc` to build our cross-compiled `gcc`, especially on a Mac where gcc has been deprecated for `clang`

Once installed, find where your packaged gcc is (remember, not clang) and export it. For example:

```
export CC=/usr/local/bin/gcc-4.9
export LD=/usr/local/bin/gcc-4.9
```

We will need to build binutils and a cross-compiled gcc, and we will put them into `/usr/local/i386elfgcc`, so
let's export some paths now. Feel free to change them to your liking.

```
export PREFIX="/usr/local/i386elfgcc"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"
```

binutils
--------

Remember: always be careful before pasting walls of text from the internet. I recommend copying line by line.

```sh
mkdir /tmp/src
cd /tmp/src
curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz # If the link 404's, look for a more recent version
tar xf binutils-2.24.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.24/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
make all install 2>&1 | tee make.log
```

gcc
---
```sh
cd /tmp/src
curl -O http://mirror.bbln.org/gcc/releases/gcc-4.9.1/gcc-4.9.1.tar.bz2
tar xf gcc-4.9.1.tar.bz2
mkdir gcc-build
cd gcc-build
../gcc-4.9.1/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-languages=c --without-headers
make all-gcc 
make all-target-libgcc 
make install-gcc 
make install-target-libgcc 
```

That's it! You should have all the GNU binutils and the compiler at `/usr/local/i386elfgcc/bin`, prefixed by `i386-elf-` to avoid
collisions with your system's compiler and binutils.

You may want to add the `$PATH` to your `.bashrc`. From now on, on this tutorial, we will explicitly use the prefixes when using
the cross-compiled gcc.


12-kernel-c
===================


*Concepts you may want to Google beforehand: C, object code, linker, disassemble*

**Goal: Learn to write the same low-level code as we did with assembler, but in C**


Compile
-------

Let's see how the C compiler compiles our code and compare it to the machine code
generated with the assembler.

We will start writing a simple program which contains a function, `function.c`.
Open the file and examine it.

To compile system-independent code, we need the flag `-ffreestanding`, so compile
`function.c` in this fashion:

`i386-elf-gcc -ffreestanding -c function.c -o function.o`

Let's examine the machine code generated by the compiler:

`i386-elf-objdump -d function.o`

Now that is something we recognize, isn't it?


Link
----

Finally, to produce a binary file, we will use the linker. An important part of this
step is to learn how high level languages call function labels. Which is the offset
where our function will be placed in memory? We don't actually know. For this
example, we'll place the offset at `0x0` and use the `binary` format which
generates machine code without any labels and/or metadata

`i386-elf-ld -o function.bin -Ttext 0x0 --oformat binary function.o`

*Note: a warning may appear when linking, disregard it*

Now examine both "binary" files, `function.o` and `function.bin` using `xdd`. You
will see that the `.bin` file is machine code, while the `.o` file has a lot
of debugging information, labels, etc.


Decompile
---------

As a curiosity, we will examine the machine code.

`ndisasm -b 32 function.bin`


More
----

I encourage you to write more small programs, which feature:

- Local variables `localvars.c`
- Function calls `functioncalls.c`
- Pointers `pointers.c`

Then compile and disassemble them, and examine the resulting machine code. Follow
the os-guide.pdf for explanations. Try to answer this question: why does the
disassemblement of `pointers.c` not resemble what you would expect? Where is
the ASCII `0x48656c6c6f` for "Hello"?


13-kernel-barebones
===================


*Concepts you may want to Google beforehand: kernel, ELF format, makefile*

**Goal: Create a simple kernel and a bootsector capable of booting it**

The kernel
----------

Our C kernel will just print an 'X' on the top left corner of the screen. Go ahead
and open `kernel.c`.

You will notice a dummy function that does nothing. That function will force us
to create a kernel entry routine which does not point to byte 0x0 in our kernel, but
to an actual label which we know that launches it. In our case, function `main()`.

`i386-elf-gcc -ffreestanding -c kernel.c -o kernel.o`

That routine is coded on `kernel_entry.asm`. Read it and you will learn how to
use `[extern]` declarations in assembly. To compile this file, instead of generating
a binary, we will generate an `elf` format file which will be linked with `kernel.o`

`nasm kernel_entry.asm -f elf -o kernel_entry.o`


The linker
----------

A linker is a very powerful tool and we only started to benefit from it.

To link both object files into a single binary kernel and resolve label references,
run:

`i386-elf-ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary`

Notice how our kernel will be placed not at `0x0` in memory, but at `0x1000`. The
bootsector will need to know this address too.


The bootsector
--------------

It is very similar to the one in lesson 10. Open `bootsect.asm` and examine the code.
Actually, if you remove all the lines used to print messages on the screen, it accounts
to a couple dozen lines.

Compile it with `nasm bootsect.asm -f bin -o bootsect.bin`


Putting it all together
-----------------------

Now what? We have two separate files for the bootsector and the kernel?

Can't we just "link" them together into a single file? Yes, we can, and it's easy,
just concatenate them:

`cat bootsect.bin kernel.bin > os-image.bin`


Run!
----

You can now run `os-image.bin` with qemu.

Remember that if you find disk load errors you may need to play with the disk numbers
or qemu parameters (floppy = `0x0`, hdd = `0x80`). I usually use `qemu-system-i386 -fda os-image.bin`

You will see four messages:

- "Started in 16-bit Real Mode"
- "Loading kernel into memory"
- (Top left) "Landed in 32-bit Protected Mode"
- (Top left, overwriting previous message) "X"

Congratulations!


Makefile
--------

As a last step, we will tidy up the compilation process with a Makefile. Open the `Makefile`
script and examine its contents. If you don't know what a Makefile is, now is a good time
to Google and learn it, as this will save us a lot of time in the future.


14-checkpoint
===================


*Concepts you may want to Google beforehand: monolithic kernel, microkernel, debugger, gdb*

**Goal: Pause and organize our code a little bit. Then learn how to debug the kernel with gdb**

Maybe you didn't realize it, but you already have your own kernel
running!

However, it does very little, just print an 'X'. Now is the time to stop for
a moment and organize the code into folders, create a scalable Makefile for future code,
and think on a strategy.

Take a look at the new folder structure. Most of the files have been symlinked
from previous lessons, so if we have to change them at some point, it will be
a better idea to remove the symlink and create a new file.

Furthermore, since from now on we will use mostly C to code, we'll take advantage of qemu's
ability to open a connection to gdb. First, let's install a cross-compiled `gdb` since
OSX uses `lldb` which is not compatible with the ELF file format (neither is the `gdb` available
on Homebrew's repos)

```sh
cd /tmp/src
curl -O http://ftp.rediris.es/mirror/GNU/gnu/gdb/gdb-7.8.tar.gz
tar xf gdb-7.8.tar.gz
mkdir gdb-build
cd gdb-build
export PREFIX="/usr/local/i386elfgcc"
export TARGET=i386-elf
../gdb-7.8/configure --target="$TARGET" --prefix="$PREFIX" --program-prefix=i386-elf-
make
make install
```

Check out the Makefile target `make debug`. This target uses builds `kernel.elf`, which
is an object file (not binary) with all the symbols we generated on the kernel, thanks to
the `-g` flag on gcc. Please examine it with `xxd` and you'll see some strings. Actually,
the correct way to examine the strings in an object file is by `strings kernel.elf`

We can take advantage of this cool qemu feature. Type `make debug` and, on the gdb shell:

- Set up a breakpoint in `kernel.c:main()`: `b main`
- Run the OS: `continue`
- Run two steps into the code: `next` then `next`. You will see that we are just about to set
  the 'X' on the screen, but it isn't there yet (check out the qemu screen)
- Let's see what's in the video memory: `print *video_memory`. There is the 'L' from "Landed in
  32-bit Protected Mode"
- Hmmm, let's make sure that `video_memory` points to the correct address: `print video_memory`
- `next` to put there our 'X'
- Let's make sure: `print *video_memory` and look at the qemu screen. It's definitely there.

Now is a good time to read some tutorial on `gdb` and learn super useful things like `info registers`
which will save us a lot of time in the future!


You may notice that, since this is a tutorial, we haven't yet discussed which kind
of kernel we will write. It will probably be a monolithic one since they are easier
to design and implement, and after all this is our first OS. Maybe in the future
we'll add a lesson "15-b" with a microkernel design. Who knows.


15-video-ports
===================


*Concepts you may want to Google beforehand: I/O ports*

**Goal: Learn how to use the VGA card data ports**

We will use C to communicate with devices via I/O registers and ports.

Open `drivers/ports.c` and examine the inline C assembler syntax. It has
some differences, like the order of the source and destination operands,
and the funny syntax to assign variables to operands.

When you understand the concepts, open `kernel/kernel.c` for an example
of use.

In this example we will examine the I/O ports which map the screen cursor
position. Specifically, we will query port `0x3d4` with value `14` to request
the cursor position high byte, and the same port with `15` for the low byte.

When this port is queried, it saves the result in port `0x3d5`

Don't miss the opportunity to use `gdb` to inspect the value of C variables,
since we still can't print them on the screen. To do so, set a breakpoint
for a specific line, `breakpoint kernel.c:21` and use the `print` command
to examine variables. Aren't you glad now that we invested some time in
compiling the cross-compiled gdb? ;)

Finally, we will use the queried cursor position to write a character
at that location.


16-video-driver
===================


*Concepts you may want to Google beforehand: VGA character cells, screen offset*

**Goal: Write strings on the screen**

Finally, we are going to be able to output text on the screen. This lesson contains
a bit more code than usual, so let's go step by step.

Open `drivers/screen.h` and you'll see that we have defined some constants for the VGA
card driver and three public functions, one to clear the screen and another couple
to write strings, the famously named `kprint` for "kernel print"

Now open `drivers/screen.c`. It starts with the declaration of private helper functions
that we will use to aid our `kprint` kernel API.

There are the two I/O port access routines that we learned in the previous lesson,
`get` and `set_cursor_offset()`.

Then there is the routine that directly manipulates the video memory, `print_char()`

Finally, there are three small helper functions to transform rows and columns into offsets
and vice versa.


kprint_at
---------

`kprint_at` may be called with a `-1` value for `col` and `row`, which indicates that
we will print the string at the current cursor position.

It first sets three variables for the col/row and the offset. Then it iterates through
the `char*` and calls `print_char()` with the current coordinates.

Note that `print_char` itself returns the offset of the next cursor position, and we reuse
it for the next loop.

`kprint` is basically a wrapper for `kprint_at`



print_char
----------

Like `kprint_at`, `print_char` allows cols/rows to be `-1`. In that case it retrieves
the cursor position from the hardware, using the `ports.c` routines.

`print_char` also handles newlines. In that case, we will position the cursor offset
to column 0 of the next row. 

Remember that the VGA cells take two bytes, one for the character itself and another one
for the attribute.


kernel.c
--------

Our new kernel is finally able to print strings.

It tests correct character positioning, spanning through multiple lines, line breaks,
and finally it tries to write outside of the screen bounds. What happens then?

In the next lesson we will learn how to scroll the screen.


17-video-scroll
===================


*Concepts you may want to Google beforehand: scroll*

**Goal: Scroll the screen when the text reaches the bottom**

For this short lesson, open `drivers/screen.c` and note that at the
bottom of `print_char` there is a new section (line 84) which checks
if the current offset is over the screen size and scrolls the text.

The actual scrolling is handled by a new function, `memory_copy`. It is
a simpler version of the standard `memcpy` but we named it differently
to avoid namespace collisions, at least for now. Open `kernel/util.c` to
see its implementation.

To help visualize scrolling, we will also implement a function to
convert integers to text, `int_to_ascii`. Again, it is a quick implementation
of the standard `itoa`. Notice that for integers which have double digits
or more, they are printed in reverse. This is intended. On future lessons
we will extend our helper functions, but that is not the point for now.

Finally, open `kernel/kernel.c`. Initially, each line displays its line
number. You can set a breakpoint on line 14 to confirm this. Then,
the following `kprint`s force the kernel to scroll down.

This lesson ends the coverage for the os-dev.pdf document. From now on, we'll
follow [the OSDev wiki](http://wiki.osdev.org/Meaty_Skeleton) and
other sources and examples. Thanks Prof. Blundell for that great document!


18-interrupts
===================


*Concepts you may want to Google beforehand: C types and structs, include guards, type attributes: packed, extern, volatile, exceptions*

**Goal: Set up the Interrupt Descriptor Table to handle CPU interrupts**

This lesson and the following ones have been heavily inspired
by [JamesM's tutorial](https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html)

Data types
----------

First, we will define some special data types in `cpu/types.h`,
which will help us uncouple data structures for raw bytes from chars and ints.
It has been carefully placed on the `cpu/` folder, where we will
put machine-dependent code from now on. Yes, the boot code
is specifically x86 and is still on `boot/`, but let's leave
that alone for now.

Some of the already existing files have been changed to use
the new `u8`, `u16` and `u32` data types.

From now on, our C header files will also have include guards.


Interrupts
----------

Interrupts are one of the main things that a kernel needs to 
handle. We will implement it now, as soon as possible, to be able
to receive keyboard input in future lessons.

Another examples of interrupts are: divisions by zero, out of bounds,
invalid opcodes, page faults, etc.

Interrupts are handled on a vector, with entries which are
similar to those of the GDT (lesson 9). However, instead of
programming the IDT in assembly, we'll do it in C.

`cpu/idt.h` defines how an idt entry is stored `idt_gate` (there need to be
256 of them, even if null, or the CPU may panic) and the actual
idt structure that the BIOS will load, `idt_register` which is 
just a memory address and a size, similar to the GDT register.

Finally, we define a couple variables to access those data structures
from assembler code.

`cpu/idt.c` just fills in every struct with a handler. 
As you can see, it is a matter
of setting the struct values and calling the `lidt` assembler command.


ISRs
----

The Interrupt Service Routines run every time the CPU detects an 
interrupt, which is usually fatal. 

We will write just enough code to handle them, print an error message,
and halt the CPU.

On `cpu/isr.h` we define 32 of them, manually. They are declared as
`extern` because they will be implemented in assembler, in `cpu/interrupt.asm`

Before jumping to the assembler code, check out `cpu/isr.c`. As you can see,
we define a function to install all isrs at once and load the IDT, a list of error
messages, and the high level handler, which kprints some information. You
can customize `isr_handler` to print/do whatever you want.

Now to the low level which glues every `idt_gate` with its low-level and
high-level handler. Open `cpu/interrupt.asm`. Here we define a common
low level ISR code, which basically saves/restores the state and calls
the C code, and then the actual ISR assembler functions which are referenced
on `cpu/isr.h`

Note how the `registers_t` struct is a representation of all the registers
we pushed in `interrupt.asm`

That's basically it. Now we need to reference `cpu/interrupt.asm` from our
Makefile, and make the kernel install the ISRs and launch one of them.
Notice how the CPU doesn't halt even though it would be good practice
to do it after some interrupts.


19-interrupts-irqs
===================


*Concepts you may want to Google beforehand: IRQs, PIC, polling*

**Goal: Finish the interrupts implementation and CPU timer**

When the CPU boots, the PIC maps IRQs 0-7 to INT 0x8-0xF
and IRQs 8-15 to INT 0x70-0x77. This conflicts with the ISRs
we programmed last lesson. Since we programmed ISRs 0-31, 
it is standard to remap the IRQs to ISRs 32-47.

The PICs are communicated with via I/O ports (see lesson 15).
The Master PIC has command 0x20 and data 0x21, while the slave has
command 0xA0 and data 0xA1.

The code for remapping the PICs is weird and includes
some masks, so check 
[this article](http://www.osdev.org/wiki/PIC) if you're curious.
Otherwise, just look at `cpu/isr.c`, new code after we set the IDT
gates for the ISRs. After that, we add the IDT gates for IRQs.

Now we jump to assembler, at `interrupt.asm`. The first task is to
add global definitions for the IRQ symbols we just used in the C code. 
Look at the end of the `global` statements.

Then, add the IRQ handlers. Same `interrupt.asm`, at the bottom. Notice
how they jump to a new common stub: `irq_common_stub` (next step)

We then create this `irq_common_stub` which is very similar to the ISR one.
It is located at the top of `interrupt.asm`, and it also defines
a new `[extern irq_handler]`

Now back to C code, to write the `irq_handler()` in `isr.c`. It sends some
EOIs to the PICs and calls the appropriate handler, which is stored in an array
named `interrupt_handlers` and defined at the top of the file. The new structs
are defined in `isr.h`. We will also use a simple function to register 
the interrupt handlers.

That was a lot of work, but now we can define our first IRQ handler!

There are no changes in `kernel.c`, so there is nothing new to run and see.
Please move on to the next lesson to check those shiny new IRQs.


20-interrupts-timer
===================


*Concepts you may want to Google beforehand: CPU timer, keyboard interrupts, scancode*

**Goal: Implement our first IRQ handlers: the CPU timer and the keyboard**

Everything is now ready to test our hardware interrupts.

Timer
-----

The timer is easy to configure. First we'll declare an `init_timer()` on `cpu/timer.h` and
implement it on `cpu/timer.c`. It is just a matter of computing the clock frequency and
sending the bytes to the appropriate ports.

We will now fix `kernel/utils.c int_to_ascii()` to print the numbers in the correct order.
For that, we need to implement `reverse()` and `strlen()`.

Finally, go back to the `kernel/kernel.c` and do two things. Enable interrupts again
(very important!) and then initialize the timer interrupt.

Go `make run` and you'll see the clock ticking!


Keyboard
--------

The keyboard is even easier, with a drawback. The PIC does not send us the ASCII code
for the pressed key, but the scancode for the key-down and the key-up events, so we
will need to translate those.

Check out `drivers/keyboard.c` where there are two functions: the callback and
the initialization which configures the interrupt callback. A new `keyboard.h` was
created with the definitions.

`keyboard.c` also has a long table to translate scancodes to ASCII keys. For the time
being, we will only implement a simple subset of the US keyboard. You can read
more [about scancodes here](http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html)

I don't know about you, but I'm thrilled! We are very close to building a simple shell.
In the next chapter, we will expand a little bit on keyboard input


21-shell
===================



**Goal: Clean the code a bit and parse user input**

In this lesson we will do tho things. First, we will clean up the code a bit, so it is ready 
for further lessons. During the previous ones I tried to put things in the most predictable places,
but it is also a good exercise to know when the code base is growing and adapt it to current
and further needs.


Code cleaning
-------------

First of all, we will quickly start to need more utility functions
for handling strings and so on. In a regular OS, this is called the C library,
or libc for short.

Right now we have a `utils.c` which we will split into `mem.c` and `string.c`, with their respective headers.

Second, we will create a new function `irq_install()` so that the kernel
only needs to perform one call to initialize all the IRQs. That function
is akin to `isr_install()` and placed on the same `irq.c`.
While we're here, we will disable the `kprint()` on `timer_callback()`
to avoid filling the screen with junk, now that we know that it works
properly.

There is not a clear distinction between `cpu/` and `drivers/`.
Keep in mind that I'm
creating this tutorial while following many others, and each of them
has a distinct folder structure. The only change we will do for now is to
move `drivers/ports.*` into `cpu/` since it is clearly cpu-dependent code.
`boot/` is also CPU-dependent code, but we will not mess with it until
we implement the boot sequence for a different machine.

There are more switches for the `CFLAGS` on the `Makefile`, since we will now
start creating higher-level functions for our C library and we don't want
the compiler to include any external code if we make a mistake with a declaration.
We also added some flags to turn warnings into errors, since an apparantly minor mistake
converting pointers can blow up later on. This also forced us to modify some misc pointer
declarations in our code.

Finally, we'll add a macro to avoid warning-errors on unused parameters on `libc/function.h`

Keyboard characters
-------------------

How to access the typed characters, then?

- When a key is pressed, the callback gets the ASCII code via a new
arrays which are defined at the beginning of `keyboard.c`
- The callback then appends that character to a buffer, `key_buffer`
- It is also printed on the screen
- When the OS wants to read user input, it calls `libc/io.c:readline()`

`keyboard.c` also parses backspace, by removing the last element
of the key buffer, and deleting it from the screen, by calling 
`screen.c:kprint_backspace()`. For this we needed to modify a bit
`print_char()` to not advance the offset when printing a backspace


Responding to user input
------------------------

The keyboard callback checks for a newline, and then calls the kernel,
telling it that the user has input something. Out final libc function
is `strcmp()`, which compares two strings and returns 0 if they
are equal. If the user inputs "END", we halt the CPU.

This is the most basic shell ever, but you should be proud, because
we implemented it from scratch. Do you realize how cool this is?

If you want to, expand `kernel.c` to parse more stuff. In the future,
when we have a filesystem, we will allow the user to run some basic commands.


22-malloc
===================


*Concepts you may want to Google beforehand: malloc*

**Goal: Implement a memory allocator**

We will add a kernel memory allocator to `libc/mem.c`. It is 
implemented as a simple pointer to free memory, which keeps
growing.

The `kmalloc()` function can be used to request an aligned page,
and it will also return the real, physical address, for later use.

We'll change the `kernel.c` leaving all the "shell" code there,
Let's just try out the new `kmalloc()`, and check out that
our first page starts at 0x10000 (as hardcoded on `mem.c`) and
subsequent `kmalloc()`'s produce a new address which is
aligned 4096 bytes or 0x1000 from the previous one.

Note that we added a new `strings.c:hex_to_ascii()` for
nicer printing of hex numbers.

Another cosmetic modification is to rename `types.c` to 
`type.c` for language consistency.

The rest of the files are unchanged from last lesson.


23-fixes
===================


*Concepts you may want to Google beforehand: freestanding, uint32_t, size_t*

**Goal: Fix miscellaneous issues with our code**

The OSDev wiki has a section [which describes some issues with
JamesM's tutorial](http://wiki.osdev.org/James_Molloy%27s_Tutorial_Known_Bugs).
Since we followed his tutorial for lessons 18-22 (interrupts through malloc), we'll
need to make sure we fix any of the issues before moving on.

1. Wrong CFLAGS
---------------

We add  `-ffreestanding` when compiling `.o` files, which includes `kernel_entry.o` and thus
`kernel.bin` and `os-image.bin`.

Before, we disabled libgcc (not libc) through the use of `-nostdlib` and we didn't re-enable
it for linking. Since this is tricky, we'll delete `-nostdlib`

`-nostdinc` was also pased to gcc, but we will need it for step 3, so let's delete it.


2. kernel.c `main()` function
-----------------------------

Modify `kernel/kernel.c` and change `main()` to `kernel_main()` since gcc recognizes "main" as 
a special keyword and we don't want to mess with that.

Change `boot/kernel_entry.asm` to point to the new name accordingly.

To fix the `i386-elf-ld: warning: cannot find entry symbol _start; defaulting to 0000000000001000`
warning message, add a `global _start;` and define the `_start:` label in `boot/kernel_entry.asm`.


3. Reinvented datatypes
-----------------------

It looks like it was a bad idea to define non-standard data types like `u32` and such, since
C99 introduces standard fixed-width data types like `uint32_t`

We need to include `<stdint.h>` which works even in `-ffreestanding` (but requires stdlibs)
and use those data types instead of our own, then delete them on `type.h`

We also delete the underscores around `__asm__` and `__volatile__` since they aren't needed.


4. Improperly aligned `kmalloc`
-------------------------------

First, since `kmalloc` uses a size parameter, we'll use the correct data type `size_t` instead
of `u32int_t`. `size_t` should be used for all parameters which "count" stuff and cannot be
negative. Include `<stddef.h>`. 

We will fix our `kmalloc` in the future, making it a proper memory manager and aligning data types.
For now, it will always return a new page-aligned memory block.


5. Missing functions
--------------------

We will implement the missing `mem*` functions in following lessons


6. Interrupt handlers
---------------------
`cli` is redundant, since we already established on the IDT entries if interrupts
are enabled within a handler using the `idt_gate_t` flags.

`sti` is also redundant, as `iret` loads the eflags value from the stack, which contains a 
bit telling whether interrupts are on or off.
In other words the interrupt handler automatically restores interrupts whether or not 
interrupts were enabled before this interrupt

On `cpu/isr.h`, `struct registers_t` has a couple issues. 
First, the alleged `esp` is renamed to `useless`.
The value is useless because it has to do with the current stack context, not what was interrupted.
Then, we rename `useresp` to `esp`

We add `cld` just before `call isr_handler` on `cpu/interrupt.asm` as suggested
by the osdev wiki.

There is a final, important issue with `cpu/interrupt.asm`. The common stubs create an instance
of `struct registers` on the stack and then call the C handler. But that breaks the ABI, since
the stack belongs to the called function and they may change them as they please. It is needed
to pass the struct as a pointer.

To achieve this, edit `cpu/isr.h` and `cpu/isr.c` and change `registers_t r` into `registers_t *t`,
then, instead of accessing the fields of the struct via `.`, access the fields of the pointer via `->`.
Finally, in `cpu/interrupt.asm`, and add a `push esp` before calling both `isr_handler` and
`irq_handler` -- remember to also `pop eax` to clear the pointer afterwards.

Both current callbacks, the timer and the keyboard, also need to be changed to use a pointer to
`registers_t`.


24-el-capitan
===================


**Goal: Update our build system to El Capitan**

If you were following this guide from the beginning, and upgraded to El Capitan only
to find that Makefiles don't compile anymore, follow these instructions to upgrade
your cross-compiler.

Otherwise, move on to the next lesson

Upgrading the cross-compiler
----------------------------

We will follow the same instructions as in lesson 11, more or less.

First, run `brew upgrade` and you will get your gcc upgraded to version 5.0 (at the time this guide was written)

Then run `xcode-select --install` to update OSX commandline tools

Once installed, find where your packaged gcc is (remember, not clang) and export it. For example:

```
export CC=/usr/local/bin/gcc-5
export LD=/usr/local/bin/gcc-5
```

We will need to recompile binutils and our cross-compiled gcc. Export the targets and prefix:

```
export PREFIX="/usr/local/i386elfgcc"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"
```

binutils
--------

Rember: always be careful before pasting walls of text from the internet. I recommend copying line by line.

```sh
mkdir /tmp/src
cd /tmp/src
curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz # If the link 404's, look for a more recent version
tar xf binutils-2.24.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.24/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
make all install 2>&1 | tee make.log
```


gcc
---
```sh
cd /tmp/src
curl -O http://mirror.bbln.org/gcc/releases/gcc-4.9.1/gcc-4.9.1.tar.bz2
tar xf gcc-4.9.1.tar.bz2
mkdir gcc-build
cd gcc-build
../gcc-4.9.1/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-languages=c --without-headers
make all-gcc 
make all-target-libgcc 
make install-gcc 
make install-target-libgcc 
```


Now try to type `make` on this lesson's folder and check that everything compiles smoothly


