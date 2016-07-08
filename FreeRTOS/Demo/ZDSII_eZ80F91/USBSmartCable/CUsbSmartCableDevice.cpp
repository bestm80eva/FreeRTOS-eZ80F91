/*
 * CUsbSmartCableDevice.cpp
 *
 *  Created on: Jun 22, 2016
 *      Author: juergen
 */

#include "CUsbSmartCableDevice.h"

#include <unistd.h>
#include<sys/socket.h>
#include <iostream>

CUsbSmartCableDevice::~CUsbSmartCableDevice()
{
	close();
	libusb_unref_device(_dev);
}

CUsbSmartCableDevice::CUsbSmartCableDevice(libusb_device *dev)
:_dev(libusb_ref_device(dev))
,_handle(0)
,_ocrefcnt(0)
,_kerneldriver(0)
,_fd(-1)
,_running(false)
{
	struct libusb_config_descriptor *config;
	int r = libusb_get_config_descriptor 	(dev, 0, &config);
	if(r)
		throw;

	_BULK_OUT = config->interface[0].altsetting[0].endpoint[1].bEndpointAddress;
	_BULK_IN = config->interface[0].altsetting[0].endpoint[0].bEndpointAddress;
}

unsigned CUsbSmartCableDevice::open()
{
	int r;

	if(!_ocrefcnt)
	{
		r = libusb_open(_dev, &_handle);
		if(r)
		{
			cerr << "libusb_open" << (const char*) libusb_strerror((enum libusb_error) r) << endl;
			throw;
		}

		++_ocrefcnt;

		_kerneldriver = libusb_kernel_driver_active(_handle, 0);

		if(_kerneldriver == 1)
			r = libusb_detach_kernel_driver (_handle, 0);

		r = libusb_claim_interface (_handle,0);
		if(r)
		{
			cerr << "libusb_claim_interface" << (const char*) libusb_strerror((enum libusb_error) r) << endl;
			throw;
		}

		r = libusb_reset_device (_handle);
		if(r)
			cerr << "libusb_reset_device" << (const char*) libusb_strerror((enum libusb_error) r) << endl;
	}
	return _ocrefcnt;
}

unsigned CUsbSmartCableDevice::close()
{
	if(_ocrefcnt)
	{
		_running = false;

		libusb_reset_device(_handle);
		libusb_release_interface(_handle,0);
		libusb_close(_handle);
		//pthread_cancel(_threadUSB);
		//pthread_join(_threadUSB,0);
		::close(_fd);
		_ocrefcnt = 0;
	}
	return _ocrefcnt;
}

string CUsbSmartCableDevice::ReadString(unsigned id)
{
	unsigned char tmp[512];
	int r;

	open();
	r = libusb_get_string_descriptor_ascii(_handle, id, tmp, sizeof(tmp));

	if(r < 0)
		throw;

	return string((const char*)tmp);
}

string CUsbSmartCableDevice::Manufacturer()
{
	struct libusb_device_descriptor desc;
	if(!libusb_get_device_descriptor(_dev, &desc))
		return 	ReadString((unsigned) desc.iManufacturer);
	throw;
}

string CUsbSmartCableDevice::Product()
{
	struct libusb_device_descriptor desc;
	if(!libusb_get_device_descriptor(_dev, &desc))
		return 	ReadString((unsigned) desc.iProduct);
	throw;
}

string CUsbSmartCableDevice::SerialNumber()
{
	struct libusb_device_descriptor desc;
	if(!libusb_get_device_descriptor(_dev, &desc))
		return 	ReadString((unsigned) desc.iSerialNumber);
	throw;
}

bool CUsbSmartCableDevice::start( int fd)
{
	_fd = fd;
	if(!running())
	{
		open();
		_running = true;
		if(pthread_create(&_threadUSB, NULL, threadUSB , this) ||
		   pthread_create(&_threadTCP, NULL, threadTCP , this))
		{
			_running = false;
			perror("start");
		}
	}
	return running();
}

void* CUsbSmartCableDevice::threadUSB(void *obj)
{
	CUsbSmartCableDevice *o = static_cast<CUsbSmartCableDevice*>(obj);
	NexusMessage message;

	while(1/*o->running()*/)
	{
		size_t r = message.read(o->_handle, o->_BULK_IN);
		if(o->_fd == -1)
		{
			sleep(1);
			continue;
		}

		if(r >= 3 )
		{
			r = message.write(o->_fd);

			if(r != message)
				cerr << "tcp write size " << r << ", " << message << endl;
		}
		else
		{
			cerr << "libusb_bulk_transfer IN "  << r << ", " << message << endl;
		}
	}

	pthread_exit(0);
}

void* CUsbSmartCableDevice::threadTCP(void *obj)
{
	CUsbSmartCableDevice *o = static_cast<CUsbSmartCableDevice*>(obj);
	NexusMessage message;

	while(1)
	{
		size_t r = 0;
		while( o->_fd == -1)
			sleep(1);

		r = message.read(o->_fd);
		if(!r)
		{
			::close(o->_fd);
			o->_fd = -1;
			continue;
		}

		if(r == message )
		{
			r = message.write(o->_handle, o->_BULK_OUT);
			if(r != message)
			{
				cerr << "write " << r << ", " << message << endl;
				break;
			}
		}
		else
		{
			perror("read");
		}
	}
	pthread_exit(0);
}
