#!/usr/bin/env python3
"""
Simple HTTP server for hosting encrypted firmware files
For testing OTA updates locally
"""

import http.server
import socketserver
import os
import sys

PORT = 8000

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Add CORS headers for cross-origin requests
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()
    
    def do_GET(self):
        print(f"[+] Serving: {self.path}")
        return super().do_GET()

def main():
    # Change to build directory if it exists
    if os.path.exists('build'):
        os.chdir('build')
        print(f"[+] Serving files from: {os.getcwd()}")
    
    Handler = MyHTTPRequestHandler
    
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"[+] HTTP Server running on port {PORT}")
        print(f"[+] Access firmware at: http://localhost:{PORT}/firmware_encrypted.bin")
        print(f"[+] Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[+] Server stopped")
            sys.exit(0)

if __name__ == '__main__':
    main()
