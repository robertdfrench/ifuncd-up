banner=@echo "\n\033[0;31m\# $(strip $(1))\033[0m"

help: $(MAKEFILE_LIST) #: Display this Help menu
	@echo "USAGE:\n"
	@awk -F':' '/#: / && !/awk/ { OFS=":"; print $$1,$$3 }' $< \
		| column -t -s':' \
		| sort

all: cpu_demo speed_demo tty_demo #: Run all IFUNC demos

cpu_demo: cpu_demo.exe #: Detect CPU features
	$(call banner, $@)
	./$<

speed_demo: speed_demo_ifunc.exe speed_demo_native.exe #: Does IFUNC save time?
	$(call banner, $@)
	seq 1 5 | xargs -n1 time -f "%U" ./speed_demo_native.exe
	seq 1 5 | xargs -n1 time -f "%U" ./speed_demo_ifunc.exe

test: all #: Run all IFUNC demos

tty_demo: tty_demo.exe #: Print color for tty, plaintext for file
	$(call banner, $@)
	./$<
	./$< > tty_demo.txt
	cat tty_demo.txt

%.exe: %.c
	gcc -o $@ $<

clean:  #: Remove any build detritus
	rm -f *.txt *.exe
