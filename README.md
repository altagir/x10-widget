# x10-widget
X10 Plasma Applet - Sébastien Sénéchal <altagir@gmail.com>
--------------------------------------------------------------

Description
-----------
X10 Widget is a KDE4 interface to remotely control X10 devices using a CM15/19 X10 controller.

Source code
-----------
X10 Widget source code is hosted on opendesktop.org
The project homepage is http://opendesktop.org/content/show.php/x10+Controller+Plasma+Widget?content=157168

Requirements
------------
To run this application you need to have the following applications installed:

* KDE 4.2

Build Requirements
------------------
To build x10-widget you need the following:

	sudo apt-get install cmake libusb-dev kdelibs5-dev kdebase-workspace-dev

To test x10-widget, you may want the following:
	sudo apt-get install plasmate d-feet
	
	
-- Build instructions --

   cd /where/your/x10-widget/is/installed
   mkdir build
   cd build

   cmake -DCMAKE_INSTALL_PREFIX=/usr ..

   On systems where both qt4 and qt5-base are installed, `qmake` refers to the 5.x version, so force cmake to use Qt4 this way:
   Else you will encounter :
	CMake Error: The following variables are used in this project, but they are set to NOTFOUND.
	Please set them or make sure they are set and tested correctly in the CMake files: QT_QT_INCLUDE_DIR

	export QT_SELECT=4
	  or using this option in cmake:
	-DQT_QMAKE_EXECUTABLE=/usr/bin/qmake-qt4
  
 
   make
   make install
   
   kbuildsycoca4
   (first time install to have the .desktop file recognized by the plasma-desktop
   or everytime you modify the .desktop, either plasma or engine)
   
	
Restart plasma to load the applet 
(necessary to reload a widget on desktop/panel, not necessary for plasmoidviewer / plasma-windowed )
	kquitapp plasma-desktop; sleep 1; plasma-desktop                                            ~
	
or view it with either
	plasmoidviewer x10
	plasma-windowed x10

to view the service, use :
	plasmaengineexplorer --engine x10

to view KDE log output of running widgets:
cat / tail -f ~/.xsession-errors | grep X10


DEBUGGING:
----------
Environement variables:
export DEBUG=1	// to see debug msgs
export ENABLE_COLOR=0 // to disable color (like in kdevelop/qtcreator)

* There is a script "makeln.sh", which remove the installed libs, and soft-link
them to the dev versions... 
Please launch INSIDE x10-widget folder or UPDATE variable "SRCDIR"
convenient for debugging without having to reinstall the whole application.

Plasma attached to the already running plasma-desktop will not updated at every build, 
you need to kill plasma-desktop as described above. But for development, each time plasmoidviewer
launches it will take the latest compiled version if the .so are linked.


* I used kdevelop 4 for developping this widget. Open the .kdev4 project.

in Run->Launch Configurations, there are 5 configurations:
	- plasmoid			- using plasmoidviewer
		plasmoidviewer disappeared from the "raring" distribution of ubuntu so I used
		the one they shipped with plasmate source code.
	- windowed			- using plasma-windowed
		same as plasmoidviewer... not using much
	- engineexplorer	- using plasmaengineexplorer
		debugging plasma engine and jobs!!
	- x10
	- x10_service
		
These tools are used as launcher for the plasmoids, 
you can put breakpoints inside the code and follow paths.

* also scripts:
	- makeln.sh		-> use the current directory as target for KDE use / plasmoidviewer
	- rmWidget.sh		-> delete installed widgets, usefull if suddenly plasma-desktop crashes
	- change_version.sh	-> use for version controlling, adapt for your own purposes
	- regenerate_service.sh	-> regenerate DBus class adapter, interface classes and xml 
			from DBus header interface definition (x10dbusservice.hpp)
	- restartPlasma.sh	-> restart plasma-desktop


DEBIAN
------

cd build
cpack


Tutorials and resources
-----------------------

The explanation of the template
http://techbase.kde.org/index.php?title=Development/Tutorials/Plasma/GettingStarted

Plasma techbase pages
http://community.kde.org/Plasma

Credits:
--------
- http://www.linuxha.com/USB/cm19a.html for the libusb code


SERVICE OPERATIONS:
-------------------

 - Service are used to perform Jobs from plasmoids (like starts)
 - 2 set of operations (just for fun)
    * unit.operations:
       Call service on source "[A-P][1-16]" (valid X10 adress like "E4")
       Commands:
          Send(char command)  // {+,-,s,b,u,r,d,l}
          Set(int value)      // 0-7; -1 off

    * service.operations:
       Call service on source ""
       Commands:
          start/stop (queueManager, events still queued)
          AllOn/AllOff
          SendCommand(char channel, int unit, char command) // e 4 +
          SetCommand(char channel, int unit, int value)     // e 4 5



* Plasmoid (x10-widget) add source "" and get engine there to start/stop
  each controlling widget (control-widget) add sources "$X10adress" ("E4")
  and send commands directly to associated service (unit.operations)



