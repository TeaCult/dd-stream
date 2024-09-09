import requests
import sys
import os

# Get arguments from command line
if len(sys.argv) != 3:
    print("Usage: script.py if=<url> of=<device name>")
    sys.exit(1)

# Parse command-line arguments
url = None
device = None
for arg in sys.argv[1:]:
    if arg.startswith("if="):
        url = arg[3:]
    elif arg.startswith("of="):
        device = arg[3:]

if not url or not device:
    print("Error: both 'if' (input URL) and 'of' (output device) must be specified.")
    sys.exit(1)

chunk_size = 1024 * 1024  # 1 MB chunks
header_size = 0  # Zero header size to skip

# Function to print progress
def print_progress(downloaded, total_size):
    progress = downloaded / total_size * 100
    print(f"\rProgress: {progress:.2f}% ({downloaded}/{total_size} bytes)", end='')

# Open the target device (e.g., /dev/sda or /dev/vdb)
with open(device, 'wb') as target_device:
    # Stream the file from the web
    with requests.get(url, stream=True) as r:
        r.raise_for_status()  # Check for HTTP errors
        
        total_size = int(r.headers.get('content-length', 0))  # Get content length
        total_skipped = 0
        downloaded = 0

        for chunk in r.iter_content(chunk_size=chunk_size):
            if chunk:  # Process non-empty chunks
                downloaded += len(chunk)
                
                # Skip the header part if necessary
                if total_skipped < header_size:
                    remaining_header = header_size - total_skipped
                    # If the chunk contains more than the header, write the rest
                    if len(chunk) > remaining_header:
                        target_device.write(chunk[remaining_header:])
                    total_skipped += len(chunk)
                else:
                    # Write the chunk directly to the target device
                    target_device.write(chunk)

                # Show progress
                if total_size > 0:
                    print_progress(downloaded, total_size)

print("\nDownload and write completed.")
