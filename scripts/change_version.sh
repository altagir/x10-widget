#!/bin/sh

### initialize variables

# SRCDIR is the location the script
SRCDIR="`dirname \"$0\"`"            # relative
SRCDIR="`( cd \"$SRCDIR\" && pwd )`" # absolutized and normalized
SRCDIR=$(dirname $SRCDIR)            # scripts/../

if [ -z "$SRCDIR" ] ; then
# error; for some reason, the path is not accessible
# to the script (e.g. permissions re-evaled after suid)
exit 1  # fail
fi    

PROGNAME=$(basename $SRCDIR)
if [ -z $PROGNAME ]; then
  echo "missing PROGNAME"
  return
fi

echo "Changing Version for $PROGNAME"
############################################


if [ -n "$DATEIN" ]; then
  CURRENT_DATE=$(grep -o 'Built [0-9]*/[0-9]*/[0-9]*' $DATEIN | cut -c 7-17)
  echo "Current version built on $CURRENT_DATE"
fi

# Current version in CMakeLists
CURRENT_MAJOR_VERSION=$(grep "set(MAJOR_VERSION" $SRCDIR/CMakeLists.txt | sed 's/[^0-9]*//g')
CURRENT_MINOR_VERSION=$(grep "set(MINOR_VERSION" $SRCDIR/CMakeLists.txt | sed 's/[^0-9]*//g')
CURRENT_PATCH_VERSION=$(grep "set(PATCH_VERSION" $SRCDIR/CMakeLists.txt | sed 's/[^0-9]*//g')
CURRENT_VERSION=$CURRENT_MAJOR_VERSION.$CURRENT_MINOR_VERSION.$CURRENT_PATCH_VERSION
echo $CURRENT_VERSION

### Get Version
echo "Increase? [M,m,r]"
read segment

case $segment in
M)
  echo "MAJOR increase."
  NEW_MAJOR_VERSION=`expr $CURRENT_MAJOR_VERSION + 1`
  NEW_MINOR_VERSION=0
  NEW_PATCH_VERSION=0
  ;;
m)
  echo "MINOR increase."
  NEW_MAJOR_VERSION=$CURRENT_MAJOR_VERSION
  NEW_MINOR_VERSION=`expr $CURRENT_MINOR_VERSION + 1`
  NEW_PATCH_VERSION=0
  ;;
r)
  echo "REVISION increase."
  NEW_MAJOR_VERSION=$CURRENT_MAJOR_VERSION
  NEW_MINOR_VERSION=$CURRENT_MINOR_VERSION
  NEW_PATCH_VERSION=`expr $CURRENT_PATCH_VERSION + 1`
  ;;
*)
  echo "Nothing selected, aborting"
  exit
  ;;
esac

NEW_VERSION=$NEW_MAJOR_VERSION.$NEW_MINOR_VERSION.$NEW_PATCH_VERSION
echo $NEW_VERSION

###

echo "Proceed? [Y/n]"
read ANS
if [ "$ANS" = "n" ]; then
        exit
fi

cd $SRCDIR

sed -i -e 's|'UNRELEASED'|'unstable'|' debian/changelog
dch --newversion=$NEW_VERSION

NEW_DATE=$(date -u +%d/%m/%Y)

sed -i -e 's|set(MAJOR_VERSION "[0-9]*")|set(MAJOR_VERSION "'$NEW_MAJOR_VERSION'")|' $SRCDIR/CMakeLists.txt
sed -i -e 's|set(MINOR_VERSION "[0-9]*")|set(MINOR_VERSION "'$NEW_MINOR_VERSION'")|' $SRCDIR/CMakeLists.txt
sed -i -e 's|set(PATCH_VERSION "[0-9]*")|set(PATCH_VERSION "'$NEW_PATCH_VERSION'")|' $SRCDIR/CMakeLists.txt

if [ -f "widget/plasmoid/plasma-applet_x10.desktop" ]; then
  echo "Updating widget/plasmoid/plasma-applet_x10.desktop"
  sed -i -e 's|'$CURRENT_VERSION'|'$NEW_VERSION'|' widget/plasmoid/plasma-applet_x10.desktop
fi

if [ -f "widget/data_engine/plasma-dataengine-x10.desktop" ]; then
  echo "Updating widget/data_engine/plasma-dataengine-x10.desktop"
  sed -i -e 's|'$CURRENT_VERSION'|'$NEW_VERSION'|' widget/data_engine/plasma-dataengine-x10.desktop
fi

if [ -f "cmd/main.cpp" ]; then
  echo "cmd/main.cpp"
  sed -i -e 's|'$CURRENT_VERSION'|'$NEW_VERSION'|' cmd/main.cpp
fi

###

echo "done"

