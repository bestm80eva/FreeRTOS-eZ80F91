/*
 * CUsbSmartCableDevice.h
 *
 *  Created on: Jun 22, 2016
 *      Author: juergen
 */

#ifndef CUSBSMARTCABLEDEVICE_H_
#define CUSBSMARTCABLEDEVICE_H_

#include "NexusMessage.h"

#include<libusb-1.0/libusb.h>
#include <pthread.h>

#include <string>

using namespace std;
/*
 *
 */

class CUsbSmartCableDevice
{
public:
	CUsbSmartCableDevice(libusb_device *dev);
	virtual ~CUsbSmartCableDevice();

	string Manufacturer();
	string Product();
	string SerialNumber();

	bool running() const { return _running;}
	bool start( int fd);
	void* stop();
protected:
	unsigned open();
	unsigned close();
	string ReadString(unsigned id);

private:
	libusb_device *_dev;
	libusb_device_handle* _handle;
	unsigned _ocrefcnt;
	int		_kerneldriver;
	string _serialnumber;
	uint8_t _BULK_OUT;
	uint8_t _BULK_IN;
	static void* threadUSB(void *obj);
	static void* threadTCP(void *obj);
	pthread_t 	_threadUSB;
	pthread_t 	_threadTCP;
	int			_fd;
	bool 		_running;
};

#endif /* CUSBSMARTCABLEDEVICE_H_ */
