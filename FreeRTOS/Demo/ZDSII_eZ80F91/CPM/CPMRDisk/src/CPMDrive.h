/*
 * CPMDrive.h
 *
 *  Created on: 21.07.2016
 *      Author: juergen
 */

#ifndef CPMDRIVE_H_
#define CPMDRIVE_H_
#include <interface.h>


class CPMDrive {
public:
	CPMDrive(const char* path, const dpb_t &dpb, const uint8_t *xlt);
	virtual ~CPMDrive();
private:
	const string _path;
	const dpb_t _dpb;
	const uint8_t *_xlt;
};

#endif /* CPMDRIVE_H_ */
