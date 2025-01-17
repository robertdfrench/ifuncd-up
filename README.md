# IFUNC'd up
*Why you should stop blaming xz-utils for [CVE-2024-3094][nvd]. Also
check out my [ETSA Talk](https://www.youtube.com/watch?v=7j8ZCDGwSr8)!*

![I think IFUNC'd up](memes/larry.jpeg)

CVE-2024-3094, more commonly known as "The xz-utils backdoor", was a
near miss for global cybersecurity. Had this attack not been discovered
in the nick of time by [Andres Freund][freund], most of our planet's SSH
servers would have begun granting root access to the party behind this
attack.

Unfortunately, too much analysis has focused on how [malicious
code][JiaT75] made its way into the xz-utils repo.  Instead, I'd like to
argue that two longstanding design decisions in critical open source
software are what made this attack possible: [linking OpenSSH against
SystemD][biebl], and the existence of [GNU IFUNC][sourceware].

**Before You Start**: Much of this discussion deals with the intricacies
of dynamic linking on Linux. If you need a refresher, check out
[`dynamic_linking.md`](dynamic_linking.md).
 


## Quick Recap of CVE-2024-3094
There are tons of good writeups outlining the high level details of the
xz-utils backdoor, like Dan Goodin's [What we know about the xz Utils
backdoor that almost infected the world][goodin1] and Sam James' [FAQ on
the xz-utils backdoor (CVE-2024-3094)][thesamesam] gist. We don't need
to rehash all that here, so the purposes of this article, here is a
**very coarse** recap:

* Some Linux distros modify OpenSSH to depend on SystemD
* SystemD depends on xz-utils, which uses GNU IFUNC
* Ergo, xz-utils ends up in the address space of OpenSSH
* This allows ifunc to modify code in the SSH server

```mermaid
flowchart TD
    G["GNU IFUNC"]
    A["OpenSSH (OpenBSD)"]
    B["Portable OpenSSH<br/>(Linux / macOS / etc)"]
    C[OpenSSH + IFUNC]
    D[xz-utils]
    E["SystemD (Linux)"]
    A -->|Remove OpenBSD specifics| B
    B -->|Add SystemD specifics| C
    D --> E
    E --> C
    C --> F["Mayhem"]
    G --> D
```



## Why do Linux Distros modify OpenSSH?
The short answer is that they have to. OpenSSH is developed by the
OpenBSD community, for the OpenBSD community, and they do not give a
flying Fedora about Linux.  The [Portable OpenSSH][mindrot] project is a
best-effort collection of patches which replace OpenBSD-specific
components with generic POSIX components, and some platform-specific
code where applicable. The software supply-chain for SSH ends up looking
something like this in practice:

```mermaid
flowchart TD
  subgraph OpenBSD Folks
    A[OpenBSD]
    B[OpenSSH]
    H[improvements]
  end
  B-->A
  A-->H
  H-->B

  B-->C
  C[Portable OpenSSH]

  subgraph Debian Folks
    D[Debian SSH]
    G[improvements]
  end
  C-->D
  D-->G
  G-->C

  subgraph Fedora Folks
    J[Fedora SSH]
    K[improvements]
  end
  C-->J
  J-->K
  K-->C
```

OpenBSD's version of OpenSSH is upstream from everything else, and most
improvements to it come from within the OpenBSD community. These changes
flow downstream to the Portable OpenSSH project, which attempts to
re-implement new features in ways that aren't specific to OpenBSD. This
is what allows SSH to work on platforms like Linux, macOS, FreeBSD, and
even Windows. 

But it doesn't stop there. Some operating systems apply further
customization beyond what Portable OpenSSH provides. For example, Apple
adds the [`--apple-use-keychain`][keith] flag to `ssh-add` to help it
integrate with the macOS password manager.

In the case of CVE-2024-3094, Fedora and Debian maintained their own
[SystemD patches][biebl] for their forks of OpenSSH in order to fix a
[race condition around `sshd` restarts][schmidt]. So the *actual* supply
chain for SSH began to look like this:

