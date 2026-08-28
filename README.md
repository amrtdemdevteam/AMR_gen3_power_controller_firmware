# How to fix when upload fails with error [upload] Broken pipe
## Direct Terminal Upload
```bash
# Ensure dfu-util is installed
sudo apt install dfu-util -y

# Put board in DFU mode (double-click RST), then check list
dfu-util -l
```
If dfu-util -l shows Found DFU: [2341:0366], you can manually flash the compiled binary:

```bash
# Navigate to the .pio/build/giga_r1_m4/ directory of your project
cd <path_to_your_project>/.pio/build/giga_r1_m4/

# Flash the compiled binary
dfu-util -d 2341:0366 -a 0 -s 0x08040000:leave -D firmware.bin
```

After using direct upload, you can return to PlatformIO and use the standard upload command.

# How to set up virtual environment for PlatformIO
```bash
# Create a virtual environment
python3 -m venv .venv

# Activate the virtual environment
source .venv/bin/activate

# Upgrade pip to the latest version
pip install --upgrade pip

# Install PlatformIO within the virtual environment
pip install platformio

# Verify PlatformIO installation
platformio --version
```

# How to build and upload the firmware using PlatformIO:

## Build the project
```bash
# Navigate to the project root directory
cd <path_to_your_project>

# Build the firmware
platformio run -d .

# Upload the firmware
platformio run -d . --target upload

```

## Build & Upload example/fsm_state_machine
```bash
# Build the firmware
platformio run -d examples/fsm_state_machine

# Upload the firmware
platformio run -d examples/fsm_state_machine -t upload
```

