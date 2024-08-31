test: main.exe
	time -p ./main.exe

main.exe: main.o libincrement.so
	gcc -Wl,-rpath,$(CURDIR) -o $@ $< -L. -lincrement

libincrement.so: libincrement.o
	gcc -shared -o $@ $<

%.o: %.c
	gcc -fPIC -Wall -c $< -o $@

clean:
	rm -f *.exe *.o *.so
