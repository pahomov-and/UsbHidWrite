#include "USB.h"
#include <iostream>

using namespace std;

USB::USB() {
    Init();
}

USB::~USB() {

    if (dev_handle != 0)
    {
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        dev_handle = 0;
    }

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


int USB::Connect() {
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

}


void USB::SetVidPid(uint16_t vID, uint16_t pID) {
vendorID = vID;
productID = pID;
}

int USB::ClaimInterface(int interface_number) {

    int r = libusb_claim_interface(dev_handle, interface_number);
    if(r < 0) {
        LOG_ERROR("Cannot Claim Interface");
        return r;
    }

    return 0;
}

int USB::ReleaseInterface(int interface_number) {

    int r = libusb_release_interface(dev_handle, 0);
    if(r!=0) {
        LOG_ERROR("Cannot Release Interface");
        return r;
    }
    LOG_INFO("Released Interface");
    return 0;
}

int USB::ReadISO(unsigned char endpoint) {

    struct libusb_transfer* transfer = libusb_alloc_transfer(512);

    libusb_fill_iso_transfer(transfer, dev_handle, endpoint, isobuf, 940 * 64, 64, callBackISO, NULL, 10000);
    return libusb_submit_transfer(transfer);
}

int USB::Write(unsigned char endpoint, unsigned char *data, size_t len) {
    int r;
    int actual;

    actual = len;
    int curent_index=0;
    r = libusb_bulk_transfer(dev_handle, (endpoint | LIBUSB_ENDPOINT_OUT),
                             &data[curent_index], actual, &actual, 0);
    if(r == 0) {
        curent_index +=actual;
    } else {
        LOG_ERROR("Write Error");
    }
}

/***
static void iso_callback(struct libusb_transfer *transfer){
    int i;
    int buf_index=0;
    for (i = 0; i < transfer->num_iso_packets; i++) {
        struct libusb_iso_packet_descriptor *desc =  &transfer->iso_packet_desc[i];
        unsigned char *pbuf = transfer->buffer + buf_index;
        buf_index+=desc->length;
        if (desc->actual_length != 0) {
            printf("isopacket %d received %d bytes:\n", i, desc->actual_length);
            print_bytes(pbuf, desc->actual_length);
        }
    }
    libusb_free_transfer(transfer);
}
***/
void USB::SetCallBack(CallBack_t call) {
    callBackISO = call;
}

