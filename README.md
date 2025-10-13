# 🍽️ Microondas Didáctico con STM32L053R8

Este proyecto simula el funcionamiento de un **horno microondas digital** usando un **microcontrolador STM32L053R8**.  
Incluye pantalla **LCD 16x2**, **teclado matricial 4x4**, **4 displays de 7 segmentos** y un **motor paso a paso (28BYJ-48 con driver ULN2003)** que representa el giro del plato.

El sistema permite:
- Ingresar tiempos manualmente o con teclas rápidas.
- Iniciar, pausar o cancelar el calentamiento.
- Mostrar mensajes en LCD y tiempo en displays.
- Girar el motor mientras “calienta”.

---

## ⚙️ Funcionalidades Principales

### 🧾 LCD 16x2
Muestra el estado del microondas:
- `Listo` → sistema en espera.  
- `Esperando *` → esperando inicio.  
- `Calentando` → proceso en curso.  
- `¡Listo!` → ciclo terminado.  
- `Tiempo inválido` → ingreso incorrecto.  
- `Cancelado` → proceso detenido.

---

### ⌨️ Teclado Matricial 4x4
Control principal del microondas.  
Cada tecla tiene una función específica:

| Tecla | Acción | Descripción |
|:------|:--------|:-------------|
| `0–9` | Ingreso manual | Define tiempo en formato MM:SS |
| `A` | Café ☕ | Programa rápido de **2:00 minutos** |
| `B` | Pollo 🍗 | Programa rápido de **3:00 minutos** |
| `C` | Sopa 🍜 | Programa rápido de **2:30 minutos** |
| `D` | Poporopos 🍿 | Programa rápido de **1:30 minutos** |
| `E / #` | Cancelar ✖️ | Detiene y limpia la operación |
| `F / *` | Iniciar ▶️ | Comienza el ciclo de calentamiento |

---

### 🔢 Displays de 7 Segmentos
- Muestran el tiempo restante (durante uso).  
- Muestran la hora (en reposo).  
- Usan **multiplexado dinámico** controlado por GPIOA y GPIOC.  
- Formato visual hora: **HH:MM**.
- Formato visual tiempo restante: **MM:SS**.

---

### 🔄 Motor Paso a Paso (28BYJ-48 + ULN2003)
Representa el plato giratorio del microondas.

#### 📡 Conexión de pines:
| Bobina | ULN2003 IN | STM32 Pin |
|:--------|:-------------|:-----------|
| IN1 | 1 | PC8 |
| IN2 | 2 | PC9 |
| IN3 | 3 | PB8 |
| IN4 | 4 | PB9 |

- Sentido: horario (puede invertirse si se cambia la secuencia).  
- Secuencia activa: **1 → 2 → 3 → 4**.  
- Velocidad ajustable modificando `motor_delay`.

---

### ⏰ Reloj Interno
- Funciona en formato **24h**.
- Se muestra en los displays cuando el microondas no está activo.

---

## 🧩 Pines Usados

| Módulo | Pines STM32 | Descripción |
|:--------|:-------------|:-------------|
| **LCD (modo 4 bits)** | PA6 (RS), PA7 (EN), PA8–PA11 (D4–D7) | Comunicación paralela |
| **Teclado 4x4** | PB0–PB3 (Columnas), PB4–PB7 (Filas) | Escaneo por software |
| **Displays 7 segmentos** | PC0–PC7 (segmentos), PA0,PA1,PA4,PA5 (control de dígitos) | Multiplexado |
| **Motor paso a paso** | PC8, PC9, PB8, PB9 | Bobinas del 28BYJ-48 |
| **Alimentación** | 5 V (motor/LCD), 3.3 V (lógica STM32) | Fuente externa |

---

## 🔧 Materiales Necesarios

| Cant. | Componente | Descripción |
|:-------|:-------------|:-------------|
| 1 | STM32 Nucleo-L053R8 | Microcontrolador principal |
| 1 | LCD 16x2 | Pantalla de estado |
| 1 | Teclado 4x4 | Entrada de usuario |
| 1 | ULN2003 | Driver para el motor |
| 1 | Motor 28BYJ-48 (5V) | Simula el plato giratorio |
| 4 | Displays 7 segmentos | Indican tiempo/reloj |
| Varios | Resistencias y cables Dupont | Conexiones |
| 1 | Protoboard o PCB | Montaje |
| 1 | Fuente 5V / 1A | Alimentación estable |

---

## ⚙️ Configuraciones Importantes

| Variable | Propósito | Valor típico |
|:-----------|:------------|:-------------|
| `time_micro` | Base de conteo de segundos | 60 |
| `motor_delay` | Controla velocidad del motor | 2–5 |
| `delay_ms()` | Ajuste fino del retardo | `i < 40` |
| `activar_motor()` | Define secuencia de bobinas | 1–2–3–4 |
| `microwave_time_move()` | Inserta dígitos del tiempo | Shift circular |

---

## 🧠 Notas de Uso y Ajuste

- Si el motor gira al revés, invertir secuencia (4 → 3 → 2 → 1).  
- Si vibra o se detiene, **aumentar `motor_delay`**.  
- Para más rapidez, **reducir `delay_ms()` o `motor_delay`**.  
- El LCD requiere pausas cortas para asegurar sincronización.  
- El multiplexado de los displays usa `delay_ms(1)` para evitar parpadeos.
- El PA4 en ocasiones puede venir con valor default 0x00, asegurarse limpiar puerto antes de configuración input/output.

---

### 💡 ¿Cómo ajustar la velocidad del motor?
En el bucle principal:
```c
if (motor_delay >= 2) {
    motor_delay = 0;
    activar_motor(&motor_change);
}
