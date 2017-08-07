# ImageDownloader
When [Conversations](https://conversations.im) uploads an encrypted image with [XEP-0363: HTTP Upload](http://xmpp.org/extensions/xep-0363.html) it appends the encryption key as an anchor to the URL.

There are now [2 formats for anchors](https://github.com/siacs/Conversations/issues/2578).  
*LEGACY FORMAT*:  The anchor is made of 96 characters which represent 48 bytes in HEX. The first 16 bytes are the IV the last 32 bytes are the key.  
*CURRENT FORMAT*: The anchor is made of 88 characters which represent 44 bytes in HEX. The first 12 bytes are the IV the last 32 bytes are the key.  

The encryption mode is ```aes-256-gcm```. The authentication tag of 16 bytes is appended to the file.

This C project provides sample code on how to decrypt and encrypt a file. Output is to stdout. Redirect to file or pipe to image viewer.

Compile with ```gcc aesgcm.c -lcrypto -o aesgcm```, openbrowser.sh also tries to compile it if the executable not already exist.

Set openbrowser.sh as your browser in gajim or another XMPP client to automatically decrypt http uploads encrypted with OMEMO or PGP with Conversations.

You can also encrypt data with aesgcm by sending in `enc` as the second parameter.
