# CITS3002 Quiz System Project

TCP/IP Server-Client for a Quiz System

Created by:
  Hank Zhang,
  Owen Smith,
  Sage Allen,
  John Lumagbas
  

## Overview


## Running the Question Bank (QB)
The question bank is written entirely in C.

To run a QB on a machine, the user can run the command `gcc QB.c -o qb` in terminal to create an executable QB object. The object can then be run with `./qb`. When the QB is executed, the following text will be displayed in the terminal window:

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

Once the port number is entered, the users default web browser will open and be directed to the login page.


## Client Webpage 
The client webpage is written in HTML, CSS and Javascript.






 
