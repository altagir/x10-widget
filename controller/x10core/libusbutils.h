#ifndef LIBUSBUTILS_H
#define LIBUSBUTILS_H

#include <usb.h>

class LibUSButils
{
public:
    //were private
    static usb_dev_handle* GetFirstDevice(int vendorid, int productid);

    static int ahpInit(usb_dev_handle* handle);

    static void closeUsbDev(usb_dev_handle* portnum);

    static ssize_t read_intr_data(usb_dev_handle* portnum, char* readbuf, size_t readLen);
    static int write_intr_data(usb_dev_handle* portnum, char* writebuf, ssize_t bufLen);

private:
    unsigned char ahpRead(usb_dev_handle* hDevice, char* buffer, ushort* pnBytes);
    static void dumpinhex(char* buf, int size);
};


#endif // LIBUSBUTILS_H
