#!/bin/sh

# this script creates various files from the DBUS definition file
# The daemon responsible for answering DBUS calls is the process x10_service

# x10dbusservice.hpp is the definition file from which all other files are 
# generated. placed in controller/service, 
# implementation of this interface is controller/service/x10dbusservice.cpp

# STEP1: XML GENERATION
# We first generate  the xml from the new  (ca.cyberdine.x10.xml)
# this xml is the dbus definition file placed for system dbus in /etc/dus-1/system.d/thethe
# /etc/dbus-1/session.d for session based service (ends on logout)

# STEP2: DBUS ADAPTOR (Server)
# we then generate from the new xml and including the new hpp, a new class named 
# X10DbusAdaptor which is needed by the daemon implementing this dbus interface 
# (x10_service which is in controller/service)...

# STEP3: DBUS INTERFACE (Clients)
# and finally we generate X10DbusInterface in common,
# which is a dbus class helper for client apps (cmd and widget/plasmoid)

# SCRIPT_DIR is the location the script
SCRIPT_DIR="`dirname \"$0\"`"                 # relative
SCRIPT_DIR="`( cd \"$SCRIPT_DIR\" && pwd )`"  # absolutized and normalized
PRJ_DIR=$(dirname $SCRIPT_DIR)                # scripts/../

if [ -z "$PRJ_DIR" ] ; then
   # error; for some reason, the path is not accessible
   # to the script (e.g. permissions re-evaled after suid)
   exit 1  # fail
fi

cd $PRJ_DIR/controller/service

# STEP1: XML GENERATION
rm ca.cyberdine.x10.xml
qdbuscpp2xml x10dbusservice.hpp > ca.cyberdine.x10.xml

# STEP2: DBUS ADAPTOR (Server)
qdbusxml2cpp  ca.cyberdine.x10.xml  -a X10DbusAdaptor
# -i  controller/service/x10dbusservice.hpp

# STEP3: DBUS INTERFACE (Clients)
cd ../../common
qdbusxml2cpp  ../controller/service/ca.cyberdine.x10.xml -p X10DbusInterface
# -i  controller/service/x10dbusservice.hpp
