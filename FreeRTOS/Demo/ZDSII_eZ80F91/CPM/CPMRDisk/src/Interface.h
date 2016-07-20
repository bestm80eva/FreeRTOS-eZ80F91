/*
 * Interface.h
 *
 *  Created on: 07.10.2015
 *      Author: juergen
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_


#include <stdint.h>

// This file implements a remote disk API for CPM.
// This code implements the client side of the API.
// At some point we may also implement the server API.
//
// Constants And Global variables
//
#define RDSK_SecSize 128 	 // 2048 Byte size of sector buffer
#define RDSK_EMsgLen 63	   	 // 63 Max length for an error message
#define RDSK_SvrPort 5020	 // Server UDP Port

// Disk Parameter Header (DPH)
#pragma pack(push,1)

typedef struct {
	uint16_t 	xlt,	scratch0,
				scratch1,scratch2,
				dirbuf,	dpb,
				cvs,	alv;
}dph_t;

// Disk Parameter Block (the DPB)
typedef struct {
	uint16_t	spt	;  // Number of 128-byte records per track
	uint8_t 	bsh	;  // Block shift. 3 => 1k, 4 => 2k, 5 => 4k....
	uint8_t 	blm	;  // Block mask. 7 => 1k, 0Fh => 2k, 1Fh => 4k...
	uint8_t 	exm	;  // Extent mask, see later
	uint16_t	dsm	;  // (no. of blocks on the disc)-1
	uint16_t	drm	;  // (no. of directory entries)-1
	uint8_t 	al0	;  // Directory allocation bitmap, first byte
	uint8_t 	al1	;  // Directory allocation bitmap, second byte
	uint16_t	cks	;  // Checksum vector size, 0 for a fixed disc
					   //    No. directory entries/4, rounded up.
	uint16_t	off	;  // Offset, number of reserved tracks
} dpb_t;  //

//#define RDSK_Ses		DL;	 // 0 Mounted disk session ident.
//#define RDSK_LastId		DW;	 // 0 Last local request ident sent to server.
//
// Remote disk command/responce codes.
typedef enum {
	RDSK_RespOk		 =  	0,	// last command completed Ok.
	RDSK_CmdMount	 =  	1,	// mount a remote disk.
	RDSK_CmdUnmount	 =  	2,	// unmount a mounted remote disk.
	RDSK_CmdRead	 =  	3,	// read a physical sector from a mounted remote disk.
	RDSK_CmdWrite	 =  	4	// write a physical sector to a mounted remote disk.
} cmd_t;

// All packets start with a code and a local tansaction identifier.
typedef struct {
	uint16_t	RDSK_Length;
	uint8_t  	RDSK_Drive;	 // Drive number 0=A, 1=B ...
	uint16_t	RDSK_Code;	 // Command/reponce code offset.
	uint16_t	RDSK_Sequenz;// Local transaction ident offset.
}hdr_t;

//
// Error responce packet offsets.
typedef struct {
	hdr_t	hdr;
	uint16_t 	RDSK_errno;
	char		RDSK_Msg[RDSK_EMsgLen];		// start of message
} rsp_error_t;

//
// Mount command packet offsets.
typedef struct {
	hdr_t	hdr;
	uint16_t  RDSK_Flg;	 	// Mount flags (currently only ReadOnly = 1,) 2 bytes.
	dpb_t	  RDSK_Dpb;		// disk parameter block
	uint8_t	  RDSK_Map;		// start of sector mapping table if any. 0 = none
} req_mount_t;

//
// Mount responce packet offsets.
typedef struct {
	hdr_t	hdr;
	uint16_t  RDSK_Flg;	 	// Mount flags (currently only ReadOnly = 1,) 2 bytes.
} rsp_mount_t;

//
// Unmount command packet offsets.
typedef struct {
	hdr_t	hdr;
} req_unmount_t;

//
// Unount responce packet offsets.
typedef struct {
	hdr_t	hdr;
} rsp_unmount_t;

//
// Nop command packet offsets
typedef struct {
	hdr_t	hdr;
} req_nop_t;

//
// Nop responce packet offsets.
typedef struct {
	hdr_t	hdr;
} rsp_nop_t;


//
// Read physical sector command packet offsets
typedef struct {
	hdr_t	hdr;
	uint16_t  RDSK_Track;	 // Track number to be read.
	uint16_t  RDSK_Sec;	 	//  Track logical sector number to be read.
}req_read_t;

//
// Read responce packet offsets.
typedef struct {
	hdr_t	hdr;
	uint8_t	  RDSK_Data[RDSK_SecSize];	 //  first data byte.
} rsp_read_t;

//
// Write physical sector command packet offsets
typedef struct {
	hdr_t	hdr;
	uint16_t  RDSK_Track;	// Track number to be read.
	uint16_t  RDSK_Sec;	 	// Track logical sector number to be read.
	uint8_t	  RDSK_Data[RDSK_SecSize];	// First sector data byte.
} req_write_t;

//
// Write responce packet offsets.
typedef struct {
	hdr_t	hdr;
} rsp_write_t;

typedef union {
	hdr_t			hdr;
	rsp_error_t		rsp_error;
	req_mount_t 	req_mount;
	rsp_mount_t 	rsp_mount;
	req_unmount_t 	req_unmount;
	rsp_unmount_t 	rsp_unmount;
	req_nop_t 		req_nop;
	rsp_nop_t 		rsp_nop;
	req_read_t 		req_read;
	rsp_read_t 		rsp_read;
	req_write_t 	req_write;
	rsp_write_t 	res_write;
	uint8_t			space[sizeof(req_mount_t) + 128];
}pkt_t;

typedef enum {
	rsp_errorID,
	req_mountID,
	rsp_mountID,
	req_unmountID,
	rsp_unmountID,
	req_nopID,
	rsp_nopID,
	req_readID,
	rsp_readID,
	req_writeID,
	res_writeID
} typeid_t;

#pragma pack(pop)


#endif /* INTERFACE_H_ */
