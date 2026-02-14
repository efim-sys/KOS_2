asm:
	/home/efim/.platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32-elf-gcc -mtext-section-literals -O0 -c -S test.c

obj:
	/home/efim/.platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32-elf-gcc -mtext-section-literals -O0 -c test.c

link:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-ld test.o  /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/xtensa-esp32s3-elf/lib/libc.a /home/efim/.espressif/tools/xtensa-esp32s3-elf/esp-12.2.0_20230208/xtensa-esp32s3-elf/lib/gcc/xtensa-esp32s3-elf/12.2.0/libgcc.a -T custom.ld -o linked.o

rlink:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-ld test.o  /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/xtensa-esp32s3-elf/lib/libc.a /home/efim/.espressif/tools/xtensa-esp32s3-elf/esp-12.2.0_20230208/xtensa-esp32s3-elf/lib/gcc/xtensa-esp32s3-elf/12.2.0/libgcc.a -T custom.ld -r -o program.elf

bin: 
	/home/efim/.platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32-elf-objcopy -O binary linked.o linked.bin
	xxd -i linked.bin test.h

objdump:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-objdump -D -s program.elf

# l_objdump:
# 	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-objdump -d -s linked.o

reloc:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-objdump -r -s program.elf

symtab:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-objdump -t test.o

readelf:
	/home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-readelf -a linked.o 

copy:
	cp program.elf /media/efim/3956-02B7/program.elf