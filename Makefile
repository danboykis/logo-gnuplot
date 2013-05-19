CC = gcc
CFLAGS = `guile-config compile`
LIBS = `guile-config link`
COMPILE = $(CC) -c

EXECUTABLE = tortoise
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))

.PHONY: clean build run

run: build
	./$(EXECUTABLE)

build: $(EXECUTABLE)

$(EXECUTABLE): $(DEPS) $(OBJS)
	$(CC) $(LIBS) -o $(EXECUTABLE) $(OBJS)

%.d: %.c
	$(CC) -M $< > $@
	$(CC) -M $< | sed s/\\.o/.d/ > $@

%.o: %.c
	$(COMPILE) -o $@ $<

clean:
	-rm $(OBJS) $(EXECUTABLE) $(DEPS)

explain: 
	@echo "Executable: $(EXECUTABLE)"
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"
	@echo "Dependencies: $(DEPS)"

depend: $(DEPS)
	@echo "Dependencies are up to date"

-include $(DEPS)
