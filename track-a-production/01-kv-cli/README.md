# kv-cli

A UART-driven command-line interface with an in-memory key-value
store, built on ESP-IDF v6.1 for the ESP32-S3.

---

## What It Does

On boot, the firmware starts a CLI task that listens on UART0. A user
connects with any serial terminal and sends text commands. The
commands operate on a fixed-size in-memory key-value store.

```
kv> set wifi.ssid MyNetwork
OK
kv> get wifi.ssid
MyNetwork
kv> list
wifi.ssid            = MyNetwork
(1 entries)
kv> del wifi.ssid
OK
kv> save
OK
kv> list
(0 entries)
```

The store persists to NVS on demand via the `save` command and
reloads automatically at boot.

---

## Commands

| Command | Usage | Description |
|---|---|---|
| `set`   | `set <key> <value>` | Insert or update a key |
| `get`   | `get <key>`         | Print the value for a key |
| `del`   | `del <key>`         | Remove a key |
| `list`  | `list`              | Print all key-value pairs |
| `clear` | `clear`             | Remove all pairs |
| `save`  | `save`              | Persist current state to NVS |
| `help`  | `help`              | List available commands |

Errors are reported inline:

```
kv> get missing
error: ESP_ERR_NOT_FOUND
kv> set
usage: set <key> <value>
```

---

## Architecture

```
main/
 ├─ app_main            wires everything together
 └─ cmd_kv              command handlers (set/get/del/list/clear/help)

components/
 ├─ cli                 UART input, line editing, tokenizer, dispatcher
 └─ kv_store            thread-safe in-memory key-value store
```

### `cli`

Handles all input and command dispatch. Installs the UART0 driver,
routes stdio through it, spawns a FreeRTOS task that reads bytes
one at a time, echoes them, handles backspace, assembles a line,
tokenizes it, and dispatches to a registered handler.

Public API: `cli_init`, `cli_start`, `cli_register_command`.

### `kv_store`

An in-memory key-value store with a fixed number of slots, no heap
allocation, and a FreeRTOS mutex around every operation. Supports
set, get, delete, clear, count, and callback-based iteration.

Public API: `kv_store_init`, `kv_store_set`, `kv_store_get`,
`kv_store_delete`, `kv_store_clear`, `kv_store_iterate`,
`kv_store_count`.

### `cmd_kv`

The bridge between the two. Each command is a small function matching
`cli_cmd_fn_t` that validates `argc`, calls the store, prints a result,
and returns an `esp_err_t`.

---

## Building

Requires ESP-IDF v6.1 with the ESP32-S3 toolchain.

```bash
cd track-a-production/01-kv-cli
idf.py set-target esp32s3
idf.py build
```

To flash and monitor:

```bash
idf.py -p <PORT> flash monitor
```

Replace `<PORT>` with your serial device (`COM3` on Windows,
`/dev/ttyUSB0` on Linux, `/dev/cu.usbserial-*` on macOS).

To run in QEMU (no hardware required):

```bash
idf.py qemu monitor
```

---

## Configuration

All tunables are configurable at build time via Kconfig. Run:

```bash
idf.py menuconfig
```

Options appear under **Component Configuration**:

### CLI Configuration

| Option | Default | Description |
|---|---|---|
| `CLI_PROMPT` | `kv> ` | Prompt string |
| `CLI_MAX_LINE_LEN` | 128 | Max input line length (bytes) |
| `CLI_MAX_ARGS` | 8 | Max tokens per command |
| `CLI_MAX_COMMANDS` | 16 | Command table capacity |
| `CLI_TASK_STACK_SIZE` | 4096 | CLI task stack (bytes) |
| `CLI_TASK_PRIORITY` | 5 | FreeRTOS task priority |
| `CLI_UART_NUM` | 0 | UART peripheral |
| `CLI_UART_BAUD` | 115200 | Baud rate |
| `KV_NVS_PARTITION` | `nvs_user` | NVS partition name |
| `KV_NVS_NAMESPACE` | `kv_store` | NVS namespace within the partition |

### KV Store Configuration

| Option | Default | Description |
|---|---|---|
| `KV_MAX_KEYS` | 16 | Number of store slots |
| `KV_MAX_KEY_LEN` | 32 | Max key length incl. NUL |
| `KV_MAX_VALUE_LEN` | 64 | Max value length incl. NUL |

Committed defaults live in `sdkconfig.defaults`. To regenerate
`sdkconfig` from those defaults:

```bash
rm sdkconfig
idf.py reconfigure
```
---

## Partition Table

Custom layout in `partitions.csv`:

| Name       | Type | SubType | Offset   | Size      |
|------------|------|---------|----------|-----------|
| `nvs`      | data | nvs     | 0x9000   | 0x6000    |
| `nvs_user` | data | nvs     | 0xf000   | 0x4000    |
| `phy_init` | data | phy     | 0x13000  | 0x1000    |
| `factory`  | app  | factory | 0x20000  | 0x100000  |

`nvs_user` is dedicated to the KV store, isolated from the
system `nvs` used by WiFi and PHY.


## Known Limitations

- **No persistence.** Values are lost on reboot. Addressed in
  Project A2 (`kv-persistent`) with NVS.
- **Single UART.** The CLI runs on UART0 only. Adding support for
  multiple transports (USB CDC, telnet) would require abstracting
  the transport from the CLI task.
- **No command history.** Up-arrow recall is not implemented. Would
  require a small ring buffer in the CLI task.
- **No tab completion.** Commands must be typed in full.
- **Not thread-safe for registration.** `cli_register_command` is
  intended to be called during setup, before `cli_start`. Registering
  commands after the task is running is technically possible but not
  recommended.
- **QEMU input verification pending.** Build verified for `esp32s3`;
  interactive behavior on real hardware not yet tested.
