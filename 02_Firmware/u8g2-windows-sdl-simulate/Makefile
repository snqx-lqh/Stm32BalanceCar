# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -O2 -Icsrc -ID:\SoftWare\Program\mingw64\x86_64-w64-mingw32\include\SDL2

# 链接选项
LDFLAGS = -LD:\SoftWare\Program\mingw64\x86_64-w64-mingw32\lib -lSDL2

# 目标可执行文件
TARGET = main.exe

# 源文件
SRCS = main.c

# u8g2 源文件
U8G2_SRCS = $(wildcard csrc/*.c)

# 所有源文件
ALL_SRCS = $(SRCS) $(U8G2_SRCS)

# 目标文件
OBJS = $(ALL_SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean