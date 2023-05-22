To compile:
    make clean
    make all

To run:
    ./ospf -i id -f infile -o outfile -h hi -a lsai -s spfi

 In this, -i, -f and -o parameters are compulsory commandline inputs else the code wont run properly.
 -h, -a and -s parameters are optional as they have a default value give.
 The infile and outfile should contain the extension (eg .txt) else seg fault may occur

Will have to run terminals equal to number of nodes and run the code on each terminal with -i id changing from 0 to N-1

The input 1 and input 2 folders contain the input and outputs for the data in report
