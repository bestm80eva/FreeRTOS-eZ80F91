/*
 * smartcable.cpp
 *
 *  Created on: Jun 21, 2016
 *      Author: juergen
 */

/*
 C socket server example
 */

#include "NexusMessage.h"
#include "CUsbSmartCableDevice.h"

#include<stdint.h>
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>

#include<libusb-1.0/libusb.h>


using namespace std;

/*
 * Vendor Id : 04e3
 * Product Id: 0001
 * SNo:"101201-0021"
 */
const uint16_t vendor_id = 0x04e3;
const uint16_t product_id= 0x0001;
string   defserialno;

static map<const string, CUsbSmartCableDevice*> device;


int initUSB()
{
	libusb_device **devs, *tmp;
	libusb_context* ctx = NULL;
	int cnt,i,j,r;
	uint8_t path[8];

	r = libusb_init(&ctx);
	if( r == LIBUSB_SUCCESS)
	{
		// libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);

		cnt = libusb_get_device_list(ctx, &devs);
		if (cnt >= 0)
		{
			i = 0;
			while((tmp = devs[i++]) != NULL)
			{
				struct libusb_device_descriptor desc;
				r = libusb_get_device_descriptor(tmp, &desc);

				if (r < 0) {
					cerr << "failed to get device descriptor " << (const char*) libusb_strerror((enum libusb_error) r) << endl;
					break;
				}
				r = libusb_get_port_numbers(tmp, path, sizeof(path));
				if(desc.idVendor == vendor_id && desc.idProduct == product_id)
				{

					CUsbSmartCableDevice *dev = new CUsbSmartCableDevice(tmp);
					device[dev->SerialNumber()] = dev;
					if(defserialno.empty())
						defserialno = dev->SerialNumber();

				}
			}
			libusb_free_device_list (devs,1);

		}
		else
		{
			cerr << "libusb_get_device_list " << (const char*) libusb_strerror((enum libusb_error) r) << endl;
		}

	}
	else
	{
		cerr << "libusb_init " << (const char*) libusb_strerror((enum libusb_error) r) << endl;
	}
	return device.size();
}


int main(int argc, char *argv[])
{

	int socket_desc, client_sock, iosize; // hier kann ich zahlen speichern und der speicher bekommt die namen
	struct sockaddr_in server, client;
	static const char serno[] = "SNO=";
	char buffer[20];

	if(!initUSB())
		return 1;

	for_each(device.begin(), device.end(), [] (const std::pair<const string, CUsbSmartCableDevice*>&pair)
		{
			cout << (char)(pair.first == defserialno ? '*':' ') << pair.first << ':'  << pair.second->Product() << ", " << pair.second->Manufacturer() << endl;
		}
	);

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}
	puts("Socket created");

	int enable = 1;
	if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
	    perror("setsockopt(SO_REUSEADDR) failed");
	    return 1;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(4040);

	//Bind
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	if (listen(socket_desc, 1))
	{
		//print the error message
		perror("listen failed. Error");
		return 1;
	}

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	do
	{
		iosize = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *) &client,
				(socklen_t*) &iosize);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");

		iosize = recv(client_sock, buffer, sizeof(buffer),MSG_PEEK);
		if(iosize < 3 || iosize != *(uint16_t*)buffer)
		{
			if(!iosize)
				cout << "Dissconnect" << endl;
			else
				perror("recv");

			close(client_sock);
			continue;
		}

		string pserno(defserialno);

		if(iosize > (int)(3+strlen(serno)) && !strncasecmp(buffer+2,serno,strlen(serno)))
		{
			pserno = buffer+3+strlen(serno);
			recv(client_sock, buffer, sizeof(buffer),0);
		}

		if(device.find(pserno) != device.end())
		{
			CUsbSmartCableDevice *scd = device.at(pserno);
			scd->start(client_sock);
		}
		else
			close(client_sock);
	} while (1);
	return 0;
}
