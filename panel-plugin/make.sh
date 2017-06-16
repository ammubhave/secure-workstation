#!/bin/sh

main=secure-workstation-plugin

gcc -Wall -shared -o lib${main}.so -fPIC exitbutton.c `pkg-config --cflags --libs libxfce4panel-2.0` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs exo-1`
#gcc -Wall -shared -o lib${main}.so -fPIC ${main}.c `pkg-config --cflags --libs libxfce4panel-2.0` `pkg-config --cflags --libs gtk+-3.0`
