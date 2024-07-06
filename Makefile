banner=@echo "\n\033[0;31m\# $(strip $(1))\033[0m"

help: $(MAKEFILE_LIST) #: Display this Help menu
	@echo "USAGE:\n"
	@awk -F':' '/#: / && !/awk/ { OFS=":"; print $$1,$$3 }' $< \
		| column -t -s':' \
		| sort

all: cpu_demo speed_demo tty_demo #: Run all IFUNC demos
	@echo ""
test: all
check: all

cpu_demo: cpu_demo.exe #: Detect CPU features
	$(call banner, $@)
	./$<

speed_demo: speed_demo_fixed.time speed_demo_ifunc.time speed_demo_pointer.time #: Does IFUNC save time?

tty_demo: tty_demo.exe #: Print color for tty, plaintext for file
	$(call banner, $@)
	./$<
	./$< | tee /dev/null

%.time: %.exe
	$(call banner, Timing $<)
	seq 1 5 \
		| xargs -n1 time -f "%U" ./$< 2>&1 \
		| awk ' { t+=$$1 }; END { print "Average: "t/NR }'

%.exe: %.c
	gcc -o $@ $<

clean:  #: Remove any build detritus
	rm -f *.txt *.exe
