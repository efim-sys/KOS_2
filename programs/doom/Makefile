# Параметры инструментов
TOOLCHAIN_BIN = /home/efim/.platformio/packages/toolchain-xtensa-esp-elf/bin
GCC     = $(TOOLCHAIN_BIN)/xtensa-esp32-elf-gcc
LD      = /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-ld
OBJDUMP = /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-objdump

# Библиотеки и пути
LIBC    = /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/xtensa-esp32s3-elf/lib/libc.a
LIBM    = /home/efim/.platformio/packages/toolchain-xtensa-esp32s3/xtensa-esp32s3-elf/lib/libm.a
LIBGCC  = /home/efim/.espressif/tools/xtensa-esp32s3-elf/esp-12.2.0_20230208/xtensa-esp32s3-elf/lib/gcc/xtensa-esp32s3-elf/12.2.0/libgcc.a
LDSCRIPT = /home/efim/Documents/PlatformIO/Projects/KOS_3/program/custom.ld
DEST    = /media/efim/3956-02B7/program.elf
INC_DIR = /home/efim/Documents/PlatformIO/Projects/KOS_3/src/launcher

# Файлы
SRCS    = $(wildcard *.c)
OBJS    = $(SRCS:%.c=build/%.o)
TARGET  = program.elf

# Флаги
CFLAGS  = -mtext-section-literals -O0 -c -I$(INC_DIR)
LDFLAGS = -T $(LDSCRIPT) -r

.PHONY: all clean copy objdump reloc symtab

all: $(TARGET) copy

# Сборка ELF из объектных файлов
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBC) $(LIBGCC) $(LIBM)

# Компиляция каждого .c в .o (в папку build)
build/%.o: %.c | build
	$(GCC) $(CFLAGS) $< -o $@

build:
	mkdir -p build

copy: $(TARGET)
	cp $(TARGET) $(DEST)

objdump:
	$(OBJDUMP) -D -s $(TARGET)

reloc:
	$(OBJDUMP) -r -s $(TARGET)

symtab:
	$(OBJDUMP) -t $(word 1, $(OBJS))

clean:
	rm -rf build $(TARGET)

tio:
	tio --map INLCRNL --baudrate 921600 /dev/ttyACM0