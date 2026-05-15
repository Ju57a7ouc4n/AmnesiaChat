# Documentación de Arquitectura: Motor de Chat Efímero (Fase 1)

## 1. Resumen del Proyecto

Este proyecto implementa un motor de mensajería P2P altamente volátil, diseñado bajo principios estrictos de Seguridad Operativa (OpSec). Soporta exclusivamente texto plano para minimizar vectores de ataque y desbordamientos de búfer. Su característica principal es la **Amnesia por Diseño**: las identidades son efímeras, no existe almacenamiento en disco y la gestión de memoria garantiza la destrucción absoluta de la información tras su uso.

La arquitectura sigue el modelo **Daemon/Client**, donde este motor (escrito en C) funciona como el _backend_ criptográfico y de red, manteniéndose completamente aislado de la futura interfaz gráfica.

---

## 2. Decisiones de Diseño (The "Why")

### 2.1. Gestión de Memoria: Arenas y Bloqueo (`mlock`)

En lugar de asignar memoria dinámicamente por cada mensaje recibido (`malloc`), se implementó un modelo de **Pool de Memoria Contigua (Arenas)**.

- **El Problema:** La asignación dinámica tradicional fragmenta la memoria y permite que el sistema operativo pagine datos sensibles al disco duro (swap), dejando rastros forenses.
    
- **La Solución:** Se pre-asigna un bloque de tamaño fijo (`ChatArena`) para toda la sesión y se ancla físicamente a la RAM utilizando la llamada al sistema `mlock()`. Cuando la sesión termina, un único pase destructivo (`memset_explicit`) actúa como una ola que limpia instantáneamente los mensajes, claves y metadatos, sin dejar estructuras huérfanas en la memoria.
    

### 2.2. Buffer Circular Estricto y Fragmentación

El historial de chat se gestiona mediante un búfer circular de tamaño fijo (ej. 20 mensajes).

- **Eficiencia O(1):** El consumo de RAM está estrictamente delimitado. El mensaje nuevo sobrescribe automáticamente al más antiguo sin necesidad de reasignar memoria.
    
- **Padding y Fragmentación:** Para prevenir el análisis de tráfico por tamaño de paquete, los mensajes se dividen o rellenan (padding) en marcos estrictos de 512 bytes. Un atacante de red solo verá bloques uniformes, independientemente de si el usuario envió una palabra o un párrafo.
    

### 2.3. Criptografía Híbrida y Curva Elíptica (X25519 vs. RSA)

Para el cifrado extremo a extremo, se descartó RSA en favor de la Criptografía de Curva Elíptica (ECC) y cifrado simétrico, preparándolo para un futuro esquema post-cuántico.

- **Velocidad y Memoria:** Generar un par de claves RSA seguras (>4096 bits) es un proceso lento y las claves ocupan más de 512 bytes, lo que rompería los límites estrictos de los marcos de memoria. X25519 (libsodium) provee mayor seguridad con claves de solo 32 bytes, permitiendo generar identidades al vuelo en microsegundos sin bloquear el hilo de ejecución.
    
- **Forward Secrecy:** Las claves asimétricas solo se usan para el "apretón de manos" inicial. Luego se deriva una llave simétrica compartida y las asimétricas se destruyen. Los mensajes se cifran en reposo dentro del motor usando `crypto_secretbox` (XSalsa20-Poly1305), garantizando que si una clave futura es comprometida, el historial pasado permanece inexpugnable.
    

---

## 3. Especificación de la API Interna (Core Functions)

### `pChat newChat(uint32_t chatId)`

Inicializa una nueva sesión de chat en memoria segura.

- **Funcionamiento:** Solicita al sistema un bloque del tamaño de `ChatArena` y ejecuta `mlock()` para prevenir la paginación a disco.
    
- **Estado Inicial:** Rellena la estructura con ceros, asigna el ID de sesión e inicializa el índice del búfer circular en `-1` para preparar el espacio para el primer mensaje.
    
- **Retorno:** Un puntero al bloque anclado en RAM, o `NULL` si el sistema deniega el bloqueo de memoria.
    

### `void newMessage(ChatArena* chat, const char* text)`

Procesa, fragmenta, encripta y almacena un mensaje entrante de texto plano en el búfer circular.

- **Fragmentación:** Calcula la longitud del texto. Si excede los límites útiles del marco (`UTIL_TEXT`), divide el texto en múltiples operaciones, marcando el flag `is_fragmented = 1`.
    
- **Cifrado al Vuelo:** Genera un Nonce aleatorio mediante `randombytes_buf` y cifra el bloque de texto directamente hacia la dirección de memoria destino usando `crypto_secretbox_easy`. El texto plano nunca persiste en la estructura.
    
- **Manejo de Índices:** Incrementa el índice del búfer de forma cíclica (0 a `MAX_INDEX - 1`), sobrescribiendo los mensajes más antiguos de forma segura (aplicando `memset` antes de escribir).
    

### `void getChatHistory(ChatArena* chat)`

Reconstruye el historial cronológico de la sesión para ser consumido por la Interfaz de Usuario.

- **Funcionamiento:** Recorre el búfer circular comenzando lógicamente desde el mensaje más antiguo que no ha sido sobrescrito.
    
- **Desencriptación:** Verifica si el marco está en uso (`used == 1`). Si es así, utiliza la llave simétrica compartida y el Nonce asociado para desencriptar el bloque al vuelo en una variable temporal.
    
- **Formato de Salida:** Imprime el texto reconstruido en la salida estándar (para ser capturado por el socket IPC), gestionando los saltos de línea en función de si el marco actualiza un mensaje fragmentado o es el final del mismo.
    

### `void endChat(ChatArena* chat)`

Ejecuta la finalización de la sesión y la destrucción de la memoria ("Botón de Pánico / Colgar").

- **Scorched Earth:** Utiliza `memset_explicit` sobre todo el puntero del `ChatArena` para evitar optimizaciones del compilador, asegurando que todos los bytes (texto cifrado, llaves simétricas, nonces y metadatos) sean sobrescritos con ceros.
    
- **Liberación:** Llama a `munlock()` para devolver los permisos de paginación al SO y finalmente libera el bloque con `free()`.