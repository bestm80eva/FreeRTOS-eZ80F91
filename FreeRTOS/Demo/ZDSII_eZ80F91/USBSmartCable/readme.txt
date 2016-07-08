Hi

I'm using the ZDSII_eZ80Acclaim!_5.2.1 and the USBSmartCable to connect the developer environment to the EZ80F910300ZCOG Developer Kit (Target). 
See https://github.com/notwendig/FreeRTOS-eZ80F91 for my current project.

This works under kvm-virtual WindowsXP on Linux host more or less stable.

To get able using ZiLOG's equipment and Software on 'wine' (the windows emulator on linux) I wrote a small gateway to map EthernetSmartCable request to/from USBSmartCable-Devices. The reason is that 'wine' doesn't support access on Host-USB devices. But 'wine' is able to use IP connections to the host.

For that I connect the USBSmartCable-device to the host's USB-port and run the Geteway-Server. 
Than I start ZiLOG's IDE under 'wine' and configure the EtherntSmartCable for debug device. (host-ip:4040)

This works under wine/linux more or less stable except the following :)

The Acclaim uses the eZ80F91algo.ini to generate the proper nexus-messages and send them to the debugger-device.

The line of interest is the first statement of the Init sequence.

The original sequence is:

[INIT_TARGET]

;Set ZDI Clock Frequency
Algo0=RED=%02XRFB=%XRFA=00
…
…

This results in two different messages, depending on which device (USB or EthernetSmartCable) I have configured. The message send from the EthernerSmartCable DLL does not work on the USBSmartCable Device.

Be using Wireshark on the USBSmartCable (nxEz80UsbSC.dll) you will see the following message for initialization.

RED=01RFB=1RFA=00RF0=0RF2=03RF3=40RF4=03RF5=80RFC=03RFD=20R11=80V00=0{A:_100V00=R03\(V00!=FF)}V04=80R10=V04V00=R03V01=!(V00>>7).[]=(V00!=FF)?((V00&20)?2:V01):3$

And on the EthernetSmartCable (nxEz80EthernetSC.dll) 

RED=05RFB=1RFA=00RF0=0RF2=03RF3=40RF4=03RF5=80RFC=03RFD=20R11=80V00=0{A:_100V00=R03\(V00!=FF)}V04=80R10=V04V00=R03V01=!(V00>>7).[]=(V00!=FF)?((V00&20)?2:V01):3$

which results in 
USB Answer "00000001" + Disconnect on the USB-Device. But it works on the Eth-Device

On the Acclaim you will get
[ERROR] Invalid Parameter
[ERROR] Unable to connect to the target.

I can Hot-fix this situation by rewriting the INIT_TARGET string.

Algo0=RED=%02RED=01XRFB=%XRFA=00

Now I can connect, downloading and debugging on 'wine'.

Register ED is described as
(RED= Target type(01:eZ80, 02:Encore...))

I attaches my small geteway server project (eclipse-project) as also my eZ80F91algo.ini file for testing purpose. You can use it on a linux PC. The USBDevice as also port 4040 must be accessible by the user which runs the delegation server and 'wine'.

You can grant access to the USBSmartCable device by the followup rule file
/etc/udev/rules.d/60-zilogsmartcable.rules

# Bus 005 Device 002: ID 04e3:0001 Zilog, Inc. 

SUBSYSTEM=="usb", ATTRS{idVendor}=="04e3", ATTRS{idProduct}=="0001", MODE="0666"
SUBSYSTEM=="usb_device", ATTRS{idVendor}=="04e3", ATTRS{idProduct}=="0001", MODE="0666"

For port access see your firewall – rules.

Have fun no frust.
Jürgen

