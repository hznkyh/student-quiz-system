# CITS3002 Quiz System Project

## Overview
This project is a 3-tier network-based system which tests students' ability to demonstrate their understanding of contemporary programming languages by correctly answering multi-choice questions and short programming challenges.


## Running the Question Bank (QB)
The question bank is written entirely in C.

TO RUN QB Open a terminal, then run the following commands:

1. Navigate to the QB directory.
```cd qb```
2. Run the makefile.
```make QBP QBC```
3. Compile the bash script for running the Question Banks.
```chmod +x runQBs```
4. Compule the bash script for quitting the Question Banks.
```chmod +x killQBs```
5. Finally, run the Question Bank bash script.
```./runQBs```
6. Once you want to quit the QBs if both are running in the background execute 
```./killQBs```
If in foreground, Ctrl + C, then `./killQBs` to kill the second QB.


When the QB is executed, the following text will be displayed in the terminal window:

```
Server running on 192.168.1.100:9001
* Ready to accept()
```

The QB will now be ready to accept communication with the test manager (TM). As seen in the first line, the QB displays the current IP address and port number it is able to receive connections on. This information is required to give the TM a location for which to connect.

### Question Bank Supporting Files
The question bank has multiple `.txt` files it has access to within its directory which store all relevant questions and answers. These files are required to allow the QB to run.

## Running the Test Manager (TM)
The test manager is written entirely in Python.

To run the test manager, the user must have Python3.5 or later installed on their machine (functionality is not guaranteed with earlier versions). Users may run the TM using the command `python3 TM.py`. This will promt the user to enter the IP address of the question bank which can be copied from the initial output from the QB.

```
Enter IP address:
> 
```

Once the IP address is entered, the TM will ask for the respective port number which is also displayed on the QB terminal window

```
Enter IP address:
> 192.168.1.100
Enter port:
> 
```

Once the port number is entered, the users default web browser will open and be directed to the login page where they can login. 

### Test Manager Supporting Files
The test manager has a supporting `studentRecords.json` file which is used to store all student info. This is accessed and changed dynamically during Test Manager runtime. 

## Client Webpage 
The client webpage is written in HTML, CSS and Javascript and in its current implementation is run locally on the same machine as the TM. 


