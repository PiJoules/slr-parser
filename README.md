## Dependencies
- g++ 4.9
  - Required for stable usage of <regex> library

## Tests

# Memory Check (Valgrind)
So running valgrind on any executables generated will show that there are 
exactly 72704 blocks of still reachable memory in 1 block. Although I am unable to free 
this, this will not be considered a bug b/c this is a fault of the c++ STL (http://valgrind.org/docs/manual/faq.html#faq.reports).

And aparently no one in GNU wants to fix this (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66339).

I also don't know how to use any other memory checkers lol.
