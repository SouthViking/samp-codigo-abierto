# Configurar y probar las prácticas de tiro

Estado local al 7 de octubre de 2026: servidor apagado a petición del usuario. Los últimos ajustes compilados y probados se cargarán en el próximo inicio solicitado.

Actualizado: 2026-10-07. Un puesto, hasta diez blancos y un participante por ronda.

## Entrar y abrir la configuración

1. Inicia con Iniciar-SARP.cmd, o desde el repositorio ejecuta:
   `powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Start`.
   Si estaba encendido, detenlo primero para aplicar cambios.
2. Conecta SA-MP **0.3.DL-R1** a **127.0.0.1:7777** desde este PC.
3. Accede como RCON con la clave local de .local-server/server.cfg. En el entorno preparado,
   Acceso-RCON.txt contiene el comando de acceso.
4. Estando vivo y a pie, escribe **/practicaconfig**, sin argumentos.

Los puntos anteriores se recuperan automáticamente. Los antiguos argumentos de configuración
solo abren el menú y ya no modifican datos.

| Opción | Función |
| --- | --- |
| Punto de inicio | Guardar directamente la posición y orientación actuales del personaje. |
| Blancos | Añadir, nombrar, elegir objeto/comportamiento, mover, girar, duplicar y eliminar. |
| Reglas del ejercicio | Armas permitidas, puntaje, errores, tiempos, orden y distribución. |
| Vista previa | Checkpoint de inicio, objetos y etiquetas privadas. |
| Iniciar práctica | Aparece si el puesto está listo, estás en el inicio y equipas un arma permitida. |
| Cancelar práctica | Aparece durante tu ronda activa. |
| Reiniciar configuración | Borrar inicio y blancos y restaurar reglas, con confirmación. |
| Reintentar guardado | Solo aparece si falló la escritura del archivo. |

El título principal es **Campo de tiro**. Iniciar práctica aparece solo cuando puedes iniciar, sin un detalle largo.
La vista previa está disponible desde el menú principal. Las condiciones se comprueban de nuevo al seleccionar Iniciar.

## Partir de cero y marcar el inicio

1. Para borrar el montaje anterior, elige **Reiniciar configuración > Reiniciar**.
2. Camina al lugar desde el que se disparará y mira hacia la zona de blancos.
3. Selecciona **Punto de inicio** en el menú principal. Guarda inmediatamente tu posición y orientación,
   sin abrir un submenú, y vuelve al menú principal.
4. Cambiar el inicio a otro interior/mundo pide confirmar la eliminación de los blancos anteriores.
   Cancelar conserva todo. En el mismo interior/mundo se conservan: revisa sus distancias con Vista previa.

## Crear, mover y orientar blancos

1. Acércate a menos de **100 metros del inicio**, en el mismo interior y mundo.
   Abre **Blancos > Añadir blanco**. Desde otra ciudad se rechaza la creación antes de abrir el editor.
2. Elige una tarjeta del catálogo: **Caja marcada (1271)**, **Silueta armada A/B (1583/1584)**
   o **Silueta civil A/B (1585/1586)**. Cada tarjeta muestra el objeto y su nombre.
3. Elige **Blanco de tiro (disparar)** o **Blanco protegido (no disparar)**, independiente del modelo.
   Esto activa distribución fija al guardar. Luego puedes activar la aleatoria en Reglas.
4. Se abre el editor visual sobre un objeto temporal, inicialmente delante del personaje.
   Usa los controles del juego para **mover y girar en los tres ejes**.
5. Colócalo entre **3 y 100 metros del inicio**, en su mismo interior/mundo.
   Para probar usa 8-15 metros y orienta la cara hacia el puesto.
6. **Confirmar en el editor guarda todos los cambios**. **Cancelar descarta toda la edición**
   y conserva lo anterior. Si era un blanco nuevo, no se añade.
7. Repite con dos blancos de disparo y uno de no disparo.