```mermaid
flowchart TD
  A[OpenSSH]
  B[Portable OpenSSH]
  C[Debian SSH]
  D[Fedora SSH]
  A-->B
  B-->C
  B-->D
  C<-->|SystemD Patches|D
```

These patches never went into Portable OpenSSH, because the Portable
OpenSSH folks were ["not interested in taking a dependency on
libsystemd"][djmdjm]. And they never went into upstream OpenSSH, because
OpenBSD doesn't have any need to support SystemD.



### Concerns about "Separation of Concerns"
This seems harmless enough, but it's an example of a much larger problem
in Open Source, particularly in Linux: critical components of the
operating system are developed by people who don't know each other, and
don't talk to each other. 

* Did the folks who patched OpenSSH for SystemD know (or care) that
  libsystemd depends on xz-utils?
* Did the SystemD folks know (or care) that xz-utils had begun using
  ifunc?
* Did the OpenSSH folks know (or care) that ifunc was a thing? It's
  certainly not a thing on OpenBSD.

In some sense, this breakdown in communication is a feature of open
source: I can adapt your work to my needs without having to bother you
about it. But it can also lead to a degree of indirection that prevents
critical design assumptions (such as a traditional dynamic linking
process) from being upheld.

The obvious corollary to [Conway's Law][conway] is that if you are
shipping your org chart, you're also shipping the bugs that live in the
cracks of your org chart. No one person or team really made a mistake
here, but with the benefit of hindsight it's clear the attackers
perceived that the left hand of Debian/Fedora SSH did not know what the
right hand of xz-utils was doing.




## What is GNU IFUNC *supposed* do?
It allows you to determine, at runtime, which version of some function
you'd like to use. It does this by giving you an opportunity to run
**arbitrary code** to influence how the linker resolves symbols.

![](memes/ifunc_hard_right.png)



### Detecting CPU Features
Suppose you have an application that must run on a wide variety of x86
CPUs.  Depending on the specific features of the current CPU, you may
prefer to use different algorithms for the same task. The original idea
behind IFUNC was to allow programs to check for CPU features the first
time a function is called, and thereafter use an implementation that
will be most appropriate for that CPU.

Take a look at [`cpu_demo.c`](code/cpu_demo.c):

```c
void print_cpu_info() __attribute__((ifunc ("resolve_cpu_info")));
void print_avx2() { printf("AVX2 is present.\n"); }
void print_nope() { printf("AVX2 is missing.\n"); }

static void* resolve_cpu_info(void) {
    __builtin_cpu_init();

	if (__builtin_cpu_supports("avx2")) {
		return print_avx2;
	} else {
		return print_nope;
	}
}

int main() {
	print_cpu_info();
	return 0;
}
```

This program shows the most common use of IFUNC: it asks the CPU whether
or not it supports certain features, and provides a different
*implementation* of a function depending on what features are supported.
In this case, our function `print_cpu_info` will end up printing either
"AVX2 is present" or "AVX2 is missing" depending on how ancient your
CPU is.



### Probing the Process Environment
While IFUNC is intended for probing CPU capabilities, nothing stops you
from running more complicated code in your resolvers. For example,
[`tty_demo.c`](code/tty_demo.c) shows how you can load a different
function implementation depending on whether STDOUT is a file or a
terminal: 

```c
// Print Green text to the Terminal
void print_to_tty(const char *message) {
    const char *green_start = "\033[32m";
    const char *color_reset = "\033[0m";
    printf("%sTTY: %s%s\n", green_start, message, color_reset);
}

// Print plain text to a file
void print_to_file(const char *message) {
    printf("FILE: %s\n", message);
}

void print_message(const char *message) \
    __attribute__((ifunc("resolve_print_function")));

void (*resolve_print_function(void))(const char *) {
    struct termios term;

    // Ask the kernel whether stdout is a file or a tty
    int result = ioctl(STDOUT_FILENO, TCGETS, &term);
    if (result == 0) {
        // stdout is a terminal
        return print_to_tty;
    } else {
        // stdout is not a terminal
        return print_to_file;
    }
}

int main() {
    print_message("Hello, World!");
    return 0;
}
```

This is not really the intended use of IFUNC, but it shows what's
possible: you can run arbitrary code before `main` in any program that
uses an IFUNC that you've declared.





## IFUNC is Probably a Bad Idea
![](memes/ifunc_change_my_mind.png)

GNU IFUNC is difficult to implement, hard to use correctly, and (as an
alleged performance tool) isn't much faster than alternatives. As we've
seen with CVE-2024-3094, it is also a very powerful tool for software
supply-chain attacks.

IFUNC is used extensively within the GNU C Library, and that's probably
fine.  Those are the folks for whom it was originally developed, and
they are closely connected with the compiler and linker teams who
actually implement IFUNC. They are in the best position to understand
the tradeoffs, and there are tons of libc functions that benefit from
CPU-specific implementations. I believe we should consider IFUNC to be
an internal interface for glibc, and avoid its use in other
applications.



### It's too Confusing to Use Safely
ifunc is entirely too difficult to use. There are too many [corner
cases][nagy], and the [official documentation][gnu-cfa] is
[scant][sourceware]. This gives users the misleading idea that adopting
ifunc is straightforward.

Even several years after ifunc became available, the advertised
interface [did not work][agner]. GCC developers have called it [a
mistake][odonell] and have considered adding warnings to compensate for
IFUNC's fragility:

> The glibc solutions required to make IFUNC robust are not in place,
> and so we should do what we can to warn users that it might break.

It isn't just IFUNC either. Apple Mach-O has a similar feature called
`.symbol_resolver` which they ["regret adding"][rjmccall].



### It Undermines RELRO
By allowing arbitrary code to run while the Global Offset Table is still
writable, protections afforded by [RELRO](dynamic_linking.md#relro) are
[rendered moot][binarly-io]. 

This is important to note, because RELRO advertises itself as a way to
protect the integrity of dynamically-loaded symbols. From a user
perspective (you, as a user of the compiler and the linker), this
violates the [Principle of Least Astonishment][pola]: no reasonable
person would expect that *loading a dynamic library* should compromise
a safety feature designed to *protect dynamic libraries*.

![](memes/linker_social_engineering.png)



### It's Not Always Necessary
There are multiple other ways to handle this situation. They each have
different tradeoffs, but they are all far simpler than IFUNC. All of
these are more portable than IFUNC, easier to understand, and harder to
exploit.

> "Ifunc is just an utterly dumb way to do runtime microarch specific
> code selection."
>
> -- [Rich Felker](https://hachyderm.io/@dalias/112952237145378821),
> maintainer of [musl](https://musl.libc.org).


#### Global Function Pointers
IFUNC is attractive because it allows developers to express function
selection *declaratively* rather than *imperatively*. But doing this
imperatively is not actually that hard. Consider
[`static_pointer.c`](code/static_pointer.c), which resolves a global
function pointer at runtime:

```c
static int (*triple)(int) = 0;
int triple_sse42(int n) { return 3 * n; }
int triple_plain(int n) { return n + n + n; }

void print_fifteen() {
	int fifteen = triple(5);
	printf("%d\n", fifteen);
}

int main() {
	__builtin_cpu_init();
	if (__builtin_cpu_supports("sse4.2")) {
		triple = triple_sse42;
	} else {
		triple = triple_plain;
	}
	
	print_fifteen();
	return 0;
}
```

Is this really so bad that we need special gimmicks in the linker just
to avoid it?

One disadvantage to this approach is that the function pointer `triple`
is writable at runtime, whereas IFUNC+RELRO would ensure that the ifunc
addresses in the GOT are immutable once they have been resolved.
However, with a little extra legwork, we could use
[`mprotect(2)`][mprotect] to mark such pointers read-only.



#### Modifying `LD_PRELOAD`
If you know what CPU features your code needs, and you have a separate
copy of your dynamic library for each case, then you could accomplish
the same thing by specifying the right library with `$LD_PRELOAD` like
so:

```bash
#!/bin/bash
if (cat /proc/cpuinfo | grep flags | grep avx2 > /dev/null); then
	LD_PRELOAD=./myfunc_avx2.so ./my_app
else
	LD_PRELOAD=./myfunc_normal.so ./my_app
fi
```

(If you are unfamiliar with `LD_PRELOAD`, check out catonmat's ["A Simple
`LD_PRELOAD` Tutorial"][catonmat].)



#### Separate Binaries per Feature Combination
How many unique CPU feature combinations do you really need to support?
How many even exist?

On the face of it, this looks like a combinatorial explosion. There are
dozens of different vector arithmetic, virtualization, and security
extensions to the amd64 ISA. But these features do not occur
independently in the wild. For example, no CPU that has AVX-512 lacks
SSE4.2 or AES-NI.

Knowing which CPU features your application needs, and
which of them occur together on real chips, can help you determine
how many distinct binaries you'd have to ship. It may not be as many as
you'd expect. Most package managers allow you to run scripts at install
time; you could ship multiple binaries in a single rpm or deb file and
use install-time logic to choose the best one for the host CPU.



### It isn't Much Faster than Alternatives

> What's been obvious to me for a long time is that, even if there were
> a performance advantage to ifunc, it could only be when the entire
> function call is so short that call overhead can be a significant
> portion of overall time.
>
> -- [Rich Felker](https://hachyderm.io/@dalias/113074264762553873),
> maintainer of [musl](https://musl.libc.org).

Given that the usual justification for ifunc is performance-related, I
wanted to see how much overhead *ifunc itself* causes. After all, any
function worth optimizing is probably called frequently, so the overhead
of the function invocation is worth acknowledging.

To figure this out, I designed an experiment that would call a
*dynamically resolved* function over and over again in a tight loop.
Take a look at [`speed_demo_ifunc.c`](code/speed_demo_ifunc.c) and
[`speed_demo_pointer.c`](code/speed_demo_pointer.c).  These programs
both do the same work (incrementing a static counter), but the
incrementer functions are resolved in different ways: the former
leverages GNU IFUNC, and the latter relies on plain old function
pointers.

Here is the overall logic:

1. Call a resolver function to determine which incrementer to use.
1. Record this answer somewhere (in the GOT, or as a function pointer).
1. Call this incrementer function a few billion times to get an estimate
   of its cost.

As a control, there is also
[`speed_demo_fixed.c`](code/speed_demo_fixed.c) which does the same
incrementer work but without any dynamically resolved functions.  This
can be used to get a help estimate what part of the runtime is dedicated
to function invocation vs what part is just doing addition.

The Makefile target `rigorous_speed_demo` makes several runs of each of
these programs and produces some simple statistics about their
performance. These numbers will of course change based on your hardware,
but the `fixed` test should serve as a baseline for comparison.

| *Results* | LOW  | HIGH | AVG   |
|-----------|------|------|-------|
| fixed     | 2.93 | 4.20 | 3.477 |
| ifunc     | 9.50 | 10.56| 9.986 |
| pointer   | 6.23 | 7.44 | 6.791 |

What we see here is that ifunc has a not-insignificant overhead compared
to using a plain-old function pointer. On average, on my hardware, it
takes about twice as long to call an ifunc function 2 billion times as
it does to invoke a function pointer 2 billion times.

Does this matter in real life? Absolutely not. Functions that are worth
optimizing are much more expensive than the "increment by one" functions
that we are analyzing here. It is only interesting because GNU IFUNC
claims to be a boon for performance, yet seems to incur more cost than
function pointers.


#### Performance of Other Techniques
There are other techniques which are slower than ifunc. Take a look at the
`super_rigorous_speed_demo`, which brings to other experiments into play:
[`speed_demo_upfront.c`](code/speed_demo_upfront.c) and
[`speed_demo_always.c`](code/speed_demo_always.c).

`speed_demo_upfront.c` behaves similarly to `speed_demo_pointer.c`,
except that it stores the results of the cpu feature checks in global
variables rather than keeping track of a function pointer. This still
requires a "resolver" function to run first to determine which
implementation gets used, based on the value of these global variables.
This technique turns out to be slower than ifunc, but it is also safer
than storing function pointers: whereas function pointers can be set to
arbitrary values, boolean flags cannot. So an attacker able to modify
these variables can make the program *slower*, but cannot make the
program behave *differently*.

`speed_demo_always.c` is designed to be the slowest technique -- it
checks all the necessary CPU features every time an implementation is
needed and picks one on the fly. Curiously, this technique is not
significantly slower than anything else. It is only marginally slower
than ifunc in the case where we have just a single CPU feature to check. 

| TEST    | LOW  | HIGH  | AVG     |
|---------|------|-------|---------|
| fixed   | 5.02 | 5.70  | 5.37    |
| pointer | 6.40 | 7.02  | 6.66    |
| ifunc   | 8.56 | 11.11 | 9.64    |
| upfront | 9.24 | 9.41  | 9.33333 |
| always  | 10.07| 10.56 | 10.2333 |




## Conclusion
GNU IFUNC is a niche feature of gcc/ld.so that few people knew about
before it was used in CVE-2024-3094. It has non-obvious pitfalls and
insufficient documentation. By letting the linker run arbitrary code
before `main`, before critical parts of the process image have been
initialized and protected, it undermines one of the most basic
assumptions of programming: that the mere act of loading a library will
not inherently *change* your program.

The performance benefits of IFUNC are real, but not meaningfully better
than alternatives. The simplicity of deploying a single binary that is
optimized for multiple CPUs is very attractive, but it can be
accomplished with simpler techniques (like function pointers).

I believe IFUNC should be disabled by default in gcc. Enabling it should
require a scary looking flag, such as `--enable-cve-2024-3094`. Anyone
using it outside of libc should be expected to provide a rigorous,
well-researched argument that no alternative solution is appropriate.

![Yes, all shared libraries](memes/brain.png)

<!-- REFERENCES -->
[aes-ni]: https://en.wikipedia.org/wiki/AES_instruction_set
[agner]: https://www.agner.org/optimize/blog/read.php?i=167
[biebl]: https://salsa.debian.org/ssh-team/openssh/-/commit/818791ef8edf087481bd49eb32335c8d7e1953d6
[binarly-io]: https://github.com/binarly-io/binary-risk-intelligence/tree/master/xz-backdoor
[catonmat]: https://catonmat.net/simple-ld-preload-tutorial
[conway]: https://en.wikipedia.org/wiki/Conway's_law
[djmdjm]: https://github.com/openssh/openssh-portable/pull/251#issuecomment-2027935208
[fr0gger]: https://infosec.exchange/@fr0gger/112189232773640259
[freund]: https://www.openwall.com/lists/oss-security/2024/03/29/4
[gnu-cfa]: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-ifunc-function-attribute
[goodin1]: https://arstechnica.com/security/2024/04/what-we-know-about-the-xz-utils-backdoor-that-almost-infected-the-world/
[jasoncc]: https://jasoncc.github.io/gnu_gcc_glibc/gnu-ifunc.html#relocations-and-pic
[JiaT75]: https://github.com/tukaani-project/xz/commit/cf44e4b7f5dfdbf8c78aef377c10f71e274f63c0
[keith]: https://keith.github.io/xcode-man-pages/ssh-add.1.html#apple-use-keychain
[mindrot]: https://anongit.mindrot.org/openssh.git
[mprotect]: https://www.man7.org/linux/man-pages/man2/mprotect.2.html
[musl]: https://musl.libc.org
[nagy]: https://sourceware.org/legacy-ml/libc-alpha/2015-11/msg00108.html
[nvd]: https://nvd.nist.gov/vuln/detail/CVE-2024-3094
[odonell]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70082#c0
[OpenSSH9.8p1]: https://www.openssh.com/releasenotes.html#9.8p1
[openssh-unix-dev]: https://marc.info/?l=openssh-unix-dev&m=171288895109872&w=2
[pola]: https://en.wikipedia.org/wiki/Principle_of_least_astonishment
[rjmccall]: https://reviews.llvm.org/D139163#3993795
[schmidt]: https://bugzilla.redhat.com/show_bug.cgi?id=1381997#c4
[sourceware]: https://sourceware.org/glibc/wiki/GNU_IFUNC
[thesamesam]: https://gist.github.com/thesamesam/223949d5a074ebc3dce9ee78baad9e27#design
