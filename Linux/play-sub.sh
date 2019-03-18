#!/bin/bash

cat recording-20190306_1002.pcm | tee >(play -q -t raw -e signed-integer -b 16 -c 1 -r 16000 - ) | ./ebclient -s mediator.pervoice.com -r -f en-EU-lecture_KIT-hybrid  -i cs -t text 2> output &

tail -f output | egrep 'received' | sed -r 's/^[^:]*: //' | python3 x.py
