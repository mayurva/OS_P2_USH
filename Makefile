includes = ush.h parse.h
objects = ush.o parse.o ush_util.o
out = ush

$(out): $(objects)
	cc -o $(out) $(objects)

$(objects): $(includes)

.PHONY: clean 
clean:
	rm -f $(out) $(objects)


