#!/bin/sh
if [ ! -d "lib" ];then
    mkdir lib
fi
gcc -shared rbtree.c -o lib/librbtree.so
ln -sf  ${PWD}/lib/librbtree.so /lib64/librbtree.so
