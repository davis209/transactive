#!/usr/bin/python

import sys
import binascii
import os
import socket
import struct
import time

BYTEMASK=0xAA

# there are 4 dictionaries of values. read and write requests, and for each of
# those a hex and bin.
requests_w_hex={}

gRemoteAddr='localhost'
gRemotePort=27000
gLocalAddr='localhost'
gLocalPort=26999

table_size={    100:51200,  200:64,
                1200:3200,  201:6,
                1201:202,   202:32,
                204:128,    1204:6400,
                301:17,     1301:213,
                302:12,     303:104,
                1303:888,   350:12,
                550:18,     560:8,
                570:1,      590:2,
                603:20254,  604:6,
                610:84,     1610:4200,
                630:6 }

write_errors={  0:'PAS_ERR_OK',
                1:'PAS_ERR_GENERIC',
                2:'PAS_ERR_BUSY_MESSAGE_SEQ',
                3:'PAS_ERR_BUSY_SOURCE',
                4:'PAS_ERR_BUSY_RECORDING',
                5:'PAS_ERR_BUSY_BROADCASTING',
                6:'PAS_ERR_INVALID_ANNOUNCE',
                7:'PAS_ERR_NO_FLAG',
                8:'PAS_ERR_NO_FIRE' }

#
# Set of printable characters. Not used.
# 
usableCharacters=' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~'
usableCharactersLength=len(usableCharacters)


##################################################################
def validateDataLength(tNum,received):
    expected=table_size[tNum]
    if (expected!=received):
        print "--->ERROR--->--->--->ERROR--->--->--->"
        print "--->ERROR: received size for table %d incorrect. Expected:%d Received:%d" % (tNum,expected,received)
        print "--->ERROR--->--->--->ERROR--->--->--->"


##################################################################
def processReadReply(tNum,data):
    datalen=len(data)
    if (datalen>512):
        print "Processing Read Reply for table %d: %d bytes" % (tNum,datalen)
        # print "data:",binascii.b2a_hex(data[:512]),"...full data",datalen,"bytes!!"
    else:
        print "Processing Read Reply for table",tNum,"data:",binascii.b2a_hex(data)

    validateDataLength(tNum,datalen)

    if (tNum==560):
        # if any of the bit patterns are '11', set them to '01' and send
        # a write request
        # As a short cut, we won't check if they are '11', we'll just clear
        # all the FLAG_A bits and send it back if it was non-zero to start with
        thelength=len(data)
        wasDirty=0
        blitzed=''
        for i in range(thelength):
            byte=(struct.unpack('B',data[i]))[0]
            addt='\x00'
            if (byte != 0):
                wasDirty=1
                # print 'data[i]=%s (%d)' % binascii.b2a_hex(ch),(struct.unpack('B',ch))[0]
                # print '(data[i] & 0x55)=',binascii.b2a_hex(data[i] & 0x55)
                addt=struct.pack('B',(byte & BYTEMASK))
            # print "Processed byte[%d]=%s" % (i,binascii.b2a_hex(addt))
            blitzed+=addt
        rtrn_rqst='W'+binascii.a2b_hex('02300008')+blitzed
        # print "Formed 560 write request:", binascii.b2a_hex(rtrn_rqst)
        return (wasDirty,rtrn_rqst)
    return (0,'')


##################################################################
def processWriteReply(tNum,value):
    print "Processing Write Reply for table %d. Response=%s" % (tNum,write_errors[value])


##################################################################
def makeReadRequest(tableNumber):
    hexstr="52%0.4x" % tableNumber
    return hexstr


##################################################################
def makeWriteRequest(tNum):
    global usableCharacters
    global usableCharactersLength
    length=table_size.get(tNum,0)
    hexstr='57%0.4x%0.4x' % (tNum,length)
    charidx=0x20
    for x in range(length):
        if charidx>0x7e:
            charidx=0x20
        hexstr+='%0.2x' % charidx
        charidx+=1
    return hexstr


##################################################################
def sendReadRequest(tNum,thesock):
    # print 'sending table',tNum,'read request:',requests_r_hex[tNum]
    # thesock.send(binascii.a2b_hex(requests_r_hex[tNum]))
    r=makeReadRequest(tNum)
    print 'sending table',tNum,'read request:',r
    thesock.send(binascii.a2b_hex(r))
    sys.stdout.flush()


##################################################################
def sendWriteRequest(tNum,thesock):
    request=makeWriteRequest(tNum)
    reqleng=len(request)
    # if (reqleng<256):
        # print 'sending table',tNum,'write request:',request
    # else:
    print 'sending table',tNum,'write request:',reqleng,'bytes'
    thesock.send(binascii.a2b_hex(request))
    sys.stdout.flush()


##########################################################
def usage():
    from os.path import basename
    print """
    Usage: %s OPTIONS
    
         where OPTIONS is one of:
         --remote-addr=x.x.x.x  ip address of the machine hosting PASimulator (default:localhost)
         --remote-port=x        port of the remote PASimulator (default:27000)
         --local-addr=x.x.x.x   ip address that the socket of this script will bind to
         --local-port=x         port that the socket of this script will bind to
         -h|--help              print this message
""" % basename(sys.argv[0])

###########################################################
def usage_extra():
    print """
        [NO EXTRA HELP TODAY]
"""

