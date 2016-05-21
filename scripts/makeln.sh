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
sudo ln -sv $SRCDIR/build/lib/plasma_applet_x10.so $INSTALLDIR/lib/kde4/

sudo rm $INSTALLDIR/lib/kde4/plasma_engine_x10.so
sudo ln -sv $SRCDIR/build/lib/plasma_engine_x10.so $INSTALLDIR/lib/kde4/

sudo rm $INSTALLDIR/bin/x10
sudo ln -sv $SRCDIR/build/cmd/x10 $INSTALLDIR/bin/

sudo rm $INSTALLDIR/lib/kde4/libexec/x10_service
sudo ln -sv $SRCDIR/build/controller/service/x10_service $INSTALLDIR/lib/kde4/libexec/

sudo rm /etc/init.d/x10-daemon
sudo ln -sv $SRCDIR/install/x10-daemon /etc/init.d/x10-daemon

sudo rm /etc/apparmor.d/usr.lib.kde4.libexec.x10_service
sudo ln -sv $SRCDIR/install/usr.lib.kde4.libexec.x10_service /etc/apparmor.d/usr.lib.kde4.libexec.x10_service


