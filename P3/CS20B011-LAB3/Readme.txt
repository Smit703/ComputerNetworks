To compile:
    make clean
    make all

To run:
    ./crc inputfile.txt outfile.txt

The outfile contains all the output data.
The inputfile has 128 bits strings each on a newline.
If crc check is failed it means error is detected by crc.

For crc-8 CCITT, all odd bit errors are detected and all burst errors of length <=8 are detected.
