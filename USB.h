#ifndef USBHIDWRITE_USB_H
#define USBHIDWRITE_USB_H

#include <functional>
#include <libusb-1.0/libusb.h>

#include "LOG.h"

typedef void(*CallBack_t)(libusb_transfer*);

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


    void SetVidPid(uint16_t vID, uint16_t pID);
    int ClaimInterface(int interface_number);
    int ReleaseInterface(int interface_number);
    int Connect();


    int ReadISO( unsigned char endpoint);
    int Write(unsigned char endpoint, unsigned char *data, size_t len);
    int WriteISO(unsigned char endpoint, unsigned char *data, size_t len);

    void SetCallBack(CallBack_t call);

private:
    libusb_context *_ctx = NULL;
    libusb_device **_devs;

    uint16_t vendorID;
    uint16_t productID;
    libusb_device_handle *dev_handle;

    CallBack_t callBackISO;

    unsigned char isobuf[65536];
};

#endif
