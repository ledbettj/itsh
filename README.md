itsh
====

itsh (it's a shell!) is a simple shell, just for funsies.  Thanks to the execellent GNU libc [Implementing a Shell](http://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html#Implementing-a-Shell) article.

### Building:

No fancy autotools or cmake here:

```
git clone git://github.com/ledbettj/itsh.git
cd itsh
make
./itsh
```

### Things it doesn't do (yet):

* job control
* IO redirection
* scripting
* stock market integration

### Things it does do:

* pipes (`ls -l | wc | awk '{print $1 * 1000 + $2 * 100 + $3 * 1}'`)
* run programs
* change directories

### Want to contribute?

* fork
* branch
* pull request
* wipe hands on pants

