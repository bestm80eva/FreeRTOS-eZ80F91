/*
 * smartcable.h
 *
 *  Created on: Jun 24, 2016
 *      Author: juergen
 */

#ifndef SMARTCABLE_H_
#define SMARTCABLE_H_

#include <iostream>

/*
 * ZiLOG's USBSmartCable
 * Vendor Id: 04e3
 * Product Id: 0001
 */
#define VENDORID  0x04e3
#define BRODUCTID 0x0001

extern int verbos_flg;

#include <iostream>
#include <sstream>

class VerbStream: public std::ostream
{
    class VerbStreamBuf: public std::stringbuf
    {
        std::ostream&   output;
        public:
            VerbStreamBuf(std::ostream& str)
                :output(str)
            {}

        virtual int sync ( )
        {
            output << setw(6) << setfill(' ') << right << clock() << ':' << str();
            str("");
            output.flush();
            return 0;
        }
    };

    VerbStreamBuf buffer;
    static bool _verbose_flag;

    public:
    VerbStream(std::ostream& str)
            :std::ostream(&buffer)
            ,buffer(str)
        {
        }
    bool set(bool to) { bool old = _verbose_flag; _verbose_flag = to; return old;}
    bool stat(void) const { return _verbose_flag;}
};

extern VerbStream verb;

#define DEFLISTEN 	"localhost"
#define DEFPORT		"4040"




#endif /* SMARTCABLE_H_ */
