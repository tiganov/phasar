#!/bin/sh

OUT_DIR="bin/"
OUT_LL="out.ll"

if [ "$1" == "-h" ]; then
  echo "Usage: 'basename $0' [swift file]"
  exit 0
elif (( $# != 1 )); then
  echo "Usage: 'basename $0' [swift file]"
  exit 0
fi

rm -rf $OUT_DIR
mkdir $OUT_DIR
swiftc -emit-ir -Onone $1 -o $OUT_DIR$OUT_LL
phasar -m $OUT_DIR$OUT_LL -D ifds-taint --swift 1 

