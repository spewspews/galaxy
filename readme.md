Galaxy
=

Galactic n-body simulator using a Barnes-Hut tree. I wrote this originally for Plan 9 in C, this version is in C++ using SDL.

Examples
-

```
mkgalaxy -sq -av=-30 -d 80+1 3000 | mkgalaxy -i -av=-40 -d 100 -o 5000,0 -gv 0,100 5000 | galaxy -i

mkgalaxy -av 100 -d 60+50 -v 10 2000 |
mkgalaxy -i -av=-70 -d 80+50 -v 10 -o 6000,2000 -gv=-80,40 3000 |
galaxy -i

mkgalaxy -sq -av 100 -d 60+1 3000 | mkgalaxy -i -o 5000,3000 4000 -av=-50 | galaxy -i
```
