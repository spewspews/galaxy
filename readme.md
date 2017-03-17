Galaxy
=

[![Build Status](https://travis-ci.org/spewspews/galaxy.svg?branch=master)](https://travis-ci.org/spewspews/galaxy)

Galactic n-body simulator using a Barnes-Hut tree. I wrote this originally for Plan 9 in C, this version is in C++ using SDL.
More documentation is in the man page file galaxy.1, read it with `man -l galaxy.1`

Examples
-

```
mkgalaxy -sq -av=-30 -d 80+1 3000 | mkgalaxy -i -av=-40 -d 100 -o 5000,0 -gv 0,100 5000 | galaxy -i

mkgalaxy -av 100 -d 60+50 -v 10 2000 |
mkgalaxy -i -av=-70 -d 80+50 -v 10 -o 6000,2000 -gv=-80,40 3000 |
galaxy -i

mkgalaxy -sq -av 100 -d 60+1 3000 | mkgalaxy -i -o 5000,3000 4000 -av=-50 | galaxy -i

mkgalaxy -av 100 -d 30+20 -sz 40+20 1000 | mkgalaxy -i -av 80 -d 40+30 -sz 20 -o 0,3000 -gv 130,-100 1500 | galaxy -i

mkgalaxy -av 40 -sq=true 8000 | mkgalaxy -i -av 50 -sq -o 10000,0 4000 | galaxy -i

mkgalaxy -av 100 -d 80+40 5000 | mkgalaxy -i -av=-150 -d 100+30 -sz 50+40 -o 3200,3005 2000 | galaxy -i

mkgalaxy -sz 50+40 5000 | mkgalaxy -i -sz 30+20 -o 4000,0 -v 50 -av 60 3000 | galaxy -i
```
