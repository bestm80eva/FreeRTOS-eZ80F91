/*
 * NexusMessage.cpp
 *
 *  Created on: Jun 21, 2016
 *      Author: juergen
 */

#include "NexusMessage.h"

#include <algorithm>
#include<sys/socket.h>
#include <errno.h>

NexusMessage::NexusMessage()
:_msg(0)
{
}

NexusMessage::~NexusMessage()
{
	clr();
}

unsigned char NexusMessage::operator [] (size_t idx) const
{
	if(!_msg)
		throw;
	return _msg->at(idx);
}

void NexusMessage::clr()
{
	if(_msg)
	{
		delete _msg;
		_msg = 0;
	}
}


size_t NexusMessage::read(int fd)
{
	uint16_t len;

	clr();

	// Read lenght-word anf flags-byte
	ssize_t s = recv(fd, &len, 2, MSG_PEEK|MSG_WAITALL);

	if(s == 2)
	{
		len = le16toh(len);
		_msg = new vector<uint8_t>(len,0);

		size_t s = recv(fd, _msg->data(), len, MSG_WAITALL);

		if(s != len || s != *this)
		{
			perror("recv");
			clr();
		}

//		cout << " readTCP:" << *this << endl;
	}
	else
		perror("recv");


	return (size_t) *this;
}

size_t NexusMessage::read(libusb_device_handle* handle, uint8_t endpoint)
{
	int s, len;

	clr();

	_msg = new vector<uint8_t>(MAIOSIZE,0);
	s = libusb_bulk_transfer(handle, endpoint, _msg->data(), MAIOSIZE, &len, 0);

	if( s || (size_t) (len & 0xFFFF) != *this)
	{
		cerr << " read libusb_bulk_transfer "
			 << hex << setw(2) << setfill('0') << (unsigned) endpoint
			 << ", len " << len
			 << ", " << (const char*) libusb_strerror((enum libusb_error) s) << endl;
		clr();

	}
	else
		_msg->resize(*this);
//	cout << " readUSB:" << *this << endl;
	return (size_t) *this;
}

size_t NexusMessage::write(int fd) const
{
	size_t len = *this;
//	cout << "writeTCP:" << *this << endl;
	if(len)
	{
		len = ::write( fd, _msg->data(), len);
		if(len != *this)
		{
			perror("write");
		}
	}
	return len;
}

size_t NexusMessage::write(libusb_device_handle* handle, uint8_t endpoint)
{
	int s,len = 0;

//	cout << "writeUSB:" << *this << endl;
	s = libusb_bulk_transfer(handle, endpoint, _msg->data(), *this, &len, 0);

	if( s || (size_t) len != *this)
	{
		cerr << "write libusb_bulk_transfer "
			 << hex << setw(2) << setfill('0') << (unsigned) endpoint
			 << ", len " << len << ", " << (const char*) libusb_strerror((enum libusb_error) s) << endl;
		len = 0;
	}
	return len;
}

ostream& operator << ( ostream &os, const NexusMessage& msg)
{
	int mode = 1;

	if(!(size_t)msg)
		os << "EMTY MESSAGE";
	else
	{
		int i = 0;
		os << "msg[" << dec << setw(4) << right << (size_t)msg << "], flg "
		   << hex << setw(2) << setfill('0') << (unsigned) msg[2] << ", data: ";

		for_each(msg._msg->begin(), msg._msg->end(), [&i, &mode, &os](uint8_t &n)
			{
				if(i++ >2 && isprint(n))
				{
					if(mode)
					{
						os << " \'";
						mode = 0;
					}
					os << n;
				}
				else
				{
					if(!mode)
					{
						os << '\'';
						mode = 1;
					}
					os << ' ' << hex << setw(2) << setfill('0') << (unsigned) n;
				}
			}
		);
		if(!mode)
			os << '\"';
	}

	return os;
}
