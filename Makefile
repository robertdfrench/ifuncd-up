banner=@echo "\n\033[0;31m\# $(strip $(1))\033[0m"
EXECUTABLES=$(patsubst code/%.c, %.exe, $(wildcard code/*.c))
.SECONDARY: $(EXECUTABLES)

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

%.dylibs: %.exe
	objdump -p $< | grep NEEDED

%.ldd: %.exe
	ldd $<

%.got: %.exe
	objdump -R $<

SPEED_DEMO_TIMES=speed_demo_ifunc.time \
		speed_demo_pointer.time \
		speed_demo_struct.time \
		speed_demo_fixed.time \
		speed_demo_always.time \
		speed_demo_upfront.time
speed_demo: $(SPEED_DEMO_TIMES) #: Is IFUNC slow?

RIGOROUS_SPEED_STATS=speed_demo_fixed.stats.txt \
		speed_demo_ifunc.stats.txt \
		speed_demo_pointer.stats.txt
rigorous_speed_demo: clean $(RIGOROUS_SPEED_STATS) #: Really, how slow is it?
	$(call banner, Final Results)
	@echo "TEST	LOW	HIGH	AVG"
	@printf "fixed\t"; cat speed_demo_fixed.stats.txt
	@printf "pointer\t"; cat speed_demo_pointer.stats.txt
	@printf "ifunc\t"; cat speed_demo_ifunc.stats.txt
	@echo ""

RIDICULOUS_STATS=fixed.stats.txt \
		ifunc.stats.txt \
		pointer.stats.txt \
		struct.stats.txt \
		always.stats.txt \
		upfront.stats.txt
ridiculous_speed_demo: clean $(RIDICULOUS_STATS) #: Compare other techniques
	$(call banner, Final Results)
	@echo "TEST	LOW	HIGH	AVG"
	@printf "fixed\t"; cat fixed.stats.txt
	@printf "pointer\t"; cat pointer.stats.txt
	@printf "struct\t"; cat struct.stats.txt
	@printf "ifunc\t"; cat ifunc.stats.txt
	@printf "upfront\t"; cat upfront.stats.txt
	@printf "always\t"; cat always.stats.txt
	@echo ""

%.stats.txt: %.low.txt %.high.txt %.avg.txt
	cat $^ | column -x -c 24 > $@

%.low.txt: %.timings.txt
	awk 'NR == 1 { l=$$1 }; { if ($$1 < l) { l=$$1 } }; END { print l }' $< > $@

%.high.txt: %.timings.txt
	awk 'NR == 1 { h=$$1 }; { if ($$1 > h) { h=$$1 } }; END { print h }' $< > $@

%.avg.txt: %.timings.txt
	awk '{ sum += $$1 }; END { print sum/NR }' $< > $@

%.timings.txt: code/speed_demo/%/main.exe
	$(call banner, Sampling performance of $<)
	(for i in `seq 1 10`; do \
		time -f "%U" ./$<; \
	done) 2>&1 | tee $@

tty_demo: tty_demo.exe #: Print color for tty, plaintext for file
	$(call banner, $@)
	./$<
	./$< | tee /dev/null

%.time: %.exe
	$(call banner, Timing $<)
	time -p ./$<

vector_add.exe: code/vector_add.c
	gcc -Wpsabi -o $@ $<

plt_example.exe: code/plt_example.c
	gcc -fPIC -no-pie -o $@ $<

cpu_demo.exe: code/cpu_demo.c
	gcc -o $@ $<

tty_demo.exe: code/tty_demo.c
	gcc -o $@ $<

speed_demo_%.exe: code/speed_demo/%/main.exe
	cp $< $@

code/speed_demo/%/main.exe:
	make -C code/speed_demo/$* main.exe

%.plt: %.exe
	objdump -d -r $< \
		| awk '/section/ { plt=0 }; /section .plt/ { plt=1 }; { if (plt) { print } }'

%.relro: %.exe
	checksec --file=./$<

docker_build.txt: Dockerfile
	docker build --tag ifuncd-up:latest .

enter_container: docker_build.txt #: Run an interactive shell in the container
	docker run -it -v $(PWD):/workspace ifuncd-up:latest

clean:  #: Remove any build detritus
	make -C code/speed_demo/always clean
	make -C code/speed_demo/fixed clean
	make -C code/speed_demo/ifunc clean
	make -C code/speed_demo/pointer clean
	make -C code/speed_demo/struct clean
	make -C code/speed_demo/upfront clean
	rm -f *.txt *.exe
