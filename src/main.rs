// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

extern crate crypto;
extern crate rustc_serialize;

use std::env;
use std::io::{self, Write, Read, stdin};

use crypto::aes;

use rustc_serialize::hex::*;

use crypto::aes_gcm::*;
use crypto::aead::{AeadEncryptor,AeadDecryptor};

// Encrypt a buffer with the given key and iv using
// AES-256/CBC/Pkcs encryption.
fn encrypt(data: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {

    // Create an encryptor instance of the best performing
    // type available for the platform.
    let mut encryptor = AesGcm::new(
        aes::KeySize::KeySize256,
        key,
        iv,
        &[]
    );

    let mut out: Vec<u8> = vec![0; data.len()];
    let mut out_tag: Vec<u8> = vec![0; 16];;

    encryptor.encrypt(&*data, &mut *out,&mut *out_tag);

    out.extend_from_slice(&out_tag);
    return out
}

// Decrypts a buffer with the given key and iv using
// AES-256/CBC/Pkcs encryption.
//
// This function is very similar to encrypt(), so, please reference
// comments in that function. In non-example code, if desired, it is possible to
// share much of the implementation using closures to hide the operation
// being performed. However, such code would make this example less clear.
fn decrypt(encrypted_data: &[u8], key: &[u8], iv: &[u8]) -> Result<Vec<u8>, &'static str> {
    // Create an decryptor instance of the best performing
    // type available for the platform.

    let mut decryptor = AesGcm::new(
        aes::KeySize::KeySize256,
        key,
        iv,
        &[]
    );

    let out_len = encrypted_data.len() - 16;
    let mut out: Vec<u8> = vec![0; out_len];;

    let result = decryptor.decrypt(&encrypted_data[0..out_len], &mut *out, &encrypted_data[out_len..]);
    if result {
        Ok(out)
    } else {
        Err("decryption failed")
    }
}

/*

extern crate ring;
use ring::aead::*;

fn encrypt_ring(data: &[u8], key: &[u8], iv: &[u8]) -> Vec<u8> {

    let sealing_key = SealingKey::new(&AES_256_GCM, key).unwrap();
    let additional_data: [u8; 0] = [];

    let mut out: Vec<u8> = repeat(0).take(data.len() + 16).collect();

    let output_size = seal_in_place(&sealing_key,
                                    &iv,
                                    &additional_data,
                                    &mut out,
                                    16)
        .unwrap();

    return out
}

*/

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut enc= false;

    if args.len() < 2 || args.len() > 3 {
        panic!("Usage: {} <key> [enc]", args[0]);
    } else if args.len() == 3 {
        match &*args[2] {
            "enc" => { enc = true; }
            _ => panic!("Usage: {} <key> [enc]", args[0])
        }
    }

    //let message = "boba";
    //println!("message: '{}' hex: '{}'", message, message.as_bytes().to_hex());
    //let key_iv = "e910dd93e4bbf608957fcb25e08dd8e049e41b7b9d453b2b019bffe5726b1669bca9f40b5fdc0917cd84426df78f8236";
    //let key_iv = "e910dd93e4bbf608957fcb2549e41b7b9d453b2b019bffe5726b1669bca9f40b5fdc0917cd84426df78f8236";
    let key_iv = &args[1];

    let mut key_iv = key_iv.from_hex().expect("Invalid hex for key");

    //let iv_len = if key_iv.len() == 48 { 16 } else { 12 };
    let iv_len = match key_iv.len() {
        // todo: rust-crypto does not support non-12-byte nonce/iv 48 => 16,
        44 => 12,
        _ => panic!("only key lengths of 44 bytes are supported")
    };
    let (iv, key) = key_iv.split_at_mut(iv_len);
    //let (iv, key) = key_iv.split_at_mut(if key_iv.len() == 48 { 16 } else { 12 }); // borrow checker explodes

    //println!("key: '{:?}' iv: '{:?}'", key, iv);
    //println!("key: '{}' iv: '{}'", key.to_hex(), iv.to_hex());

    let mut data : Vec<u8> = Vec::new();
    stdin().read_to_end(& mut data).expect("error reading from stdin");

    let data : Vec<u8> = if enc {
        encrypt(&data, &key, &iv)
    } else {
        decrypt(&data, &key, &iv).expect("error during decryption")
    };
    io::stdout().write(&data).expect("error writing to stdout");

    /*
    let message = "boba";
    let data = message.as_bytes();
    let encrypted_data = encrypt(&data, &key, &iv);
    println!("message: '{}' encrypted: '{}'", message, encrypted_data.to_hex());
    let decrypted_data = decrypt(&*encrypted_data, &key, &iv).expect("error during decryption");

    // aesgcm bd92a7b359f5d9a359c6bbda242b1f4cb2f4c8d9
    // rust   bd92a7b359f5d9a359c6bbda242b1f4cb2f4c8d9
    // ring   dffdc5d2460bc181e2a34d139e72b32c0985018a

    assert!(data == &*decrypted_data);

    println!("decrypted: {:?}", String::from_utf8(decrypted_data).unwrap());
    */
}