Para editar uno existente selecciona su fila: Nombre, Objeto, Comportamiento, Mover y orientar,
Duplicar blanco, Eliminar blanco y, al final, Guardar cambios. El nombre admite 32 caracteres. Escribe
un guion (-) para restaurar el nombre automático. Cambios de nombre/modelo/comportamiento quedan
pendientes hasta Guardar cambios o confirmar el editor. Descartar conserva el blanco anterior.

Duplicar abre el editor con una copia. Confirmar usa un espacio libre sin alterar el original.
Eliminar exige confirmación. Máximo diez blancos. Si no aparece el editor, acércate al blanco.
Termina cualquier otro editor de objetos antes de abrirlo. Solo un administrador puede editar
el puesto a la vez. Las rondas esperan hasta que guarde o descarte.

Al abrir Blancos aparecen automáticamente los blancos guardados y el inicio, solo para ti.
Siguen visibles en los diálogos, el catálogo y el editor. El blanco que estás moviendo sustituye
su copia guardada mientras lo editas. Los demás conservan sus posiciones y permanecen visibles.
Guardar actualiza el blanco cambiado. Cancelar restaura su versión guardada.
Volver al menú principal retira esta vista automática. Si activaste Vista previa previamente,
seguirá activada hasta que la ocultes.

El inicio se puede marcar en una ubicación válida aunque no haya un inicio previo o estés a más
de 100 metros del anterior. El radio de 100 metros limita la colocación de blancos.

## Vista previa

1. Selecciona **Vista previa**.
2. El diálogo se cierra y puedes caminar. Se muestra el checkpoint de inicio y todos los
   objetos con sus modelos, posiciones y rotaciones. Las etiquetas indican nombre y comportamiento. El nombre automático es Blanco N, sin repetir el número.
3. Solo tú ves estos elementos. No hay ronda, sonidos ni puntuación.
   Los impactos se bloquean y el objeto alcanzado se repone para evitar que quede desaparecido.
   La reposición también funciona en la vista automática de configuración.
4. Abre /practicaconfig y selecciona **Ocultar vista previa** para retirarlos.

La vista se limpia al desconectarte, morir, entrar en un vehículo o cambiar de interior/mundo.
Guardar cambios actualiza las vistas sin retirar los blancos que no cambiaron. Iniciar una práctica retira todas las vistas previas.

## Reglas y primera ronda

Para dos blancos de disparo y uno de no disparo configura:

| Regla | Valor de prueba |
| --- | --- |
| Armas permitidas | Desert Eagle (24), opcionalmente M4 (31) |
| Puntos mínimos | 2 |
| Errores máximos | 0 |
| Preparación / exposición / intervalo | 3 / 5 / 1 segundos |
| Orden | Aleatorio |
| Distribución | Fija por posición |

Abre **Reglas del ejercicio > Armas permitidas**. Selecciona un arma y pulsa **Cambiar**:
alterna entre **Permitida** y **No permitida**. Puedes habilitar varias. Siempre debe quedar al menos una.
Cada cambio se guarda inmediatamente. Pulsa **Volver** al terminar.
Puedes alternar entre las armas habilitadas durante la ronda. Elegir o disparar con otra cancela la práctica.

Los campos numéricos muestran una explicación, el valor actual y el rango permitido:

- **Puntos mínimos:** puntuación neta necesaria para aprobar. Aciertos menos errores.
  Ejemplo: 4 aciertos y 1 error dejan 3 puntos. El máximo alcanzable depende de los blancos de tiro configurados.
- **Errores máximos:** disparar ante un blanco protegido cuenta, incluso al aire. 0 exige no cometer errores.
  Superar el límite impide aprobar, pero el ejercicio continúa hasta el final.
