# OneBox
A toolbox software provides serial port, TCP server/client, UDP server/client functions based on Qt4.


V1.0 2021-Oct-03
1. Add signals serverChanged() and connectionChanged() in class UDPServer to notice UI widget update connection status
2. Add slots void updateServerInfo() and updateConnectionStatus() in class UdpServerWidget to show connection status from model


V1.0 2021-Oct-02
1. Add signals serverChanged() and connectionChanged() in class UDPClient to notice UI widget update connection status
2. Add slots void updateServerInfo() and updateConnectionStatus() in class UdpClientWidget to show connection status from model
3. Remove compile warnings in sign and unsigned comparison


V1.0 2021-Sep-17
1. Update to support Qt5 build


V1.0 2021-Jul-09
1. Initial release.
2. Tool support TCP/UDP server client, serial port.
