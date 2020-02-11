#!/bin/bash

SUBDIRS=$(find . -maxdepth 1 -not -path . -type d)

for dir in $SUBDIRS; do
    cp ../Module.mk $dir/Makefile
done

if [ "$1" = "c" ]; then
    cd ../ ; make clean ; cd src
    for dir in $SUBDIRS; do
        cd $dir
        if ! make clean ; then exit 1; fi
        cd ../
    done
    echo "removing Makefiles"
    for dir in $SUBDIRS; do
        rm -f $dir/Makefile
    done
    exit
fi

for dir in $SUBDIRS; do
    cd $dir
    make depend && make
    if [ $? != 0 ] ; then exit 1; fi
    cd ..
done

cd ../ ; make
