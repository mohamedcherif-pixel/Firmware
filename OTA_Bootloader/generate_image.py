import urllib.request
from PIL import Image
import io
import os

def generate_header():
    image_path = "conor.jpg"
    print(f"Processing local image: {image_path}...")

    try:
        img = Image.open(image_path)
        img = img.resize((240, 240))
        img = img.convert("RGB")
    except Exception as e:
        print(f"Failed to process image: {e}")
        return
    
    pixels = list(img.getdata())
    
    print("Converting to RGB565...")
    
    with open("myImage4.h", "w") as f:
        f.write("#ifndef MY_IMAGE_4_H\n")
        f.write("#define MY_IMAGE_4_H\n\n")
        f.write("#include <pgmspace.h>\n\n")
        # Using uint16_t for RGB565
        f.write("const uint16_t myImage4[57600] PROGMEM = {\n")
        
        for i, (r, g, b) in enumerate(pixels):
            # RGB565 conversion
            # 5 bits Red, 6 bits Green, 5 bits Blue
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            
            f.write(f"0x{rgb565:04X}")
            if i < len(pixels) - 1:
                f.write(", ")
            if (i + 1) % 16 == 0:
                f.write("\n")
                
        f.write("};\n\n")
        f.write("#endif\n")
        
    print("myImage.h generated successfully!")

if __name__ == "__main__":
    generate_header()