##################################################################
def parseCmdLineOptions():
    import getopt
    try:
        global gRemoteAddr
        global gRemotePort
        global gLocalAddr
        global gLocalPort
        cmdoptions= ['remote-addr=','remote-port=','local-addr=','local-port=']
        cmdoptions+=['help','help-extra']
        opts, pargs = getopt.getopt(sys.argv[1:], 'h',cmdoptions)
        for opt,arg in opts:
            if (opt == '--remote-addr'):
                gRemoteAddr=arg
            elif (opt == '--remote-port'):
                gRemotePort=int(arg)
            elif (opt == '--local-addr'):
                gLocalAddr=arg
            elif (opt == '--local-port'):
                gLocalPort=int(arg)
            elif (opt == '--help-extra'):
                usage()
                usage_extra()
                sys.exit(0)
            elif (opt == '-h') or (opt == '--help'):
                usage()
                sys.exit(0)
    except getopt.GetoptError,e:
        print "ERROR: unrecognised option or argument (%s): %s" % (e.opt,e.msg)
        usage()
        sys.exit(1)


parseCmdLineOptions()

##################################################################
# WRITE REQUESTS
#
# write request for table 200
# 57 = 'W'
# 00C8 = 200 (table number)
# 0040 = 64 (table size)
# the table data is hex 30 thru hex 6F, ie. ascii characters '0' (zero) thru 'o' (lowercase oh)
requests_w_hex[200]='5700c80040303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f'

# print "Bin Write Requests",requests_w_bin
# sys.exit(0)


print "Remote = %s:%d" % (gRemoteAddr,gRemotePort)
print "Local  = %s:%d" % (gLocalAddr,gLocalPort)
sys.stdout.flush()
# time.sleep(10)
# sys.exit(0)


#
#
thesock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

thesock.bind((gLocalAddr,gLocalPort))
tryport=gRemotePort
good=0
while not good:
    try:
        print "Attempting connect((%s,%d))" % (gRemoteAddr,tryport)
        sys.stdout.flush()
        thesock.connect((gRemoteAddr,tryport))
        good=1
    except socket.error,e:
        good=0
        print "Port Error:",tryport,e
        sys.stdout.flush()
        tryport+=1
        if (tryport>27003):
            sys.exit(1)


#
# Only connect when we're good and ready
try:
    input("Waiting for keypress...")
except EOFError,e:
    pass
except SyntaxError,e:
    pass


#
# FastPoll tables
sendReadRequest(200,thesock)
sendReadRequest(1200,thesock)
sendReadRequest(302,thesock)
sendReadRequest(560,thesock)

for thekey in table_size.keys():
    sendReadRequest(thekey,thesock)

#
# FastPoll tables
sendReadRequest(200,thesock)
sendReadRequest(1200,thesock)
sendReadRequest(302,thesock)
sendReadRequest(560,thesock)

#
# These are the tables marked as ISCS->PA in the SICD Appendix A.
#
sendWriteRequest(201,thesock)
sendWriteRequest(1201,thesock)
sendWriteRequest(301,thesock)
sendWriteRequest(1301,thesock)
sendWriteRequest(350,thesock)
sendWriteRequest(570,thesock)
sendWriteRequest(604,thesock)
sendWriteRequest(630,thesock)

sendReadRequest(201,thesock)
sendReadRequest(1201,thesock)
sendReadRequest(301,thesock)
sendReadRequest(1301,thesock)
sendReadRequest(350,thesock)
sendReadRequest(570,thesock)
sendReadRequest(604,thesock)
sendReadRequest(630,thesock)

#
# FastPoll tables
sendReadRequest(200,thesock)
sendReadRequest(1200,thesock)
sendReadRequest(302,thesock)
sendReadRequest(560,thesock)


# thesock.listen(4)
received=''

##################################################################
#
# Main Loop
#
while (1):
    try:
        received += thesock.recv(65535)
    except socket.error,e:
        print
        print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        print "Socket Exception:",e
        print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        sys.exit(0)

    received_hex=binascii.b2a_hex(received)
    print '-------------------------------'

    # 
    # print some introductory statistics about the packet
    # 
    longness=len(received)
    packetType=received[0]
    tableNumber=socket.ntohs((struct.unpack('H',received[1:3]))[0])
    if (packetType=='W'):
        print "packetType =",packetType
        print "tableNumber =",tableNumber
        replyValue=(struct.unpack('B',received[3:4]))[0]
        print "replyValue =",replyValue
        #
        # process a write reply packet.  These are always short
        #
        # print "Received table 560 WRITE reply"
        # rdata=received[5:]
        processWriteReply(tableNumber,replyValue)
        if (tableNumber==560):
            sendReadRequest(560,thesock)

    elif ( packetType == 'R' ):
        #
        # process a read reply packet.  These are as long as the table they
        # requested (plus 5 bytes)
        # 
        tableSize=socket.ntohs((struct.unpack('H',received[3:5]))[0])
        if (longness<tableSize+5):
            # we don't yet have all the reply
            print "(Still receiving data for table %d)" % tableNumber
            continue
        else:
            print "packetType =",packetType
            print "tableNumber =",tableNumber
            print "tableSize =",tableSize

        # print "Received table",tableNumber,"READ reply"
        rdata=received[5:]
        (dirty,wdata)=processReadReply(tableNumber,rdata)
        if (dirty):
            thesock.send(wdata)
            print 'Sent packet:',binascii.b2a_hex(wdata)


    #
    # some terminals don't do this (like cygwin's xterm !!)
    #
    sys.stdout.flush()

    #
    # reset the received data buffer
    #
    received=''


# vim:tabstop=4:expandtab:shiftwidth=4
