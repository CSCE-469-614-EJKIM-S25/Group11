#!/bin/bash

mkdir SHIP
cp ../hw4/LRU/* ./SHIP/
cd SHIP/
sed -i 's/LRU/SHIP/g' *.cfg