- **Preparación:** de 1 a 30 segundos enteros antes del primer blanco, con cuenta regresiva visual y pitidos.
- **Tiempo por blanco:** exposición máxima. Un blanco de tiro sin impacto desaparece sin sumar puntos;
  ante un blanco protegido hay que esperar sin disparar. Acepta de 0.1 a 60.0 segundos.
- **Intervalo:** pausa entre blancos, de 0.0 a 30.0 segundos. 0 elimina la pausa programada.
  Disparar en ella reduce la precisión.

Tiempo por blanco e Intervalo admiten un decimal, con punto o coma, por ejemplo **0.5** o **0,5**.
Para probar tiempos cortos, usa exposición 0.5 e intervalo 0.2. La exposición no admite 0.
La preparación y los límites de puntos/errores siguen usando enteros.

Cada regla se guarda al confirmarla. Orden aleatorio cambia cuándo aparece cada punto,
conservando su objeto/comportamiento. Distribución aleatoria mueve **objeto y comportamiento juntos**
entre posiciones. Conserva cantidades y no sobrescribe el montaje. Una secuencia puede repetirse.
Las rotaciones pertenecen a las posiciones: usa orientaciones compatibles para distribución aleatoria.

Atajo de desarrollo: con RCON y estando vivo y a pie, usa **/irpruebas** para ir a la zona de pruebas:
XYZ 802.8580, 1674.5205, 5.2813, orientación 181.2324, interior y mundo 0.
El comando mueve al personaje. El inicio se marca desde Punto de inicio en /practicaconfig.

1. Vuelve al inicio, a menos de 1.5 metros.
2. En este servidor de desarrollo, obtén la Desert Eagle con **/creararma 24 100 0** y equípala.
   Para probar el cambio de arma, permite también M4 y obtén una con **/creararma 31 100 0**.
3. Usa **/practica**, o Iniciar práctica desde el menú.
4. Verás **Comienza en 3, 2, 1** en pantalla, con un pitido por segundo. El contador desaparece
   al aparecer el primer blanco. Oirás otro pitido en cada aparición. Ambos comportamientos usan
   el mismo pitido. Cajas: roja DISPARAR, azul NO DISPARAR. Las siluetas conservan sus texturas:
   el instructor debe explicar el criterio y elegir figuras distinguibles.
5. Permanece a menos de 3 metros del inicio. Cada posición aparece una sola vez.
6. Al terminar hay otra señal, una acción ambiental y un resumen privado de cinco líneas. Todo el texto es blanco, salvo
   **Aprobado** en verde o **Reprobado** en rojo. «Veredicto:» y la puntuación siguen blancos.

Los pitidos de cuenta regresiva y la primera aparición quedan separados por al menos un segundo.
Si el servidor se retrasa, no recupera los pitidos en ráfaga ni acorta la última pausa.

Un impacto válido en DISPARAR suma **+1**. Cualquier disparo durante NO DISPARAR, incluso al aire,
resta **1** y registra un error. El primero retira el blanco. Dejarlo vencer sin disparar es correcto.
Los disparos del intervalo cuentan para precisión sin volver a penalizar el blanco anterior.
Repetir disparos ante un mismo blanco de tiro también reduce la precisión. Para un ejercicio limpio,
procura un tiro por blanco de tiro y ninguno ante los protegidos. Disparar de más no cambia por sí
solo el veredicto, que sigue dependiendo de los puntos netos y los errores de identificación.
Puntos netos = impactos menos errores, incluso negativos. Se exige alcanzar el mínimo de puntos
y no superar los errores. Los disparos de preparación quedan excluidos.

El resultado queda organizado en **reglas, resultados y veredicto**:

- **Reglas:** puntos netos mínimos, errores máximos y valor de acierto/error.
- **Resultados:** aciertos, blancos sin impactar a tiempo, protegidos respetados y errores de identificación.
- **Disparos y precisión:** impactos en blancos de tiro divididos por disparos contados.
  Se muestra con el símbolo %, por ejemplo «Precisión: 50.0%.», sin explicación entre paréntesis. Sin disparos, aparece «no disponible».
