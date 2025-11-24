#!/usr/bin/env python3
"""
Quick RSA key generator for Arduino IDE users
Run this once to generate rsa_public.h for your ESP32 project
"""

from Crypto.PublicKey import RSA

def generate_keys():
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
    
    # Create the header file in the ESP32_Encrypted_Firmware directory
    header_path = '../ESP32_Encrypted_Firmware/rsa_public.h'
    with open(header_path, 'w') as f:
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
    
    print("✓ Keys generated successfully!")
    print(f"  - Private key: rsa_private.pem")
    print(f"  - Public key: rsa_public.pem")
    print(f"  - ESP32 header: {header_path}")
    print("\nNow you can compile your ESP32 project!")

if __name__ == '__main__':
    generate_keys()
