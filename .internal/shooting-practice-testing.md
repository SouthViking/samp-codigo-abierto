# Probar las practicas de tiro

Esta version permite configurar un puesto y completar practicas de 1 a 10 blancos.
Las dianas provisionales son cajas del juego (modelo 1271). No hay que instalar modelos personalizados.

## 1. Actualizar e iniciar el servidor local de Windows

Desde la raiz del fork, en la rama `feat/shooting-practice-mvp`:

```powershell
git pull --ff-only origin feat/shooting-practice-mvp
# Solo si el entorno aun no esta preparado:
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/prepare-local-server.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Start
```

Si ya estaba encendido, usa `-Action Stop` antes de `Start`: el inicio compila los fuentes actuales.
Conecta un cliente SA-MP 0.3.DL-R1 a **127.0.0.1:7777**.
En otro servidor de pruebas, compila `gamemodes/SARP.pwn`, instala el AMX y verifica `lagcompmode 1`
en su `server.cfg`. Los plugins y requisitos siguen siendo los del proyecto.

## 2. Configurar una prueba corta

1. Entra como administrador usando `/rcon login <clave local>`. La clave esta en
   `.local-server/server.cfg`; no la publiques.
2. Busca una zona despejada, ponte a pie y usa `/practicaconfig inicio`.
3. Camina unos **8-15 metros**, en una posicion visible desde el inicio, y usa
   `/practicaconfig blanco 1`. La caja aparecera centrada en la posicion del personaje.
4. Marca otras dos posiciones con `/practicaconfig blanco 2` y `/practicaconfig blanco 3`.
5. Configura las reglas:

```text
/practicaconfig arma 24
/practicaconfig aprobar 2
/practicaconfig preparacion 3
/practicaconfig tiempo 5
/practicaconfig intervalo 1
/practicaconfig ver
```

6. Regresa al inicio, equipa una Desert Eagle con municion y usa `/practica`.
   En el servidor local puedes obtenerla con el comando ya existente `/creararma 24 100 0`.
   La practica no entrega ni repone equipo.
7. Veras la cuenta atras, una caja por vez y un resultado por chat. Con esta configuracion,
   acertar dos de las tres cajas alcanza para aprobar. La precision se informa por separado.

Los cambios se guardan automaticamente en `scriptfiles/shooting_practice.cfg`, relativo al directorio
del servidor. Con el lanzador local es `.local-server/scriptfiles/shooting_practice.cfg`.
Un mensaje confirma cada guardado. Si falla, los cambios siguen en memoria: corrige los permisos
(o crea `scriptfiles`) y reintenta `/practicaconfig guardar` antes de reiniciar.

## Comandos disponibles

| Comando | Funcion |
| --- | --- |
| `/practicaconfig` o `ayuda` | Ayuda de configuracion; requiere administrador RCON. |
| `/practicaconfig inicio` | Marca o reemplaza el puesto desde la posicion actual. |
| `/practicaconfig blanco <1-10>` | Marca o reemplaza una posicion; no hace falta llenar los diez espacios. |
| `/practicaconfig eliminar <1-10>` | Quita una posicion. |
| `/practicaconfig arma <22-34>` | Selecciona el arma permitida. El ID 24 es Desert Eagle. |
| `/practicaconfig aprobar <puntos>` | Minimo de aciertos, entre 1 y la cantidad de blancos configurados. |
| `/practicaconfig preparacion <1-30>` | Segundos de cuenta atras. |
| `/practicaconfig tiempo <1-60>` | Segundos maximos por blanco. |
| `/practicaconfig intervalo <0-30>` | Segundos de espera entre blancos. |
| `/practicaconfig ver` | Lista posiciones, reglas y cualquier problema pendiente. |
| `/practicaconfig guardar` | Reintenta el guardado manualmente. `exportar` funciona como alias. |
| `/practica` | Inicia desde el puesto con el arma seleccionada. No requiere RCON. |
| `/practica cancelar` | Cancela tu intento y libera el puesto. |

Cada posicion configurada se usa **una vez**, en orden numerico, saltando los espacios vacios.
Un acierto vale un punto y retira inmediatamente el blanco. Al vencer su tiempo se registra una omision.
Los disparos entre blancos cuentan para la precision; los de la preparacion no.

El inicio exige estar a un maximo de 1.5 metros del puesto. Durante la ronda puedes moverte hasta
3 metros. Los blancos deben estar entre 3 y 100 metros del inicio, en el mismo interior y mundo;
para la primera prueba usa distancias cortas compatibles con el arma.

Los tiempos se revisan mediante el temporizador compartido de 500 ms. Son tiempos nominales:
las transiciones pueden ocurrir en el siguiente tick. Un intervalo 0 avanza en el siguiente tick.
Con diez blancos, preparacion 3, exposicion 5 e intervalo 1, el maximo nominal es 62 segundos;
acertar antes acorta la ronda.

## Comprobaciones dentro del juego

- Acertar a todas las cajas; dejar expirar todas; combinar aciertos y omisiones.
- Fallar disparos e intentar disparar entre cajas: esos disparos bajan la precision.
- Cancelar y comenzar otra ronda inmediatamente.
- Alejarte, entrar a un vehiculo, cambiar de arma, morir o desconectarte durante una ronda.
  Debe quedar incompleta y el puesto debe liberarse.
- Quedarte sin municion: los blancos deben seguir venciendo, sin recargas automaticas.
- Con otro jugador: comprobar que no pueda ocupar el puesto, cancelar tu ronda ni sumar tus puntos.
- Intentar editar la configuracion mientras el puesto esta ocupado: debe impedirlo.
- Reiniciar el servidor y usar `/practicaconfig ver`: debe recuperar posiciones y reglas.
- Confirmar que la caja es visible y registra impactos. Su altura, modelo y colision son parte
  de esta primera validacion en el cliente; los tests automaticos no dibujan objetos reales.

La configuracion es de un solo puesto compartido. Cada ronda admite un participante.
Todavia no hay historial, certificaciones FTP, premios, interfaz grafica ni varias instalaciones.
Los exports antiguos `shooting_practice_points.inc` no se importan automaticamente; marca las posiciones
con los nuevos comandos para crear la configuracion persistente.
