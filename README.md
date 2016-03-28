# ImageDownloader
When [Conversations](https://conversations.im) uploads an encrypted image with [XEP-0363: HTTP Upload](http://xmpp.org/extensions/xep-0363.html) it appends the encryption key as an anchor to the URL.

The anchor is made of 96 characters which represent 48 bytes in HEX. The first 16 bytes are the IV the last 32 bytes are the key.

The encryption mode is ```aes-gcm```. The authentcation tag of 16 bytes is appended to the file.

This Java project provides sample code on how to download and dercypt a file. Output is to stdout. Redirect to file or pipe to image viewer.

Usage: ```java -jar ImageDownloader.jar http://host.tld/path/to/file.jpg#theivandkey```