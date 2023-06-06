#!/bin/sh

#convert -background none $1.svg $2.png
#inkscape -z -w 64 -h 64 %1.svg -e %1.png
inkscape --export-type="png" $1.svg
