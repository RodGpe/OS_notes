CC = gcc
CFLAGS = -Wall -Wextra -O0

PROGS = demo_cpu_bound demo_io_bound demo_ctxt_switches demo_two_children_sched

.PHONY: all clean

all: $(PROGS)

demo_cpu_bound: demo_cpu_bound.c
demo_io_bound: demo_io_bound.c
demo_ctxt_switches: demo_ctxt_switches.c
demo_two_children_sched: demo_two_children_sched.c

$(PROGS): %: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGS)
