#!/bin/bash


ffmpeg -framerate 30 -i output/%05d.bmp -c:v libx264 -pix_fmt yuv420p -vf vflip output.mp4
