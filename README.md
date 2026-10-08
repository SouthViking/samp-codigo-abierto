<div align="center">
  <a href="https://sarp.es/" target="_blank">
    <img style="width: 200px;" src="https://i.imgur.com/EFK15nk.png" alt="San Andreas Roleplay"/>
  </a>
</div>

![Discord Shield](https://discord.com/api/guilds/1118327104908570654/widget.png?style=shield)
[![License: MIT](https://img.shields.io/badge/License-MIT%20License-yellow.svg)](https://opensource.org/licenses/MIT) 

# Modo de juego de código abierto de SARP.es
Este repositorio representa la base de desarrollo de código abierto para la creación y mejora de scripts utilizados en SARP.es, el servidor de rol en Grand Theft Auto: San Andreas. Está pensado como un espacio colaborativo en el que cualquier miembro de la comunidad, sin importar su nivel de experiencia, puede participar activamente en el crecimiento técnico del proyecto.

Invitamos a todos los desarrolladores interesados a contribuir con ideas, sistemas o mejoras. Toda propuesta será revisada y sometida a pruebas por el equipo de desarrollo oficial. Aquellos sistemas que cumplan con los estándares de calidad, rendimiento y compatibilidad, serán integrados al script principal del servidor, permitiendo que sus autores vean su trabajo reflejado directamente en el juego.

El objetivo de este proyecto de código abierto es democratizar el conocimiento, fomentar el aprendizaje mutuo y brindar una oportunidad real a quienes siempre han soñado con aportar en el desarrollo de un proyecto serio y colaborativo dentro de su juego favorito de Grand Theft Auto - SAMP.

## Requerimientos
### SA-MP 0.3.DL-R1
- [SA-MP 0.3.DL-R1 Windows Server](https://sampforum.blast.hk/files/03DL/samp03DL_svr_R1_win32.zip)
- [SA-MP 0.3.DL-R1 Linux Server](https://sampforum.blast.hk/files/03DL/samp03DLsvr_R1.tar.gz)

### Pawn Community Compiler:
[Release 3.10.10](https://github.com/pawn-lang/compiler/releases/tag/v3.10.10)

### Instrucciones
1. Descargar Windows Server o Linux Server y pegar ejecutables en el repositorio.
2. Descomprimir el Pawn Compiler en la carpeta /pawno.
3. Compilar en tu editor favorito y ejecutar samp-server.exe.

## Servidor local en Windows
La preparación reproducible y los comandos de inicio, estado y apagado están en [.internal/local-windows-server.md](.internal/local-windows-server.md).
El entorno se instala en `.local-server/` (ignorado por Git) y permite conectar desde el mismo PC a `127.0.0.1:7777`.

## Practicas de tiro (MVP)
Abre `/practicaconfig` para configurar hasta diez blancos desde diálogos: objeto y comportamiento independientes, editor visual de posición/rotación, vista previa privada, varias armas permitidas, reglas con ayuda y aleatoriedad. Inicia con `/practica`; incluye cuenta regresiva en pantalla y resultados explicados.
La guia para probarlo esta en [.internal/shooting-practice-testing.md](.internal/shooting-practice-testing.md).

## Licencia
Este proyecto está licenciado bajo los términos de la [Licencia MIT](https://opensource.org/licenses/MIT). Eres libre de usar, modificar y distribuir este software con pocas restricciones.  