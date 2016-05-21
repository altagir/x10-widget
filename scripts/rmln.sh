#!/bin/sh

INSTALLDIR="/usr"

# SRCDIR is the location the script
SRCDIR="`dirname \"$0\"`"            # relative
SRCDIR="`( cd \"$SRCDIR\" && pwd )`" # absolutized and normalized
SRCDIR=$(dirname $SRCDIR)            # scripts/../

if [ -z "$SRCDIR" ] ; then
   # error; for some reason, the path is not accessible
   # to the script (e.g. permissions re-evaled after suid)
   exit 1  # fail
fi


sudo rm $INSTALLDIR/lib/kde4/plasma_applet_x10.so
sudo rm $INSTALLDIR/lib/kde4/plasma_engine_x10.so
sudo rm $INSTALLDIR/bin/x10
sudo rm $INSTALLDIR/lib/kde4/libexec/x10_service
sudo rm /etc/init.d/x10-daemon
sudo rm /etc/apparmor.d/usr.lib.kde4.libexec.x10_service


