main: strmatch.o
	g++ -g -DDEBUG  main.cpp rfc.cpp gpio.cpp device.cpp band.cpp strmatch.cpp -o main
.PHONY clean:
	@rm main *.o
	@rm band_name.txt
