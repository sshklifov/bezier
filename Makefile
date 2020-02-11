PREFIX ?= .
include Config.mk

$(EXECUTABLE): $(wildcard $(LIBDIR)/*.o)
	g++ $(CFLAGS) $(OPT) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
