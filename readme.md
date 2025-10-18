[![](https://dcbadge.vercel.app/api/server/3E8ca2dkcC)](https://discord.gg/osmu)

![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/bitaxeorg/esp-miner/total)
![GitHub commit activity](https://img.shields.io/github/commit-activity/t/bitaxeorg/esp-miner)
![GitHub contributors](https://img.shields.io/github/contributors/bitaxeorg/esp-miner)
![Alt](https://repobeats.axiom.co/api/embed/70889479b1e002c18a184b05bc5cbf2ed3718579.svg "Repobeats analytics image")

# ESP-Miner
esp-miner is open source ESP32 firmware for the [Bitaxe](https://github.com/bitaxeorg/bitaxe)

If you are looking for premade images to load on your Bitaxe, check out the [latest release](https://github.com/bitaxeorg/ESP-Miner/releases/latest) page. Maybe you want [instructions](https://github.com/bitaxeorg/ESP-Miner/blob/master/flashing.md) for loading factory images.

# Bitaxetool
We also have a command line python tool for flashing Bitaxe and updating the config called Bitaxetool 

**Bitaxetool Requires Python3.4 or later and pip**

Install bitaxetool from pip. pip is included with Python 3.4 but if you need to install it check <https://pip.pypa.io/en/stable/installation/>

```
pip install --upgrade bitaxetool
```
The bitaxetool includes all necessary library for flashing the binaries to the Bitaxe Hardware.

**Notes**
 - The bitaxetool does not work properly with esptool v5.x.x, esptool v4.9.0 or earlier is required.
 - Bitaxetool v0.6.1 - locked to using esptool v4.9.0

```
pip install bitaxetool==0.6.1
```

- Flash a "factory" image to a Bitaxe to reset to factory settings. Make sure to choose an image built for your hardware version (401) in this case:

```
bitaxetool --firmware ./esp-miner-factory-401-v2.4.2.bin
```
- Flash just the NVS config to a bitaxe:

```
bitaxetool --config ./config-401.cvs
```
- Flash both a factory image _and_ a config to your Bitaxe: note the settings in the config file will overwrite the config already baked into the factory image:

```
bitaxetool --config ./config-401.cvs --firmware ./esp-miner-factory-401-v2.4.2.bin
```

## AxeOS API
The esp-miner UI is called AxeOS and provides an API to expose actions and information.

For more details take a look at [`main/http_server/openapi.yaml`](./main/http_server/openapi.yaml).

Available API endpoints:
  
**GET**

* `/api/system/info` Get system information
* `/api/system/asic` Get ASIC settings information
* `/api/system/statistics` Get system statistics (data logging should be activated)
* `/api/system/statistics/dashboard` Get system statistics for dashboard
* `/api/system/wifi/scan` Scan for available Wi-Fi networks

**POST**

* `/api/system/restart` Restart the system
* `/api/system/OTA` Update system firmware
* `/api/system/OTAWWW` Update AxeOS

**PATCH**

* `/api/system` Update system settings

### API examples in `curl`:

```bash
# Get system information
curl http://YOUR-BITAXE-IP/api/system/info

# Get ASIC settings information
curl http://YOUR-BITAXE-IP/api/system/asic

# Get system statistics
curl http://YOUR-BITAXE-IP/api/system/statistics

# Get dashboard statistics
curl http://YOUR-BITAXE-IP/api/system/statistics/dashboard

# Get available Wi-Fi networks
curl http://YOUR-BITAXE-IP/api/system/wifi/scan


# Restart the system
curl -X POST http://YOUR-BITAXE-IP/api/system/restart

# Update system firmware
curl -X POST \
     -H "Content-Type: application/octet-stream" \
     --data-binary "@esp-miner.bin" \
     http://YOUR-BITAXE-IP/api/system/OTA

# Update AxeOS
curl -X POST \
     -H "Content-Type: application/octet-stream" \
     --data-binary "@www.bin" \
     http://YOUR-BITAXE-IP/api/system/OTAWWW


# Update system settings
curl -X PATCH http://YOUR-BITAXE-IP/api/system \
     -H "Content-Type: application/json" \
     -d '{"fanspeed": "desired_speed_value"}'
```

## Administration

The firmware hosts a small web server on port 80 for administrative purposes. Once the Bitaxe device is connected to the local network, the admin web front end may be accessed via a web browser connected to the same network at `http://<IP>`, replacing `IP` with the LAN IP address of the Bitaxe device, or `http://bitaxe`, provided your network supports mDNS configuration.

### Recovery

In the event that the admin web front end is inaccessible, for example because of an unsuccessful firmware update (`www.bin`), a recovery page can be accessed at `http://<IP>/recovery`.

### Unlock Settings

In order to unlock the Input fields for ASIC Frequency and ASIC Core Voltage you need to append `?oc` to the end of the settings tab URL in your browser. Be aware that without additional cooling overclocking can overheat and/or damage your Bitaxe.

## Development

### Prerequisites

- Install the ESP-IDF toolchain from https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/
- Install nodejs/npm from https://nodejs.org/en/download
- (Optional) Install the ESP-IDF extension for VSCode from https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension

### Building

At the root of the repository, run:
```
idf.py build && ./merge_bin.sh ./esp-miner-merged.bin
```

Note: the merge_bin.sh script is a custom script that merges the bootloader, partition table, and the application binary into a single file.

Note: if using VSCode, you may have to configure the settings.json file to match your esp hardware version. For example, if your bitaxe has something other than an esp32-s3, you will need to change the version in the `.vscode/settings.json` file.

### Flashing

With the bitaxe connected to your computer via USB, run:

```
bitaxetool --config ./config-xxx.cvs --firmware ./esp-miner-merged.bin
```

where xxx is the config file for your hardware version. You can see the list of available config files in the root of the repository.

A custom board version is also possible with `config-custom.cvs`. A custom board needs to be based on an existing `devicemodel` and `asicmodel`.

**Notes:** 
  - If you are developing within a dev container, you will need to run the bitaxetool command from outside the container. Otherwise, you will get an error about the device not being found.
  - Some Bitaxe versions can't directly connect to a USB-C port. If yours is affected use a USB-A adapter as a workaround. More about it [here](https://github.com/bitaxeorg/bitaxeGamma/issues/37).
  - Only ESP32-S3-WROOM-1 module type N16R8 (16MB Flash, 8MB Octal SPI PSRAM) is supported. This model number should be visible on the ESP32 module. Other module types without PSRAM or with Quad SPI PSRAM will not work with the normal firmware. More about it [here](https://github.com/bitaxeorg/ESP-Miner/issues/826).

### Wi-Fi routers

There are some Wi-Fi routers that will block mining, ASUS Wi-Fi routers & some TP-Link Wi-Fi routers for example.
If you find that your not able to mine / have no hash rate you will need to check the Wi-Fi routers settings and disable the following;

1/ AiProtection

2/ IoT 

If your Wi-Fi router has both of these options you might have to disable them both.

If your still having problems here, check other settings within the Wi-Fi router and the bitaxe device, this includes the URL for
the Stratum Host and Stratum Port.

## ASIC Interface Points and Starting Nonce Control

This section documents the ASIC interface points in the ESP-Miner firmware and discusses how to programmatically provide a starting nonce for the ASIC.

### Overview

ESP-Miner supports four ASIC models, each with its own driver implementation:
- **BM1397** - Located in `components/asic/bm1397.c`
- **BM1366** - Located in `components/asic/bm1366.c`
- **BM1368** - Located in `components/asic/bm1368.c`
- **BM1370** - Located in `components/asic/bm1370.c`

All ASIC models share a common interface abstraction layer defined in `components/asic/asic.c`.

### Key Interface Points

#### 1. **Job Packet Structure**

Each ASIC model has its own job packet structure that includes a `starting_nonce` field:

**BM1397** (`components/asic/include/bm1397.h`):
```c
typedef struct __attribute__((__packed__))
{
    uint8_t job_id;
    uint8_t num_midstates;
    uint8_t starting_nonce[4];    // Starting nonce for the ASIC
    uint8_t nbits[4];
    uint8_t ntime[4];
    uint8_t merkle4[4];
    uint8_t midstate[32];
    uint8_t midstate1[32];
    uint8_t midstate2[32];
    uint8_t midstate3[32];
} job_packet;
```

**BM1366, BM1368, BM1370** (similar structure):
```c
typedef struct __attribute__((__packed__))
{
    uint8_t job_id;
    uint8_t num_midstates;
    uint8_t starting_nonce[4];    // Starting nonce for the ASIC
    uint8_t nbits[4];
    uint8_t ntime[4];
    uint8_t merkle_root[32];
    uint8_t prev_block_hash[32];
    uint8_t version[4];
} BM1366_job;  // Similar for BM1368_job and BM1370_job
```

#### 2. **Job Construction Flow**

The starting nonce flows through the following components:

1. **Mining Job Structure** (`components/stratum/include/mining.h`):
   ```c
   typedef struct {
       uint32_t version;
       uint32_t version_mask;
       uint8_t prev_block_hash[32];
       uint8_t prev_block_hash_be[32];
       uint8_t merkle_root[32];
       uint8_t merkle_root_be[32];
       uint32_t ntime;
       uint32_t target;
       uint32_t starting_nonce;     // Starting nonce value
       // ... other fields
   } bm_job;
   ```

2. **Job Construction** (`components/stratum/mining.c:62`):
   ```c
   bm_job construct_bm_job(mining_notify *params, const char *merkle_root, 
                           const uint32_t version_mask, const uint32_t difficulty)
   {
       bm_job new_job;
       // ... initialization code
       new_job.starting_nonce = 0;  // Currently hardcoded to 0
       // ... rest of job construction
   }
   ```

3. **Job Generation** (`main/tasks/create_jobs_task.c`):
   - The `generate_work()` function creates jobs by calling `construct_bm_job()`
   - Jobs are enqueued to `GLOBAL_STATE->ASIC_jobs_queue`

4. **Job Transmission** (e.g., `components/asic/bm1366.c:266`):
   ```c
   void BM1366_send_work(void * pvParameters, bm_job * next_bm_job)
   {
       BM1366_job job;
       // ... initialization
       memcpy(&job.starting_nonce, &next_bm_job->starting_nonce, 4);
       // ... rest of job packet construction
       _send_BM1366((TYPE_JOB | GROUP_SINGLE | CMD_WRITE), (uint8_t *)&job, 
                    sizeof(BM1366_job), BM1366_DEBUG_WORK);
   }
   ```

#### 3. **Serial Communication**

All ASIC communication occurs via UART (serial) interface:
- **Interface**: `components/asic/serial.c`
- **Functions**: 
  - `SERIAL_init()` - Initialize UART1 (pins TX:17, RX:18)
  - `SERIAL_send()` - Send data to ASIC
  - `SERIAL_rx()` - Receive data from ASIC
  - `SERIAL_set_baud()` - Configure baud rate

The job packets are sent to the ASIC through the serial interface after being wrapped with protocol headers (preamble, length, CRC).

#### 4. **ASIC Task Flow**

The main ASIC task (`main/tasks/asic_task.c`) coordinates work:
1. Dequeues jobs from `ASIC_jobs_queue`
2. Calls `ASIC_send_work()` to transmit job to ASIC
3. Calls `ASIC_process_work()` to receive results

### Current Starting Nonce Implementation

**Current Behavior**: The starting nonce is **hardcoded to 0** in `construct_bm_job()` function (`components/stratum/mining.c:62`).

```c
new_job.starting_nonce = 0;
```

This means every job sent to the ASIC starts searching from nonce value 0.

### How to Programmatically Set Starting Nonce

To programmatically provide a starting nonce for the ASIC, you have several options:

#### Option 1: Modify Job Construction (Recommended)

Modify the `construct_bm_job()` function in `components/stratum/mining.c`:

```c
bm_job construct_bm_job(mining_notify *params, const char *merkle_root, 
                        const uint32_t version_mask, const uint32_t difficulty)
{
    bm_job new_job;
    // ... existing code ...
    
    // Instead of hardcoded 0, calculate or provide starting nonce
    new_job.starting_nonce = calculate_starting_nonce();  // Custom function
    
    // ... rest of function ...
}
```

#### Option 2: Add API Endpoint

Add a new API endpoint to set starting nonce programmatically:

1. Add to `main/http_server/http_server.c`:
   ```c
   POST /api/system/starting_nonce
   Body: {"starting_nonce": 12345678}
   ```

2. Store the value in `GlobalState` structure
3. Use it in `construct_bm_job()`

#### Option 3: Extend BAP Protocol

Add a new parameter to the BAP protocol (`main/bap/`) for setting starting nonce:

```
Host:   $BAP,SET,starting_nonce,12345678*XX\r\n
Device: $BAP,ACK,starting_nonce,12345678*YY\r\n
```

This would allow external devices connected via UART to control the starting nonce.

#### Option 4: Use Extranonce2 for Work Segmentation

The existing implementation already uses `extranonce_2` to generate different work units. This is incremented for each generated job (`main/tasks/create_jobs_task.c:57`), effectively distributing the nonce space across multiple work units without directly modifying the starting nonce.

### Interface Summary

| Interface Point | Location | Purpose |
|----------------|----------|---------|
| `ASIC_init()` | `components/asic/asic.c` | Initialize ASIC with frequency and difficulty |
| `ASIC_send_work()` | `components/asic/asic.c` | Send job packet to ASIC |
| `ASIC_process_work()` | `components/asic/asic.c` | Receive and process ASIC results |
| `ASIC_set_frequency()` | `components/asic/asic.c` | Set ASIC operating frequency |
| `ASIC_set_version_mask()` | `components/asic/asic.c` | Configure version rolling mask |
| `ASIC_read_registers()` | `components/asic/asic.c` | Read ASIC internal registers |
| `construct_bm_job()` | `components/stratum/mining.c` | Create job with starting_nonce |
| `SERIAL_send()` | `components/asic/serial.c` | Low-level serial transmission |
| `SERIAL_rx()` | `components/asic/serial.c` | Low-level serial reception |

### Register Access

Some ASICs support register access for configuration:
- BM1366: Register read/write via `_send_BM1366()` with `TYPE_CMD` header
- BM1370: Additional `BM1370_read_registers()` function
- Registers control frequency, difficulty masks, and chip addresses

### Recommendations

For implementing programmatic starting nonce control:

1. **Start with Option 1** (Modify `construct_bm_job()`): This is the cleanest approach and affects all ASIC models uniformly.

2. **Add configuration storage**: Store the starting nonce strategy (fixed, random, incremental) in NVS config.

3. **Consider nonce space partitioning**: If running multiple devices, partition the 32-bit nonce space (0 to 4,294,967,296) across devices to avoid duplicate work.

4. **Test thoroughly**: Different starting nonces may affect ASIC performance and result reporting.

5. **Document in API**: If exposing via HTTP API, document the endpoint in `main/http_server/openapi.yaml`.

### Example Implementation

Here's a simple example of adding starting nonce control:

```c
// In components/stratum/mining.c
static uint32_t custom_starting_nonce = 0;

void set_starting_nonce(uint32_t nonce) {
    custom_starting_nonce = nonce;
}

bm_job construct_bm_job(mining_notify *params, const char *merkle_root,
                        const uint32_t version_mask, const uint32_t difficulty)
{
    bm_job new_job;
    // ... existing initialization ...
    
    new_job.starting_nonce = custom_starting_nonce;
    
    // ... rest of function ...
}
```

This allows external code to call `set_starting_nonce()` before job construction to control the starting nonce value sent to the ASIC.

## Attributions

The display font is Portfolio 6x8 from https://int10h.org/oldschool-pc-fonts/ by VileR.
