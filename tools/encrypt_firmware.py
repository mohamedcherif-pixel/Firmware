#!/usr/bin/env python3
"""
ESP32 Firmware Encryption Tool
Encrypts firmware binary using AES-256-CBC for secure OTA updates
"""

import os
import sys
import argparse
import hashlib
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
from Crypto.Util.Padding import pad

def encrypt_firmware(input_file, output_file, key_file=None):
    """
    Encrypt firmware binary with AES-256-CBC
    
    Args:
        input_file: Path to input firmware binary
        output_file: Path to output encrypted firmware
        key_file: Optional path to key file (if None, generates new key)
    """
    
    # Generate or load AES key
    if key_file and os.path.exists(key_file):
        with open(key_file, 'rb') as f:
            key = f.read(32)
        print(f"[+] Loaded AES key from {key_file}")
    else:
        key = get_random_bytes(32)  # AES-256
        key_output = key_file if key_file else 'aes_key.bin'
        with open(key_output, 'wb') as f:
            f.write(key)
        print(f"[+] Generated new AES-256 key: {key_output}")
        print(f"[!] Keep this key secure - it's needed for decryption!")
    
    # Generate random IV
    iv = get_random_bytes(16)
    
    # Read firmware binary
    with open(input_file, 'rb') as f:
        firmware_data = f.read()
    
    print(f"[+] Read firmware: {len(firmware_data)} bytes")
    
    # Calculate hash of original firmware
    firmware_hash = hashlib.sha256(firmware_data).digest()
    print(f"[+] Firmware SHA-256: {firmware_hash.hex()}")
    
    # Encrypt firmware
    cipher = AES.new(key, AES.MODE_CBC, iv)
    padded_firmware = pad(firmware_data, AES.block_size)
    encrypted_firmware = cipher.encrypt(padded_firmware)
    
    print(f"[+] Encrypted firmware: {len(encrypted_firmware)} bytes")
    
    # Write encrypted firmware (IV + encrypted data)
    with open(output_file, 'wb') as f:
        f.write(iv)  # First 16 bytes are IV
        f.write(encrypted_firmware)
    
    print(f"[+] Encrypted firmware saved to: {output_file}")
    
    # Calculate hash of encrypted firmware
    encrypted_hash = hashlib.sha256(iv + encrypted_firmware).digest()
    print(f"[+] Encrypted SHA-256: {encrypted_hash.hex()}")
    
    # Save metadata
    metadata_file = output_file + '.meta'
    with open(metadata_file, 'w') as f:
        f.write(f"Original Size: {len(firmware_data)}\n")
        f.write(f"Encrypted Size: {len(encrypted_firmware)}\n")
        f.write(f"Original Hash: {firmware_hash.hex()}\n")
        f.write(f"Encrypted Hash: {encrypted_hash.hex()}\n")
        f.write(f"IV: {iv.hex()}\n")
        f.write(f"Key File: {key_file if key_file else 'aes_key.bin'}\n")
    
    print(f"[+] Metadata saved to: {metadata_file}")
    
    # Generate C header with key (for development only!)
    header_file = 'encryption_key.h'
    with open(header_file, 'w') as f:
        f.write("// WARNING: This file contains encryption keys!\n")
        f.write("// DO NOT commit to version control!\n")
        f.write("// For development/testing only!\n\n")
        f.write("#ifndef ENCRYPTION_KEY_H\n")
        f.write("#define ENCRYPTION_KEY_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("// AES-256 Key (32 bytes)\n")
        f.write("static const uint8_t AES_KEY[32] = {\n    ")
        f.write(", ".join([f"0x{b:02x}" for b in key]))
        f.write("\n};\n\n")
        f.write("// IV (16 bytes)\n")
        f.write("static const uint8_t AES_IV[16] = {\n    ")
        f.write(", ".join([f"0x{b:02x}" for b in iv]))
        f.write("\n};\n\n")
        f.write("#endif // ENCRYPTION_KEY_H\n")
    
    print(f"[+] C header generated: {header_file}")
    print(f"\n[✓] Encryption complete!")
    print(f"\n[!] IMPORTANT: Store the key securely!")
    print(f"[!] In production, use secure key storage (HSM, secure element, etc.)")

def decrypt_firmware(input_file, output_file, key_file):
    """
    Decrypt firmware binary (for verification)
    
    Args:
        input_file: Path to encrypted firmware
        output_file: Path to output decrypted firmware
        key_file: Path to key file
    """
    
    # Load key
    with open(key_file, 'rb') as f:
        key = f.read(32)
    
    # Read encrypted firmware
    with open(input_file, 'rb') as f:
        iv = f.read(16)
        encrypted_data = f.read()
    
    print(f"[+] Read encrypted firmware: {len(encrypted_data)} bytes")
    
    # Decrypt
    cipher = AES.new(key, AES.MODE_CBC, iv)
    decrypted_data = cipher.decrypt(encrypted_data)
    
    # Remove padding
    padding_len = decrypted_data[-1]
    firmware_data = decrypted_data[:-padding_len]
    
    # Write decrypted firmware
    with open(output_file, 'wb') as f:
        f.write(firmware_data)
    
    print(f"[+] Decrypted firmware saved to: {output_file}")
    print(f"[+] Decrypted size: {len(firmware_data)} bytes")
    print(f"[✓] Decryption complete!")

def main():
    parser = argparse.ArgumentParser(description='ESP32 Firmware Encryption Tool')
    parser.add_argument('mode', choices=['encrypt', 'decrypt'], help='Operation mode')
    parser.add_argument('-i', '--input', required=True, help='Input firmware file')
    parser.add_argument('-o', '--output', required=True, help='Output file')
    parser.add_argument('-k', '--key', help='AES key file (optional for encrypt, required for decrypt)')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"[!] Error: Input file not found: {args.input}")
        sys.exit(1)
    
    if args.mode == 'encrypt':
        encrypt_firmware(args.input, args.output, args.key)
    elif args.mode == 'decrypt':
        if not args.key:
            print("[!] Error: Key file required for decryption")
            sys.exit(1)
        decrypt_firmware(args.input, args.output, args.key)

if __name__ == '__main__':
    main()
