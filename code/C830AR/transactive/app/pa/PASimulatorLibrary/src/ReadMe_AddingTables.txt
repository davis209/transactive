
                                The

                           Simple Guide

                                to

                        Adding Table Classes

                              to the

                          PASimulatorLibrary.



1.  Create a table class derived from PASimTable.  A convenient way of doing so
    is to copy from a pre-existing table class which data is most similar to
    your new class. Pay attention to overridding the following useful methods:
    setupDependents()
    providerChanged()

    Also make sure that any processing with the table's data needs to be
    network-byte-order aware, and that any structs used are "packed" if they
    are to be sucked directly from the underlying data.


2.  In PATableRequestProcessor, do the following:
    2.1 Include "app/pa/PASimulatorLibrary/src/PATableXXX.h"
    2.2 in the constructor, instantiate your new table class 

3.  Add the new .h and .cpp to the Makefile and PASimulatorLibrary.dsp (via
    the "Add Files to Folder..." in Visual Studio).

4.  Add the new files to the CVS repository

