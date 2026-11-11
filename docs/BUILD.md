
# NANA-PS3 — BUILD SYSTEM

## Objetivo

Compilar el proyecto para PlayStation 3 utilizando un entorno PS3DEV y PSL1GHT.

## Herramientas

- PS3DEV: toolchain de desarrollo.
- PSL1GHT: SDK homebrew.
- PPU GCC: compilador para el procesador principal.
- GNU Make: sistema de compilación.
- GitHub Actions: automatización de compilaciones.

## Entorno de compilación

La compilación se realizará en un entorno Linux remoto.

No se requiere instalar el SDK directamente en Android.

## Resultados esperados

- ELF: ejecutable de desarrollo.
- SELF: ejecutable preparado mediante las herramientas del SDK.
- PKG: paquete instalable, cuando el proyecto tenga configurados sus metadatos.

## Estado

Pendiente de configurar y probar el sistema de compilación.

## Importante

Una compilación exitosa no garantiza que el programa funcione en todas las consolas PS3. Se necesitan pruebas reales.
