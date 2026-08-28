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