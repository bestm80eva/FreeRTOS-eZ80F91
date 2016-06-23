/*******************************************************************************
** File:        basetypes.h
** Description: Declares custom datatypes in addition to the supplied defines.h.
**
** Copyright 2013 Zilog Inc. ALL RIGHTS RESERVED.
*
********************************************************************************
* 
* The source code in this file was written by an authorized Zilog employee or a
* licensed consultant. The source code has been verified to the fullest extent 
* possible.
*
* Permission to use this code is granted on a royalty-free basis. However, 
* users are cautioned to authenticate the code contained herein.
*
* ZILOG DOES NOT GUARANTEE THE VERACITY OF THIS SOFTWARE; ANY SOFTWARE 
* CONTAINED HEREIN IS PROVIDED "AS IS." NO WARRANTIES ARE GIVEN, WHETHER 
* EXPRESS, IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES OF FITNESS FOR 
* PARTICULAR PURPOSE OR MERCHANTABILITY. IN NO EVENT WILL ZILOG BE LIABLE FOR 
* ANY SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES OR ANY LIABILITY IN TORT,
* NEGLIGENCE, OR OTHER LIABILITY INCURRED AS A RESULT OF THE USE OF THE 
* SOFTWARE, EVEN IF ZILOG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* ZILOG ALSO DOES NOT WARRANT THAT THE USE OF THE SOFTWARE, OR OF ANY 
* INFORMATION CONTAINED THEREIN WILL NOT INFRINGE ANY PATENT, COPYRIGHT, OR    
* TRADEMARK OF ANY THIRD PERSON OR ENTITY.

* THE SOFTWARE IS NOT FAULT-TOLERANT AND IS NOT DESIGNED, MANUFACTURED OR 
* INTENDED FOR USE IN CONJUNCTION WITH ON-LINE CONTROL EQUIPMENT, IN HAZARDOUS 
* ENVIRONMENTS, IN APPLICATIONS REQUIRING FAIL-SAFE PERFORMANCE, OR WHERE THE 
* FAILURE OF THE SOFTWARE COULD LEAD DIRECTLY TO DEATH, PERSONAL INJURY OR 
* SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE (ALL OF THE FOREGOING, "HIGH RISK 
* ACTIVITIES"). ZILOG SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTY TO
* HIGH RISK ACTIVITIES.
*
*******************************************************************************/
#ifndef __BASETYPES_H__
#define __BASETYPES_H__

#include <defines.h>        // use data types in ZDSII std defines

//#define TRUE            (1)
//#define FALSE           (0)

// for eZ80F91 register access
typedef volatile BYTE __INTIO *IORegInt8;
typedef volatile BYTE __EXTIO *IORegExt8;
typedef volatile WORD __INTIO *IORegInt16;
typedef volatile WORD __EXTIO *IORegExt16;

// Generir Error
#define SYSERR            (-1)

#endif    //__BASETYPES_H__

// End of file
