# Secure LSB Steganography in C

This project hides encrypted text messages inside BMP images using **Least Significant Bit (LSB) steganography** and **XOR-based password encryption**.

## 💡 Features
- Hide messages inside `.bmp` images
- Password-protected XOR encryption
- Pure C implementation (no external libraries)
- Runs in Linux (WSL + GCC)

## 🛠️ How to Use
### 1. Compile
```bash
gcc encode.c test_encode.c -o stego
### 2. Encode
```bash
./stego -e awesome.bmp secret.txt stego_img.bmp
### 2. Decode
./stego -d stego_img.bmp extracted.txt
