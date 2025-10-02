# UK101ESP32 - Emulador de Ohio Scientific Superboard II para ESP32

## 📖 Descripción

**OSIESP32** es un emulador completo del **Emulador de Ohio Scientific Superboard II** original, portado para ejecutarse en un **ESP32**.

El emulador incluye:
- ✅ CPU 6502 completo con timing preciso
- ✅ Monitor OSI
- ✅ Microsoft BASIC
- ✅ Interfaz serie a 115200 baud
- ✅ Compatibilidad con minicom y otros terminales

Más información en Minibots: [https://minibots.wordpress.com/2025/10/02/emulador-de-osi-superboard-ii-con-esp32]

## 🏗️ Arquitectura

### Componentes Principales

1. **CPU 6502** (`cpu6502.cpp/h`)
   - Emulación ciclo-precisa del microprocesador 6502
   - Ejecuta a 1.000 MHz (velocidad original del OSI)

2. **MC6850 ACIA** (`mc6821.cpp/h`)
   - Interfaz de E/S
   - Gestiona comunicación teclado/pantalla

3. **Motherboard** (`motherboard.cpp/h`)
   - Mapeo de memoria (32KB RAM + 32KB ROM)
   - Decodificación de direcciones

4. **Terminal** (`terminal.cpp/h`)
   - Entrada serie desde ESP32
   - Salida formateada para terminales modernos

5. **ROM Embebida** (`uk101.rom.h`)
   - Monitor OSI + BASIC
   - Datos embebidos en memoria flash

## 🚀 Instalación y Uso

### Requisitos

- **Hardware**: ESP32 (cualquier modelo)
- **Software**:
  - Arduino IDE con soporte ESP32
  - Terminal serie (minicom, screen, PuTTY, etc.)
- **Conexión**: USB a serie (FTDI, CP2102, etc.)

### Compilación

1. **Instalar Arduino IDE** con soporte ESP32:
   ```bash
   # En Linux, instalar desde repositorio o descargar
   sudo apt install arduino
   ```

2. **Agregar board ESP32**:
   - En Arduino IDE: Archivo → Preferencias
   - URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Herramientas → Placa → ESP32 Arduino → ESP32 Dev Module

3. **Compilar y subir**:
   ```bash
   cd airesp32
   arduino-cli compile --fqbn esp32:esp32:esp32 airesp32.ino
   arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 osiresp32.ino
   ```

### Configuración del Terminal

#### Usando minicom:
```bash
# Instalar minicom
sudo apt install minicom

# Conectar al ESP32
minicom -D /dev/ttyUSB0 -b 115200

# Configuración (Ctrl+A → O):
# Serial port setup → 115200 8N1
# Hardware flow control → No
# Software flow control → No
```

#### Usando screen:
```bash
screen /dev/ttyUSB0 115200
```

## 📄 Licencia

Este proyecto está bajo **GNU Lesser General Public License v3.0**.

Basado en AIRE/UK101RE.

## 🙏 Agradecimientos

- **Miguel A. Vallejo**: Creador del emulador AIRE/UK101RE original
- **Comunidad ESP32**: Por hacer posible este port
