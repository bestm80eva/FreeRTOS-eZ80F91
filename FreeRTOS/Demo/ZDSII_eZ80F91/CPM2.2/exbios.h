#ifndef _EXBIOS_H_
#define _EXBIOS_H_

#if defined(MIXEDMODE) 

typedef struct {
	uint8_t  mbaseL;
	uint8_t  mbase;
	uint8_t  mbaseU;
	uint24_t hl_;
	uint24_t de_;
	uint24_t bc_;
	uint24_t af_;
	uint24_t iy;
	uint24_t ix;
	uint24_t hl;
	uint24_t de;
	uint24_t bc;
	uint24_t af;
	uint8_t	 trapflg;
	uint24_t trapret;
	} trapargs_t;
	
#if defined(CPM22)
	
//	CONIO,PRT,AUX IOPorts
typedef enum {
	CONSTA = 0x01,	// console status port
	CONDAT,			// console data port
	PRTSTA,			// printer status port
	PRTDAT,			// printer data port
	AUXDAT			// auxiliary data port
} port_CONIO_t;

typedef enum {
	FDCD   = 0x01,	// fdc-port: # of drive
	FDCT,			// fdc-port: # of track
	FDCS,			// fdc-port: # of sector
	FDCOP,			// fdc-port: command
	FDCST			// fdc-port: status
} port_FDIO_t;

typedef enum {
	MONITOR = 0x30,
	CONIO,
	FDIO,
	DMAIO,
	ROMBOOT,
	MAXIO = 0x37	
} xebioscall_t;

void prvTCPCpmIOTask( void *ram );
void exbioscall(trapargs_t* arg);

#endif 	// CPM22
#endif	// MIXEDMODE	
#endif  // _EXBIOS_H_
