# AmnesiaChat P2P

> **Arquitectura Híbrida C / Java con Seguridad Avanzada y Endurecimiento de Memoria Local.**
> Developed by **ju57a70uc4n**

AmnesiaChat es una plataforma de mensajería instantánea descentralizada y asíncrona basada en una topología **Punto a Punto (P2P)**. El sistema rompe el paradigma tradicional al dividir estrictamente sus responsabilidades en dos procesos independientes: una interfaz gráfica robusta y reactiva desarrollada en **Java (Swing)** y un motor criptográfico de alto rendimiento programado en **C puro** (`amnesia_engine`). 

Ambos mundos se comunican mediante canales de comunicación entre procesos (**IPC**) utilizando descriptores de archivos y tuberías estándar (*Pipes*), delegando toda la confidencialidad de la red en la suite **libsodium**.

---

## Funcionalidades Principales

### 1. Red P2P Asíncrona sin Servidor
* **Conexiones Directas:** Establecimiento de sockets directos `AF_INET` mediante protocolos TCP estándar para comunicación par a par.
* **I/O No Bloqueante con `poll()`:** El bucle principal del motor en C monitorea hasta 11 descriptores en simultáneo (tuberías de entrada de Java, sockets de balizas/Faro y conexiones activas de clientes). Cero sobrecarga de hilos (*multithreading*), garantizando estabilidad total y bajo consumo de recursos.

### 2. Hardening de Memoria Local (Anti-Forense)
Diseñado específicamente para mitigar ataques de volcado físico en frío (*Cold Boot*) o inspecciones maliciosas del sistema de archivos mediante `root` (`/proc/[PID]/mem`):
* **Prevención de Paginación (`mlock`):** La arena de memoria (`ChatArena`) donde residen las claves y las estructuras de los mensajes se bloquea en la RAM física, prohibiendo al Kernel enviarla al espacio de intercambio (*Swap*) en el disco.
* **Sanitización de Volatilidad (`sodium_memzero`):** Los buffers temporales tanto en la pila (*Stack*) como en el *Heap* se destruyen inmediatamente después de ser utilizados, eliminando residuos de texto plano.
* **Desactivación de Core Dumps:** Restricción a nivel de Kernel (`RLIMIT_CORE = 0`) para impedir que el sistema genere volcados ante fallos críticos del motor.

### 3. Cifrado Simétrico Autenticado (AEAD)
* **Privacidad y Autenticidad:** Cada paquete que viaja por la red es sellado usando construcciones criptográficas de grado militar: cifrado simétrico **ChaCha20** combinado con la firma de autenticación de mensajes (MAC) **Poly1305**.
* **Protección Anti-Replay:** Uso estricto de Nonces criptográficos únicos por paquete, asegurando que dos mensajes idénticos generen flujos cifrados totalmente distintos imposibles de predecir o inyectar.

### 4. Botón de Pánico (Autodestrucción Atómica)
* Gatillado instantáneo desde el controlador gráfico. El motor en C intercepta la señal, cierra las conexiones de red, purga las estructuras críticas llenándolas con ceros reales y finaliza mediante un `exit(0)` fulminante en microsegundos, sin dejar rastros en los mapas de memoria.

---

## Auditoría y Pruebas Realizadas

### Auditoría de Tráfico de Red (`tcpdump`)
Para comprobar la opacidad del canal de transporte, se interceptó la interfaz virtual de *loopback* (`lo`) en el puerto de escucha. Como se observa en el volcado, el texto plano fue completamente destruido y transformado en entropía pura antes de salir del socket:

### Análisis Forense de RAM con GDB

Durante la fase de auditoría interna, se realizó un volcado de memoria virtual del Heap (dump memory volcado_ram.bin) encontrando trazas de texto plano procedentes de los buffers automáticos de la biblioteca estándar de C (glibc).

---

## Patrones de Diseño Aplicados

### Patrones de Diseño GoF (Gang of Four)

 **Model-View-Controller (MVC)**: Arquitectura central de la interfaz en Java. La vista gráfica (`MainView`) está completamente desacoplada del estado interno de los datos, delegando toda la captura de eventos, validación de inputs y el despacho de comandos hacia el proceso en C al componente `Controller`. 
 **Factory (Fábrica)**: Utilizado para la abstracción y creación de componentes críticos del sistema (como la instanciación abstracta de comandos estructurados IPC o la configuración estandarizada de sockets de red). Centraliza la lógica de inicialización para evitar la duplicación de código de bajo nivel. 

### Principios y Patrones GRASP

* **Controlador (Controller):** El objeto `Controller` de Java actúa como el primer punto de contacto para todos los eventos del sistema fuera de la capa gráfica. Recibe las señales físicas (como el clic en el botón de pánico) y las traduce en comandos IPC formateados antes de inyectarlos al motor C.
* **Bajo Acoplamiento (Low Coupling):** La separación física en dos procesos independientes a nivel de sistema operativo asegura que un fallo crítico en el hilo de Swing de Java no afecte la integridad del motor criptográfico en C, manteniendo los sockets protegidos.
* **Alta Cohesión (High Cohesion):** Cada módulo tiene una responsabilidad única y bien delimitada. `MainView` se encarga exclusivamente del renderizado de componentes, `Controller` maneja el flujo lógico, y el motor en C se concentra únicamente en la red, el parseo y el endurecimiento de la RAM.
* **Indirección (Indirection):** El canal de comunicación IPC actúa como un puente abstracto entre Java y C, eliminando dependencias nativas complejas (como JNI o JNA) y logrando interoperabilidad mediante flujos estándar de entrada y salida (`stdin`/`stdout`).

---
