LSB Image Steganography (C Project)
This project implements Least Significant Bit (LSB) steganography using C. It hides secret text data inside a BMP image without visibly altering the image. It is a command-line utility that allows encoding (hiding) and decoding (extracting) secret messages within 24-bit BMP images.

Project Structure
Stegnography/
├── Code/
│ ├── main.c
│ ├── encode.c / encode.h
│ ├── decode.c / decode.h
│ ├── common.h
│ ├── types.h
│ ├── output.txt (Decoded output)
│ ├── secret.txt (Input message to encode)
│ ├── beautiful.bmp (Original image)
│ ├── stego.bmp (Output image after encoding)
│ └── a.out (Compiled binary)
├── References/
│ ├── LSB Stegnography.docx
│ ├── Screenshots, Diagrams, Hex files, etc.

Features
Embed text into a 24-bit BMP image using LSB encoding.

Supports a magic string for stego-image verification.

Decode and extract hidden message from BMP images.

Basic CLI interface (no GUI).

Validates input image capacity before embedding.

Requirements
C compiler (GCC recommended)

Command line environment (Linux terminal / Git Bash / Windows cmd)

BMP image (24-bit only)

Concepts Used
File I/O and pointer arithmetic in C

Bitwise operations

Structure manipulation

BMP image header parsing

LSB manipulation

