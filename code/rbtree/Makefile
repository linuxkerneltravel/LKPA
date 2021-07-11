build_lib_sh=sh/build_lib.sh

.PHONY: build_lib main clean
main : build_lib main.c
	gcc main.c -lrbtree -o main
build_lib : rbtree.c rbtree.h ${build_lib_sh}
	chmod 755 ${build_lib_sh} && dos2unix ${build_lib_sh} && sh ${build_lib_sh}
clean :
	rm -f /lib64/librbtree.so && rm -rf lib && rm -f main
