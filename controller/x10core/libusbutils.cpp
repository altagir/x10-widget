#include "libusbutils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"

#define EP1 0x81  // Input
#define EP2 0x02  // Output


usb_dev_handle* LibUSButils::GetFirstDevice(int vendorid, int productid)
{
    struct usb_bus*    bus;
    struct usb_device* dev;

    usb_dev_handle*    device_handle = NULL; // it's a null

    usb_init();
    usb_set_debug(0);       /* 3 is the max setting */

    usb_find_busses();
    usb_find_devices();

    // Loop through each bus and device until you find the first match
    // open it and return it's usb device handle

    for (bus = usb_busses; bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vendorid && dev->descriptor.idProduct == productid)
            {
                device_handle = usb_open(dev);
                // Need error checking for when the device isn't found!
                if (!device_handle)
                {
                    Logger::Log(ERROR, "Couldn't open usb device: %s", strerror(errno));
                    // errno = Exxxxx;      // usb_open should set the error code.
                    return NULL;
                }

                Logger::ClearLastError();
                return (device_handle);
            }
        }
    }
    Logger::SetLastError("No Such device");
    errno = ENOENT;     // No such device
    return (device_handle);
}


/*
 * In an excellent example of sacrificing backward compatability
 * for conformance to some proported "standard", the latest Linux
 * kernel USB drivers (uhci-alternate in 2.4.24+, and uhci in 2.6.x)
 * no longer support low speed bulk mode transfers-- they give
 * "invalid argument", errno=-22, on any attempt to do a low speed
 * bulk write.  Thus, we need interrupt mode transfers, which are
 * only available via the CVS version of libusb.
 *
 * (Thanks to Steven Michalske for diagnosing the true problem here.)
 */
unsigned char LibUSButils::ahpRead(usb_dev_handle* hDevice, char* buffer, ushort* pnBytes)
{
    // Synchronous read:
    long    nBytes = *pnBytes;

    // read
    //nBytes = usb_bulk_read(hDevice,    // handle (Not in 2.6)
    nBytes = usb_interrupt_read(hDevice,     // handle
                                EP1,     // which endpoint to read from
                                buffer,  // buffer to contain read results
                                *pnBytes,    // number of bytes to read
                                100);      // libusb timeout mS (does not work)

    if (nBytes < 0)
    {
        Logger::Log(ERROR, "ERROR reading %d", nBytes);
        return 0;
    }
    else
    {
        *pnBytes = (ushort)nBytes;
        return 1;
    }
}


/* Initialize CM19A so it recognizes signals from the CR12A or CR14A remote */
int LibUSButils::ahpInit(usb_dev_handle* handle)
{
    int ret;

    // Send this out on the control pipe
    char write_ok[] = { 0x20, 0x34, (char)0xcb, 0x58, (char)0xa7 };
    char init1[]    = { (char)0x80, 0x01, 0x00, 0x20, 0x14 };
    char init2[]    = { (char)0x80, 0x01, 0x00, 0x00, 0x14, 0x24, 0x20, 0x20 };

    ret = usb_interrupt_write(handle, EP2, write_ok, 5, 5000);
    Logger::Log(DEBUG, "Write_Ok : Wrote (%d) bytes", ret);

    sleep(1);

    ret = usb_interrupt_write(handle, EP2, init1, 5, 5000);
    Logger::Log(DEBUG, "Init1    : Wrote (%d) bytes", ret);

    sleep(1);

    ret = usb_interrupt_write(handle, EP2, init2, 8, 5000);
    Logger::Log(DEBUG, "Init2    : Wrote (%d) bytes", ret);

    sleep(1);

    return ret;
}


void LibUSButils::closeUsbDev(usb_dev_handle* portnum)
{
    usb_set_debug(0);       // We don't need this here

    if (portnum != 0)
    {
        Logger::Log(INFO, "Closing usb device");
        usb_release_interface(portnum, 0);
        usb_close(portnum);
        portnum = 0;
    }
}

void LibUSButils::dumpinhex(char* buf, int size)
{
    //OWERROR(0);
    char tmp[1024];
    tmp[0] = 0;

    if (buf[0] == 0x20)
    {
    } // else {

    for (int i = 0; i < size; i++)
    {
        sprintf(tmp, "%s%02X ", tmp, (unsigned char)buf[i]);
    }
    // }

    Logger::Log(DEBUG, "read size(%d) \"%s\"", size, tmp);
}


/**
 * Read data from the USB device's interrupt endpoint
 */
ssize_t LibUSButils::read_intr_data(usb_dev_handle* portnum, char* readbuf, size_t readLen)
{
    //  char readbuf[readLen+1];
//  int i = 10;
    int size;

//  while (i) // to loop through -110:  device temporary unavailable
    {
        size = usb_interrupt_read(
                   portnum,     // handle
                   EP1,         // which endpoint to read from
                   readbuf,     // buffer to contain read results
                   readLen,     // number of bytes to read
                   500);        // libusb timeout mS (does not work)

        if (size == -110)
        {
            Logger::Log(ERROR, "read_intr_data error: (-110: %s)", strerror(errno));
        }
        else if (size < 0)
        {
            // 110 = resource temporary unavailable
            Logger::Log(ERROR, "read_intr_data error: (%s)", strerror(errno));

            //      if (!i--)
            //      {
            //          // 10 trys in a row and we exit
            //          Logger::Log(ERROR, "Too many IntRead error, closing (%s)", strerror(errno));
            //          closeUsbDev(portnum);
            //          portnum = 0;
            //          return -1;
            //      }
        }
        else if (size > 0)
        {
            dumpinhex(readbuf, size);
            //      i = 10;
        }
    }

    return size;
}

/** Write raw data to USB device */
int LibUSButils::write_intr_data(usb_dev_handle* portnum, char* writebuf, ssize_t bufLen)
{
    return usb_interrupt_write(portnum, EP2, writebuf, bufLen, 5000);
}
