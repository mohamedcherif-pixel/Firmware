#!/usr/bin/env python3
"""
RSA Digital Signature Tool for ESP32 Firmware
Signs firmware binaries and generates signature files
"""

import argparse
import hashlib
from Crypto.PublicKey import RSA
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256

def generate_key_pair():
    """Generate RSA key pair for signing and verification"""
    print("Generating RSA 2048-bit key pair...")
    
    # Generate private key
    private_key = RSA.generate(2048)
    
    # Extract public key
    public_key = private_key.publickey()
    
    # Save private key
    with open('rsa_private.pem', 'wb') as f:
        f.write(private_key.export_key())
    
    # Save public key
    with open('rsa_public.pem', 'wb') as f:
        f.write(public_key.export_key())
    
    # Save public key as C header for ESP32
    public_key_der = public_key.export_key(format='DER')
    with open('rsa_public.h', 'w') as f:
        f.write('#ifndef RSA_PUBLIC_H\n')
        f.write('#define RSA_PUBLIC_H\n\n')
        f.write('const unsigned char rsa_public_key[] = {\n')
        for i, byte in enumerate(public_key_der):
            if i % 16 == 0:
                f.write('    ')
            f.write(f'0x{byte:02x}')
            if i < len(public_key_der) - 1:
                f.write(', ')
            if i % 16 == 15:
                f.write('\n')
        if len(public_key_der) % 16 != 0:
            f.write('\n')
        f.write('};\n\n')
        f.write(f'const size_t rsa_public_key_len = {len(public_key_der)};\n\n')
        f.write('#endif // RSA_PUBLIC_H\n')
    
    print("✓ Keys generated:")
    print("  - Private key: rsa_private.pem")
    print("  - Public key: rsa_public.pem")
    print("  - Public key header: rsa_public.h")

def sign_firmware(input_file, output_file, private_key_file='rsa_private.pem'):
    """Sign firmware binary with RSA private key"""
    try:
        # Load private key
        with open(private_key_file, 'rb') as f:
            private_key = RSA.import_key(f.read())
        
        # Read firmware
        with open(input_file, 'rb') as f:
            firmware_data = f.read()
        
        # Calculate SHA-256 hash of firmware
        hash_obj = SHA256.new(firmware_data)
        
        # Sign the hash
        signature = pkcs1_15.new(private_key).sign(hash_obj)
        
        # Save signature
        with open(output_file, 'wb') as f:
            f.write(signature)
        
        print(f"✓ Firmware signed:")
        print(f"  - Input: {input_file}")
        print(f"  - Signature: {output_file}")
        print(f"  - Firmware size: {len(firmware_data)} bytes")
        print(f"  - Signature size: {len(signature)} bytes")
        
        return True
        
    except Exception as e:
        print(f"✗ Error signing firmware: {e}")
        return False

def verify_signature(firmware_file, signature_file, public_key_file='rsa_public.pem'):
    """Verify firmware signature with RSA public key"""
    try:
        # Load public key
        with open(public_key_file, 'rb') as f:
            public_key = RSA.import_key(f.read())
        
        # Read firmware
        with open(firmware_file, 'rb') as f:
            firmware_data = f.read()
        
        # Read signature
        with open(signature_file, 'rb') as f:
            signature = f.read()
        
        # Calculate SHA-256 hash of firmware
        hash_obj = SHA256.new(firmware_data)
        
        # Verify signature
        verifier = pkcs1_15.new(public_key)
        verifier.verify(hash_obj, signature)
        
        print(f"✓ Signature verified successfully")
        return True
        
    except Exception as e:
        print(f"✗ Signature verification failed: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='RSA Digital Signature Tool for ESP32 Firmware')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Generate keys command
    gen_parser = subparsers.add_parser('generate', help='Generate RSA key pair')
    
    # Sign command
    sign_parser = subparsers.add_parser('sign', help='Sign firmware binary')
    sign_parser.add_argument('-i', '--input', required=True, help='Input firmware file')
    sign_parser.add_argument('-o', '--output', required=True, help='Output signature file')
    sign_parser.add_argument('-k', '--key', default='rsa_private.pem', help='Private key file')
    
    # Verify command
    verify_parser = subparsers.add_parser('verify', help='Verify firmware signature')
    verify_parser.add_argument('-f', '--firmware', required=True, help='Firmware file')
    verify_parser.add_argument('-s', '--signature', required=True, help='Signature file')
    verify_parser.add_argument('-k', '--key', default='rsa_public.pem', help='Public key file')
    
    args = parser.parse_args()
    
    if args.command == 'generate':
        generate_key_pair()
    elif args.command == 'sign':
        sign_firmware(args.input, args.output, args.key)
    elif args.command == 'verify':
        verify_signature(args.firmware, args.signature, args.key)
    else:
        parser.print_help()

if __name__ == '__main__':
    main()
