//============================================================================
// Name        : CPMRDisk.cpp
// Author      : JSievers@Augenpunkte.de
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
using namespace std;
#include "Interface.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addrlinux 64 pointer size
#include <unistd.h>	//write
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>

pkt_t pkt;
uint16_t sequenz = 1;

const char *dname[10] = {"disks/drivea.cpm","disks/driveb.cpm","disks/drivec.cpm","disks/drived.cpm"
		,"","","","","disks/drivei.cpmhd","disks/drivej.cpmhd"};

class Drive
{
public:
	Drive (const char* name, const dpb_t &dpb, const uint8_t *xlt=0)
	:_hndl(-1)
	,_filename(name)
	,_mode(O_RDWR)
	,_linear(true)
	,_dpb(dpb)
	,_xlt(new uint8_t [_dpb.spt])
	,_tracks(((RDSK_SecSize << dpb.bsh) * (dpb.dsm + 1) / RDSK_SecSize + (dpb.spt -1)) / dpb.spt + dpb.off)
	{
		for(int i = 0; i < _dpb.spt; i++)
			if(xlt)
			{
				// map back to linear
				assert(xlt[i]-1 < _dpb.spt);
				_xlt[xlt[i]-1] = i+1;
			}
			else
				_xlt[i] = i+1;
	}

	bool Mount()
	{
		if(-1 == _hndl)
			_hndl = open(_filename, _mode);
		clog << "Mount:" << *this << endl;
		return _hndl != -1;
	}

	void UnMount()
	{
		if(-1 != _hndl)
		{
			close(_hndl);
			_hndl = -1;
		}
	}

	bool Read(uint16_t track, uint8_t sect, uint8_t* dma)
	{
		assert(sect-1 < _dpb.spt);
		assert(track < _tracks);
		assert(dma);
		//assert(track < _tracks);
		assert(-1 != _hndl);

		off_t off = (track * _dpb.spt + (_linear? sect: _xlt[sect-1])-1) * RDSK_SecSize;
		clog << "Read trk " << track << ", sec " << (unsigned)sect << endl;
		return lseek(_hndl, off, SEEK_SET) == off &&
				read(_hndl, dma, RDSK_SecSize) == RDSK_SecSize;
	}

	bool Write(uint16_t track, uint8_t sect, uint8_t* dma)
	{
		assert(sect-1 < _dpb.spt);
		assert(track < _tracks);
		assert(dma);
		//assert(track < _tracks);
		assert(-1 != _hndl);

		off_t off = (track * _dpb.spt + (_linear? sect: _xlt[sect-1])-1) * RDSK_SecSize;
		clog << "Write trk " << track << ", sec " << (unsigned)sect << ", dma " << (void*)dma << endl;

		off_t offs = lseek(_hndl, off, SEEK_SET);

		return offs == off &&
				write(_hndl, dma, RDSK_SecSize) == RDSK_SecSize;
	}

	friend ostream& operator << ( ostream& os, const Drive& drive)
	{
		os << "F:" << drive._filename << ", mode " << drive._mode << ", tracks " << drive._tracks
		   << ", bpb: spt " << drive._dpb.spt
		   << ", bsh " << (unsigned)drive._dpb.bsh
		   << ", blm " << (unsigned)drive._dpb.blm
		   << ", exm " << (unsigned)drive._dpb.exm
		   << ", dsm " << drive._dpb.dsm
		   << ", drm " << drive._dpb.drm
		   << ", al0 " << (unsigned)drive._dpb.al0
		   << ", al1 " << (unsigned)drive._dpb.al1
		   << ", chs " << drive._dpb.cks
		   << ", off " << drive._dpb.off << ')';

		for(int i = 0; i < drive._dpb.spt; i++)
		{
			if(!(i%16))
				os << endl;
			else
				os << ',';
			os << setw(4) << (unsigned)drive. _xlt[i];
		}
		os << endl;
		return os;
	}
protected:
	size_t offset(uint16_t trk, uint16_t sct)
	{
		uint16_t abssct = (_linear || trk < _dpb.off ? sct : _xlt[sct-1]) -1;
		size_t res = (trk * _dpb.spt + abssct) * RDSK_SecSize;
		return res;
	}

private:
	int 		_hndl;
	const char*	_filename;
	int			_mode;
	bool		_linear;
	dpb_t		_dpb;
	uint8_t*	_xlt;
	unsigned	_tracks;
};

static Drive *Session[10] = {0,0,0,0,0,0,0,0,0,0};

