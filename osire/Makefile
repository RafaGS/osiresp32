src = $(wildcard ./*.c)
obj = $(src:.c=.o)

CCFLAGS = -pthread -Wall -O2

osire: $(obj)
	$(CC) $(CCFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) osire
