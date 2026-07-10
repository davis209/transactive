
SUMMARY
-------

This ReadMe.txt contains basic usage instructions for the PASimulator.

QUICKSTART
----------

Start the PASimulator from a command line.  It gives a generic simulator
commands and warns "There are no modules loaded".  At the prompt, enter:

    addmod <name> PA <hostname> <port>
    start

Substituting where appropriate:
<name> an arbitrary name you give to the PA module
<hostname> <port> is the ip_address:port that the Simulator will bind to.

NOTE: 27000 is the SICD-specified port for the ISCS-PAAgent to connect with.

The Simulator responds with some lines of text announcing that it is starting.

From there, enter "help" for PA-specific commands. Currently supported are:

showTable <number>  displays the contents of the specified table (currently
                    only dumps raw hex)
update <number>     re-randomises a table (only works for 100,200,1200)
set603 <message#> <type> <label>
                    where
                    7<=message#<=500
                    type is one of [X|E|N]
                    Label is a 40 character DVA Message Label
status              reports on any agents that are connected.