int main(int argc, char *argv[]) {
	int sockfd; /* socket */
	int portno; /* port to listen on */
	socklen_t clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	int optval; /* flag value for setsockopt */
	int res; /* message byte size */
	pkt_t pkt;

	portno = RDSK_SvrPort;

	/*
	 * socket: create the parent socket
	 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		cerr << "ERROR opening socket " << strerror(errno) << endl;
		exit(errno);
	}
	/* setsockopt: Handy debugging trick that lets
	 * us rerun the server immediately after we kill it;
	 * otherwise we have to wait about 20 secs.
	 * Eliminates "ERROR on binding: Address already in use" error.
	 */
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int));

	/*
	 * build the server's Internet address
	 */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short) portno);

	/*
	 * bind: associate the parent socket with a port
	 */
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
	{
		cerr << "ERROR on binding" << strerror(errno) << endl;
		exit(errno);
	}

	/*
	 * main loop: wait for a datagram, then echo it
	 */

	while (1)
	{
		clientlen = sizeof(clientaddr);
		/*
		 * recvfrom: receive a UDP datagram from a client
		 */
		bzero(&pkt, sizeof(pkt));
		res = recvfrom(sockfd, &pkt, sizeof(pkt), 0,
				(struct sockaddr *) &clientaddr, &clientlen);
		if (res < 0)
		{
			cerr << "ERROR in recvfrom " << strerror(errno) << endl;
			continue;
		}

		if (res == 0)
		{
			cerr << "DISCONNECT " << strerror(errno) << endl;
			continue;
		}

		if(res != pkt.hdr.RDSK_Length)
		{
			cerr << "ERROR in recvfrom length " << strerror(errno) << endl;
			continue;
		}

		clog << "CMD["<< setw(4) << pkt.hdr.RDSK_Sequenz << "] => "
			 << pkt.hdr.RDSK_Code << ", ID="
			 << (unsigned)pkt.hdr.RDSK_Drive << ": " << "Length=" << pkt.hdr.RDSK_Length << ": ";

		if(RDSK_CmdMount == pkt.hdr.RDSK_Code)
		{
			unsigned id = pkt.req_mount.hdr.RDSK_Drive;

			if(id < 10 && (Session[id] ||
			   (Session[id] = new Drive (dname[id], pkt.req_mount.RDSK_Dpb, &pkt.req_mount.RDSK_Map))) &&
				Session[id]->Mount())
			{
				pkt.req_mount.hdr.RDSK_Code = RDSK_RespOk;
				pkt.req_mount.hdr.RDSK_Length = sizeof(rsp_mount_t);
				sequenz = pkt.req_mount.hdr.RDSK_Sequenz;
			}
			else
			{
				rsp_error_t &err = pkt.rsp_error;
				err.RDSK_errno = errno;
				strcpy(err.RDSK_Msg,"Can't mount disk.");
				err.hdr.RDSK_Length = sizeof(rsp_error_t);
			}
		}
		else
		{
			Drive *drive = Session[pkt.hdr.RDSK_Drive];
			if(drive && pkt.hdr.RDSK_Sequenz == sequenz)
			{
				switch (pkt.hdr.RDSK_Code)
				{
					case RDSK_CmdUnmount:
						drive->UnMount();
						pkt.hdr.RDSK_Code = RDSK_RespOk;
						pkt.hdr.RDSK_Length = sizeof(rsp_unmount_t);
						break;

					case RDSK_CmdRead:
						if(drive->Read( pkt.req_read.RDSK_Track, pkt.req_read.RDSK_Sec,pkt.rsp_read.RDSK_Data))
						{
							pkt.hdr.RDSK_Code = RDSK_RespOk;
							pkt.hdr.RDSK_Length = sizeof(rsp_read_t);
						}
						else
						{
							rsp_error_t &err = pkt.rsp_error;
							err.RDSK_errno = errno;
							strcpy(err.RDSK_Msg,"Can't read disk.");
							err.hdr.RDSK_Length = sizeof(rsp_error_t);
						}
						break;

					case RDSK_CmdWrite:
						if(drive->Write( pkt.req_write.RDSK_Track, pkt.req_write.RDSK_Sec,pkt.req_write.RDSK_Data))
						{
							pkt.hdr.RDSK_Code = RDSK_RespOk;
							pkt.hdr.RDSK_Length = sizeof(rsp_write_t);
						}
						else
						{
							rsp_error_t &err = pkt.rsp_error;
							err.RDSK_errno = errno;
							strcpy(err.RDSK_Msg,"Can't write disk.");
							err.hdr.RDSK_Length = sizeof(rsp_error_t);
						}
						break;

					case RDSK_RespOk:
					default:
						cerr << "ERROR unknown request " << strerror(errno) << endl;
						break;
				}
			}
			else
			{
				rsp_error_t &err = pkt.rsp_error;
				err.RDSK_errno = errno;
				strcpy(err.RDSK_Msg,"Out of Sequence.");
				err.hdr.RDSK_Length = sizeof(rsp_error_t);
			}
		}

		/*
		 * sendto: echo the input back to the client
		 */
		pkt.hdr.RDSK_Sequenz = ++sequenz;
		res = sendto(sockfd, &pkt, pkt.hdr.RDSK_Length, 0,
				(struct sockaddr *) &clientaddr, clientlen);
		if (pkt.hdr.RDSK_Length != res)
			cerr << "ERROR in sendto " << strerror(errno) << endl;
	}
	return 0;
}
