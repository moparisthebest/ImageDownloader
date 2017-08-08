#!/bin/bash

# try different size files to encrypt/decrypt
#dd if=/dev/urandom bs=1M count=100 of=/dev/shm/randombytes

# compile C and rust code this way
#gcc aesgcm.c -lcrypto -O3 -o aesgcm
#cargo build --release

export key=e910dd93e4bbf608957fcb2549e41b7b9d453b2b019bffe5726b1669bca9f40b5fdc0917cd84426df78f8236

test () {
    bin=$1
    #tee >(md5sum - 1>&2) - < /dev/shm/randombytes | $bin $key enc | $bin $key | md5sum -
    $bin $key enc < /dev/shm/randombytes | $bin $key &>/dev/null
}

time test ./aesgcm
time test ./target/release/image_downloader
