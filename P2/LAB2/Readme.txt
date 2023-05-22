To compile:
    make clean
    make all
To run:
    ./dns portnum inputfile

example = ./dns 12000 input.txt
Run on a non priviledged port with input.txt containing the input data

For testing just input any domain name in format abc.pqr.xyz or input bye to kill all servers and terminate the client.
Any other format of input will give invalid message.

Will give the clients response on terminal. 
NR will print its outputs to NRoutput.txt.
RDS will print its outputs to RDSoutput.txt.
TLD will print its outputs to TLDoutput.txt.
ADS will print their outputs to ADSoutput.txt. 

The files are opened in append mode so make sure to delete the previous files else it will append to the previous data

Have submitted the script file and corresponding output files for the server outputs while the script was running on input file 1 on moodle.
