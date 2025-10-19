#!/usr/bin/env python3
"""
Network Configuration Helper
Helps beginners set up the attack correctly
"""

import socket
import subprocess
import platform
import sys

def get_local_ip():
    """Get local IP address"""
    try:
        # Create a socket to get local IP
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except Exception:
        return "Unable to detect"

def check_port_available(port):
    """Check if port is available"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(1)
        result = sock.connect_ex(('localhost', port))
        sock.close()
        return result != 0  # True if available
    except Exception:
        return False

def ping_host(ip):
    """Ping a host to check connectivity"""
    param = '-n' if platform.system().lower() == 'windows' else '-c'
    command = ['ping', param, '1', ip]
    try:
        output = subprocess.run(command, capture_output=True, timeout=5)
        return output.returncode == 0
    except Exception:
        return False

def print_banner():
    """Print helper banner"""
    print("""
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║     ESP32 MITM Attack - Network Configuration Helper     ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
    """)

def main():
    """Main helper function"""
    print_banner()
    
    print("🔍 Checking your network configuration...\n")
    
    # Get local IP
    local_ip = get_local_ip()
    print(f"✓ Your Computer's IP: {local_ip}")
    
    # Check required ports
    print("\n📡 Checking required ports:")
    
    port_8000 = check_port_available(8000)
    port_8080 = check_port_available(8080)
    
    if port_8000:
        print("  ✓ Port 8000 (Firmware Server): Available")
    else:
        print("  ✗ Port 8000 (Firmware Server): In use or blocked")
    
    if port_8080:
        print("  ✓ Port 8080 (MITM Proxy): Available")
    else:
        print("  ✗ Port 8080 (MITM Proxy): In use or blocked")
    
    # Get ESP32 IP
    print("\n📱 ESP32 Configuration:")
    esp32_ip = input("  Enter ESP32's IP address (or press Enter to skip): ").strip()
    
    if esp32_ip:
        print(f"\n  Testing connection to ESP32 ({esp32_ip})...")
        if ping_host(esp32_ip):
            print(f"  ✓ ESP32 is reachable at {esp32_ip}")
        else:
            print(f"  ✗ Cannot reach ESP32 at {esp32_ip}")
            print("    Make sure ESP32 is powered on and connected to WiFi")
    
    # Generate configuration
    print("\n" + "="*60)
    print("📋 CONFIGURATION SUMMARY")
    print("="*60)
    print(f"\n1. Your Computer's IP: {local_ip}")
    print(f"2. Firmware Server: http://{local_ip}:8000")
    print(f"3. MITM Proxy: http://{local_ip}:8080")
    
    if esp32_ip:
        print(f"4. ESP32 IP: {esp32_ip}")
    
    print("\n📝 Arduino Code Configuration:")
    print("="*60)
    print(f'const char* OTA_FIRMWARE_URL = "http://{local_ip}:8080/firmware_encrypted.bin";')
    print("="*60)
    
    print("\n✅ Next Steps:")
    print("1. Copy the OTA_FIRMWARE_URL line above")
    print("2. Paste it into your Arduino sketch")
    print("3. Start firmware server: python simple_http_server.py")
    print("4. Start MITM proxy: python real_mitm_attack.py")
    print("5. Upload sketch to ESP32 and watch the attack!")
    
    print("\n📖 For detailed instructions, see: BEGINNER_ATTACK_GUIDE.md\n")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nHelper stopped.")
    except Exception as e:
        print(f"\nError: {e}")
