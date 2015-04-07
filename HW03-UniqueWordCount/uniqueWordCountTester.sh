#!/bin/sh

# Put any punctuation we want to remove here
PUNCT=";:,."

if [ x"$1" = "x" ]; then
        echo "You need to give this a filename."
        exit 1
fi

awk '{for(x=1;$x;++x)print $x}' "${1}" | tr "${PUNCT}" "@" | sed 's/@//g' | sort | uniq -c