QT              =   core                \
                    gui                 \

TEMPLATE        =   app
#DESTDIR         =   "../bin"
TARGET          =   DCResistance

include         (QextSerialPort.pri)
include         (DCResistance.pri)
include         (CWirelessChannel.pri)

SOURCES         +=

HEADERS         +=


RESOURCES       =
