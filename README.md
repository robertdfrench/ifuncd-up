# IFUNC'd up
*Or: How I learned to stop blaming xz-utils for [CVE-2024-3094][2]*

![I think IFUNC'd up](larry.jpeg)

## What does GNU IFUNC even do?
It allows you to determine, at runtime, which version of some function you'd
like to use.

### Isn't that just function pointers?
No, actually, it's slower than function pointers, and requires custom tooling in
the compiler and the dynamic loader. Take a look at this:

```console
$ make speed_demo

# Timing speed_demo_fixed.exe
seq 1 5 \
        | xargs -n1 time -f "%U" ./speed_demo_fixed.exe 2>&1 \
        | awk ' { t+=$1 }; END { print "Average: "t/NR }'
Average: 2.834

# Timing speed_demo_ifunc.exe
seq 1 5 \
        | xargs -n1 time -f "%U" ./speed_demo_ifunc.exe 2>&1 \
        | awk ' { t+=$1 }; END { print "Average: "t/NR }'
Average: 7.502

# Timing speed_demo_pointer.exe
seq 1 5 \
        | xargs -n1 time -f "%U" ./speed_demo_pointer.exe 2>&1 \
        | awk ' { t+=$1 }; END { print "Average: "t/NR }'
Average: 4.598
```

So right off the bat, GNU IFUNC is a way to memoize a function's address that
is both less portable and more expensive than function pointers. I am going to
spend the rest of this page arguing that, because of things like CVE-2024-3094,
GNU IFUNC is even less secure than C function pointers.

[1]: https://robertdfrench.com/xz-utils.html
[2]: https://nvd.nist.gov/vuln/detail/CVE-2024-3094
