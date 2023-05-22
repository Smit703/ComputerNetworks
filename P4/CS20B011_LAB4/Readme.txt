To compile:
    make clean
    make all

To run:
    ./SenderGBN -s localhost -p 20000 -l 512 -r 10 -n 400 -w 3 -b 10 -d
    ./ReceiverGBN -p 20000 -n 400 -e 0.0001 -d

