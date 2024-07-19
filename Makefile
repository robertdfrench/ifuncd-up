banner=@echo "\n\033[0;31m\# $(strip $(1))\033[0m"
EXECUTABLES=$(patsubst src/%.c, %.exe, $(wildcard src/*.c))
.SECONDARY: $(EXECUTABLES)

help: $(MAKEFILE_LIST) #: Display this Help menu
	@echo "USAGE:\n"
	@awk -F':' '/#: / && !/awk/ { OFS=":"; print $$1,$$3 }' $< \
		| column -t -s':' \
		| sort

rll: cpu_demo speed_demo tty_demo #: Run all IFUNC demos
	@echo ""
test: all
check: all

cpu_demo: cpu_demo.exe #: Detect CPU features
	$(call banner, $@)
	./$<

ifunc_late: ifunc_late.exe
	./$<

speed_demo: speed_demo_ifunc.time speed_demo_pointer.time #: Is IFUNC slow?

rigorous_speed_demo: clean speed_demo_fixed.stats.txt speed_demo_ifunc.stats.txt speed_demo_pointer.stats.txt #: Really, how slow is it?
	$(call banner, Final Results)
	@echo "TEST	LOW	HIGH	AVG"
	@printf "fixed\t"; cat speed_demo_fixed.stats.txt
	@printf "pointer\t"; cat speed_demo_pointer.stats.txt
	@printf "ifunc\t"; cat speed_demo_ifunc.stats.txt
	@echo ""

super_rigorous_speed_demo: clean speed_demo_fixed.stats.txt speed_demo_ifunc.stats.txt speed_demo_pointer.stats.txt speed_demo_always.stats.txt speed_demo_upfront.stats.txt #: Really, how slow is it?
	$(call banner, Final Results)
	@echo "TEST	LOW	HIGH	AVG"
	@printf "fixed\t"; cat speed_demo_fixed.stats.txt
	@printf "pointer\t"; cat speed_demo_pointer.stats.txt
	@printf "ifunc\t"; cat speed_demo_ifunc.stats.txt
	@printf "upfront\t"; cat speed_demo_upfront.stats.txt
	@printf "always\t"; cat speed_demo_always.stats.txt
	@echo ""

%.stats.txt: %.low.txt %.high.txt %.avg.txt
	cat $^ | column -x -c 24 > $@

%.low.txt: %.timings.txt
	awk 'NR == 1 { l=$$1 }; { if ($$1 < l) { l=$$1 } }; END { print l }' $< > $@

%.high.txt: %.timings.txt
	awk 'NR == 1 { h=$$1 }; { if ($$1 > h) { h=$$1 } }; END { print h }' $< > $@

%.avg.txt: %.timings.txt
	awk '{ sum += $$1 }; END { print sum/NR }' $< > $@

%.timings.txt: %.exe
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

plt_example.exe: src/plt_example.c
	gcc -fPIC -no-pie -o $@ $<

modify_got: modify_got.exe modify_got_library.so #: Show that IFUNC allows the GOT to be modified
	LD_LIBRARY_PATH=. ./$<

modify_got.exe: src/modify_got.c
	gcc -o $@ $< -ldl

modify_got_library.so: src/modify_got_library.c
	gcc -shared -fPIC -Wl,-z,norelro -o $@ $<

%.exe: src/%.c
	gcc -o $@ $<

plt_example: plt_example.exe #: Get the hang of the Procedure Linkage Table
	objdump -d -r $< \
		| awk '/section/ { plt=0 }; /section .plt/ { plt=1 }; { if (plt) { print } }'

relro_example: plt_example.exe #: Show our RELRO status
	checksec --file=./$<

docker_build.txt: Dockerfile
	docker build --tag ifuncd-up:latest .

enter_container: docker_build.txt #: Run an interactive shell in the container
	docker run -it -v $(PWD):/workspace ifuncd-up:latest

clean:  #: Remove any build detritus
	rm -f *.txt *.exe
