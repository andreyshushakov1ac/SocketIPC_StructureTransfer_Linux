Create an application consisting of two processes:
− the first process sends textual and structured information;
− the second process receives the information and displays it in the process window.
The structured information format is a package consisting of a header, a body, and the end of the package.
The header contains the following fields: the size of the transmitted data; the current time.
The body contains the following fields: basic information for transmission
Package Ending: CRC16

Information transfer mechanisms:
1) Socket
