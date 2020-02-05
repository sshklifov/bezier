#!/bin/bash

export PREFIX=../..

SUBDIRS=$(find . -maxdepth 1 -not -path . -type d)

if [ "$1" = "c" ]; then
    for dir in $SUBDIRS; do
        rm -f $dir/*.o $dir/Makefile
    done
    # TODO
    exit
fi

for dir in $SUBDIRS; do
    cp ../Makefile.gen $dir/Makefile
done

for dir in $SUBDIRS; do
    cd $dir
    make depend && make
    if [ $? != 0 ] ; then
        exit
    fi
    cd ..
done

cd ../
make
