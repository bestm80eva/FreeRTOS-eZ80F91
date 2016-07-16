/*
 * NexusMessage.h
 *
 *  Created on: Jun 21, 2016
 *      Author: juergen
 */

#ifndef NEXUSMESSAGE_H_
#define NEXUSMESSAGE_H_
#include <iostream>
#include <iomanip>
//#include <cstring>
#include <vector>

#include <endian.h>
#include<libusb-1.0/libusb.h>
#include <unistd.h>

using namespace std;


/*
 *
 */
#define MAIOSIZE 8192 //4096

class NexusMessage
{
public:
	NexusMessage();
	virtual ~NexusMessage();

	unsigned char operator [] (size_t idx) const;
	operator size_t () const { return (size_t) (_msg ? le16toh(*(uint16_t*)_msg->data() ) : 0);}
	operator uint8_t* () {return _msg ? _msg->data():0;}
	operator const uint8_t* () const {return _msg ? _msg->data():0;}
	void clr();

	size_t read(int fd);
	size_t read(libusb_device_handle* handle, uint8_t endpoint);
	size_t write(int fd) const;
	size_t write(libusb_device_handle* handle, uint8_t endpoint) ;

	friend ostream& operator << ( ostream &os, const NexusMessage& msg);

protected:
	bool tout(char c, unsigned t) { cout << c << flush; usleep(t); return true;}
	vector<uint8_t>*  _msg;
};

#endif /* NEXUSMESSAGE_H_ */
