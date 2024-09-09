# dd-stream
It is essentially like using dd over the internet. This utility streams a raw image from a URL and writes it directly to a disk via its device path (or symlink).

### You can use either python version:
`python3 dd-stream.py if=<url> of=<device>`

Real Example:
`python3 dd-stream.py if=http://192.168.122.1:8000/tinyLinux_usb.raw of=/dev/vdb`

### Or you can use compile and use c version which uses curl: 

`gcc -o dd-stream dd-stream.c -lcurl`

`./dd-stream <url> <device>`

Real Example:
`./dd-stream http://192.168.122.1:8000/tinyLinux_usb.raw /dev/vdb`


### OTHER WAYS: 

### Over Internet: 
`wget http://example.com/file.raw -O - | dd of=/dev/sdX bs=16M`

### Over ssh:
```
inputFile="/dev/sdX"  # this is on local 
outputFile="/dev/sdY" # this is on remote 
host="user@destination"
dd if="${inputFile}" | gzip -6 - | pv | ssh "${host}"  "gunzip -c - | dd of="${outputFile}"
```