- **Veredicto:** solo Aprobado en verde o Reprobado en rojo. El rótulo y la puntuación neta son blancos, en su propia línea.

/practica cancelar interrumpe tu ronda. /practica ayuda muestra armas, reglas y puntuación.
Si sales del radio de 3 metros, verás **Incompleta: Cancelada automáticamente por salir del puesto de tiro.**
La práctica se cancela y quienes están cerca del puesto ven
**Nombre Apellido abandona el puesto de tiro**, con tu nombre de rol.
Cualquier jugador puede practicar. Configurar requiere RCON. No se entrega ni repone equipo.
Sonidos y acciones de rol se limitan a 10 metros del puesto en su mismo interior/mundo.

## Guardado y comprobaciones

- Archivo local: .local-server/scriptfiles/shooting_practice.cfg. En otro servidor:
  scriptfiles/shooting_practice.cfg, relativo a su directorio de ejecución.
- Formatos 1 a 4 conservan posiciones y reglas, y su arma se mantiene como única permitida.
  Los formatos 1 a 3 usan caja 1271 con rotaciones X/Y cero. El 4 conserva modelos y rotaciones completos.
  El formato 5 conserva las múltiples armas. Los formatos 1 a 5 convierten exposición e intervalo
  de segundos a milisegundos al cargar. El siguiente guardado usa formato 6 y conserva las décimas.
  Cargar no reescribe el archivo.
- Si falla la escritura, Sin guardar y Reintentar guardado permiten guardar lo que sigue en memoria
  antes de reiniciar. La escritura no es transaccional.
- Para que aparezca Iniciar práctica debe existir un blanco de tiro y los puntos y errores deben respetar
  sus cantidades. También debes estar vivo, a pie, cerca del inicio y con un arma permitida.
  El puesto debe estar libre y nadie puede estar editándolo. /practica explica qué falta si intentas iniciar.
  Durante una ronda no se puede cambiar la configuración.
- Comprueba en el cliente: catálogo, mover/girar/confirmar/cancelar, checkpoint, etiquetas,
  colisiones de siluetas, tildes, posición del contador y sonidos. Dispara a cada modelo en la vista previa
  y comprueba que se repone sin desaparecer el resto. El motor puede mostrar brevemente su rotura.
  La reparación se aplica a impactos de bala, no garantiza protección ante golpes o explosiones.
  Comprueba que solo Aprobado/Reprobado tenga color y que el resto del resultado sea blanco.
  Confirma el símbolo % en la precisión. Prueba dos blancos de tiro: falla un intento, acierta,
  dispara durante el intervalo y acierta el segundo. Debe mostrar 2 aciertos, 4 disparos y 50.0%.
  Verifica el espacio de un segundo entre pitidos y primera aparición.
  Prueba exposición 0.5 e intervalo 0.2, guarda y revisa esos valores después de un reinicio solicitado. Con dos jugadores confirma la privacidad del preview y que
  ambos pueden observar el blanco de una ronda real y el aviso de abandono. Solo el participante ve
  el contador y sus resultados. Reinicia y revisa nombres/modelos/rotaciones y armas seleccionadas.
- **606 comprobaciones automáticas sin fallos** y compilación completa de esta versión.
  El servidor sigue activo en 127.0.0.1:7777 con la versión del último reinicio solicitado.
  La corrección de marcar el inicio y el comando /irpruebas están cargados y la configuración se conservó.
  Los ajustes posteriores de veredicto, porcentaje, orden del guardado, tipo de blanco y mensaje
  de cancelación están compilados y probados, pendientes del próximo reinicio solicitado.
  El servidor no se reinicia automáticamente durante tus pruebas.
  Los controles, renderizado y audio del cliente siguen siendo comprobaciones manuales.

Alcance: un puesto, sin historial, facciones, permisos por rango, varias instalaciones ni assets propios.
