SOURCE_FILES=band.cpp device.cpp gpio.cpp main.cpp match.cpp rfc.cpp
OBJS=$(SOURCE_FILES:%.cpp=%.o)

CPPFLAGS=-g 

main: $(OBJS)
	g++ $(CPPFLAGS) $(OBJS) -o main

%.o: %.c
	g++ -c $(CPPFLAGS) $< -o $@
	
.PHONY clean:
	rm -rf main *.o 2>/dev/null
	rm -rf code_changed 2>/dev/null
