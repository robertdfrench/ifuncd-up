help:  #: Display this Help menu
	@echo "USAGE:\n"
	@awk -F':' '/#: / && !/awk/ { OFS="\t"; print "$$ make "$$1,"#"$$3 }' \
		$(MAKEFILE_LIST)

cpu_demo: cpu_demo.exe #: Detect CPU features
	./$<

tty_demo: tty_demo.exe #: Print color for tty, plaintext for file
	./$<
	./$< > tty_demo.txt
	cat tty_demo.txt

%.exe: %.c
	gcc -o $@ $<

clean:  #: Remove any build detritus
	rm -f *.txt *.exe
