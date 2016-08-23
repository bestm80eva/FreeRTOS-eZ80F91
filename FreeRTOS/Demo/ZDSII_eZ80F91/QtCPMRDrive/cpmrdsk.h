#ifndef CPMRDSK_H
#define CPMRDSK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"  {
#endif

#pragma pack(push,1)

#define RDSK_PORT	4050

typedef enum {
    // used for file, status is the user number
    USER0,USER1,USER2,USER3,USER4,USER5,USER6,USER7,
    USER8,USER9,USER01,USER11,USER12,USER13,USER14,USER15,
    // status is the user number (P2DOS) or used for password extent (CP/M 3 or higher)
    USER16_31,
    LABEL = 32, // disc label
    TIMEST,     // 33: time stamp (P2DOS)
    UNUSED = 0xE5   // unused/deleted
  } dirstat_t;

typedef struct {
    uint8_t status;
    char    name[8];
    char    next[3];
    uint8_t Xl,Bc,Xh,Rc;
    union {
        uint8_t  b[16];
        uint16_t w[ 8];
    }Al;
} dir_t;

// Disk Parameter Header (the DPH)
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
} dpb_t;


typedef enum {
    // Requests
    RDSK_MountRequest       = 0x01,
    RDSK_UnmountRequest     = 0x02,
    RDSK_ReadRequest        = 0x03,
    RDSK_WriteRequest       = 0x04,

    // Response flag
    RDSK_Response           = 0x10,

    // Responses
    RDSK_MountResponse      = RDSK_Response | RDSK_MountRequest,
    RDSK_UnmountResponse    = RDSK_Response | RDSK_UnmountRequest,
    RDSK_ReadResponse       = RDSK_Response | RDSK_ReadRequest,
    RDSK_WriteResponse      = RDSK_Response | RDSK_WriteRequest,

    // Error indicator
    RDSK_ErrorFlag          = 0x80
} pdutype_t;

typedef enum {
    READONLY    = 0x01,
    LINEAR      = 0x02
} iomode_t;

typedef struct {
    uint16_t    pdusz;  // total size of pdu
    uint8_t     cmdid;  // command / status
    uint8_t     devid;  // device id 0=A. 1=B ...
    uint16_t    seqnz;  // sequenz
} hdr_t;

typedef struct {
    hdr_t hdr;
    uint8_t  diskid[12];    // Disk identifiecation 8.3
    uint8_t  mode;          // != 0 then writeable
    uint16_t secsz;         // Sector size
    dpb_t    dpb;           // Disk parameter block
    uint8_t  xlt;        	// Start of Sector translation table if not 0
} mountreq_t;

typedef struct {
    hdr_t hdr;
    uint16_t    track;
    uint16_t    sect;
    uint8_t     data;
} ioreq_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // CPMRDSK_H
