# PicoFTP
A small multi user Passive Mode FTP server

## Building
Although it is a Netbeans project it's usage is not required for building the program.
Running `make build` in the `PicoFTP/PicoFTP/` directory ought to be everything you need to do.
## Usage
This software should __not__ be used in a production/public environment. It currently accepts __any__ credentials supplied with full read/write permission of the user it's running under.
#### parameters
```
-a <ip address> (mandatory)

It is used to supply the client with an IP address to connect to when the client invokes the PASV command. 
Some clients choose to ignore this and use the same IP they connected to by default.

-p <port> (default 21)

Control port. Client and server exchange commands and responses using this port.

-d <path> (default .)

The root folder of the FTP server.
```
#### Features

```
- Full Passive Mode FTP Implementation supporting "unlimited" concurrent users
- Filezilla supported directory listing.
- Adding, removing and renaming directories.
- Removing and renaming files.
- Uploading and downloading files
```

The full list of commands can be found in `ftp.c` and `lookup.c`
