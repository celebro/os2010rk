OBJECTS = lcd_demo.o
CC = arm-linux-gcc
CFLAGS =
RM = rm -rf
EXEC = lcd_demo

.PHONY: all clean

%.o: %.c
	@echo 'Building file: $<'
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo 'Finished building file: $<'
	@echo ' '

all: $(EXEC)

$(EXEC): $(OBJECTS)
	@echo 'Building target: $@'
	$(CC) -o $@ $(OBJECTS) -L./ -llcd
	@echo 'Finished building target: $@'
	@echo ' '

lcd_demo.o: lcd_demo.c lcd_demo.h display_lib.h

clean:
	$(RM) $(OBJECTS) $(EXEC)
