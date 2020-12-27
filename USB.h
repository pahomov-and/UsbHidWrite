#ifndef USBHIDWRITE_USB_H
#define USBHIDWRITE_USB_H

#include <libusb-1.0/libusb.h>
#include "LOG.h"

class IUSB {
public:
    virtual int Init() = 0;
    virtual int GetDevice() = 0;
    virtual int FindVendor(uint16_t vendorID, uint16_t productID) = 0;

};

class USB : public  IUSB {
public:
    USB();
    ~USB();
    int Init();
    int GetDevice();
    int FindVendor(uint16_t vendorID, uint16_t productID);
    void ShowDevices();
    void ShowInterfaces(ssize_t dev_num);

    int Write(uint16_t vendorID, uint16_t productID,
              int interface_number, unsigned char endpoint,
              unsigned char *data, int len);

private:
    libusb_context *_ctx = NULL;
    libusb_device **_devs;
};

#endif
