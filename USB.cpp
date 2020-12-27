#include "USB.h"
#include <iostream>

using namespace std;

USB::USB() {
    Init();
}

USB::~USB() {
    libusb_free_device_list(_devs, 1);
    libusb_exit(_ctx);
}

int USB::Init() {
    int r;
    r = libusb_init(&_ctx);
    if(r < 0) {
        LOG_ERROR("Init Error ",r);
        return 1;
    }
    libusb_set_debug(_ctx, 3);

    USB::GetDevice();

    return 0;
}

int USB::GetDevice() {
    ssize_t cnt;
    cnt = libusb_get_device_list(_ctx, &_devs);
    if(cnt < 0) {
        LOG_ERROR("Get Device Error");
        return 0;
    }
    LOG_INFO(cnt, " Devices in list.");

    return cnt;
}

int USB::FindVendor(uint16_t vendorID,  uint16_t productID) {
    libusb_device_descriptor desc;
    ssize_t cnt;
    for(int i = 0; i < cnt; i++) {
        int r = libusb_get_device_descriptor(_devs[i], &desc);
        if (r < 0) {
            cout << "failed to get device "<< i <<" descriptor"<< endl;
        }

        cout<<"\nNumber of possible configurations: "<<(int)desc.bNumConfigurations<<"\n";
        cout<<"Device Class: "<<(int)desc.bDeviceClass<<"\n";
        cout<<"VendorID: "<<desc.idVendor<<"\n";
        cout<<"ProductID: "<<desc.idProduct<<endl;

        if (vendorID == desc.idVendor && productID == desc.idProduct)
            return i;
    }

    return -1;
}

void USB::ShowDevices() {
    libusb_device_descriptor desc;
    ssize_t cnt = GetDevice();

    for(int i = 0; i < cnt; i++) {
        int r = libusb_get_device_descriptor(_devs[i], &desc);
        if (r < 0) {
            LOG_ERROR("failed to get device ", i, " descriptor");
        }

        cout<<"\nNumber of possible configurations: "<<(int)desc.bNumConfigurations<<"\n";
        cout<<"Device Class: "<<(int)desc.bDeviceClass<<"\n";
        cout<<"VendorID: "<<desc.idVendor<<"\n";
        cout<<"ProductID: "<<desc.idProduct<<endl;

        ShowInterfaces(i);

    }

}

void USB::ShowInterfaces(ssize_t dev_num) {
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(_devs[dev_num], 0, &config);
    cout<<"Interfaces: "<<(int)config->bNumInterfaces<<"\n";
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;
    for(int i=0; i<(int)config->bNumInterfaces; i++) {
        inter = &config->interface[i];
        cout << "\tNumber of alternate settings: "<<inter->num_altsetting<<"\n";
        for(int j=0; j<inter->num_altsetting; j++) {
            interdesc = &inter->altsetting[j];
            cout<<"\t\tInterface Number: "<<(int)interdesc->bInterfaceNumber<<"\n";
            cout<<"\t\tNumber of endpoints: "<<(int)interdesc->bNumEndpoints<<"\n";
            for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
                epdesc = &interdesc->endpoint[k];
                cout<<"\t\t\tDescriptor Type: "<<(int)epdesc->bDescriptorType<<"\n";
                cout<<"\t\t\tEP Address: "<<(int)epdesc->bEndpointAddress<<"\n";

            }
        }

    }
    cout<<endl<<endl<<endl;
    libusb_free_config_descriptor(config);

}

int USB::Write(uint16_t vendorID, uint16_t productID,
               int interface_number, unsigned char endpoint,
               unsigned char *data, int len) {
    libusb_device_handle *dev_handle;
    int r;
    int actual;

    dev_handle = libusb_open_device_with_vid_pid(_ctx, vendorID, productID);

    if(dev_handle == NULL)
        LOG_ERROR("Cannot open device");
    else
        LOG_INFO("Device Opened");

    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        LOG_INFO("Kernel Driver Active");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0)
            LOG_INFO("Kernel Driver Detached!");
    }

    r = libusb_claim_interface(dev_handle, interface_number);
    if(r < 0) {
        LOG_ERROR("Cannot Claim Interface");
        return 1;
    }

    actual = len;
    int curent_index=0;
    r = libusb_bulk_transfer(dev_handle, (endpoint | LIBUSB_ENDPOINT_OUT),
                             &data[curent_index], actual, &actual, 0);

    if(r == 0) {
        curent_index +=actual;
        LOG_INFO("Bytes: ", actual, " Writing Successful!");
    } else {
        LOG_ERROR("Write Error");
    }

    r = libusb_release_interface(dev_handle, 0);

    if(r!=0) {
        LOG_ERROR("Cannot Release Interface");
        return 1;
    }
    LOG_INFO("Released Interface");

}
