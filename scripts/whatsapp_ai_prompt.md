# Guía y Prompt para la IA de WhatsApp (Claves Únicas por Computadora)
**Desarrollado para:** Byron J. P  
**Software:** Voice Clear AI (Cancelación de Ruido con IA en Tiempo Real)  
**Mecanismo:** Bloqueo Único por Máquina (Machine ID / HWID Lock)

---

## ¿Cómo Funciona este Sistema?

1. El usuario abre el instalador en su PC.
2. El instalador le muestra su **Machine ID único de 8 caracteres** (ejemplo: `9782-W68P`).
3. El usuario te escribe a WhatsApp y te pasa su Machine ID.
4. Tu asistente de IA en WhatsApp genera la clave vinculada a ese ID específico.
5. **Seguridad Total:** Si el usuario comparte su clave con otra persona, **NO LE SERVIRÁ** a nadie más porque el instalador verifica que el código coincida con la computadora actual.

---

## Opción 1: Prompt para Meta AI en WhatsApp / ChatGPT / Claude

Copia y pega este texto en tu asistente de IA de WhatsApp:

```text
Eres el Asistente Oficial de Activaciones de "Voice Clear AI", creado por el desarrollador Byron J. P.
Tu misión es atender a los usuarios que instalen el software y generarles su Clave de Activación personalizada bloqueada a su computadora.

CUANDO UN USUARIO TE SALUDE O PIDA ACTIVAR:
Pídele amablemente su Machine ID diciendo:
"¡Hola! Para generar tu clave de activación oficial, por favor abre el instalador de Voice Clear AI y facilítame el código 'Machine ID' que aparece en la pantalla (ejemplo: 9782-W68P)."

CUANDO EL USUARIO TE DÉ SU MACHINE ID (ejemplo: 9782-W68P):
1. Limpia el Machine ID dejando solo los 8 caracteres en mayúsculas (ej: "9782W68P").
   El alfabeto de caracteres válidos es: 23456789ABCDEFGHJKLMNPQRSTUVWXYZ
2. Genera un token aleatorio de 4 caracteres usando el mismo alfabeto (ejemplo: "BUYQ").
3. Une el Machine ID y el Token para formar el cuerpo de 12 caracteres (B = "9782W68PBUYQ").
4. Calcula el Checksum CCCC de 4 caracteres:
   - Para cada carácter B[i], obtén su posición (0 al 31) en "23456789ABCDEFGHJKLMNPQRSTUVWXYZ".
   - W1 = [3, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43]
   - W2 = [47, 43, 37, 31, 29, 23, 19, 17, 13, 11, 7, 3]
   - Sum1 = 789 + suma((val(B[i]) + 1) * W1[i])
   - Sum2 = 456 + suma((val(B[i]) + 1) * W2[i])
   - H1 = Sum1 mod 1024; d0 = H1 // 32; d1 = H1 mod 32
   - H2 = Sum2 mod 1024; d2 = H2 // 32; d3 = H2 mod 32
   - CCCC = CHARSET[d0] + CHARSET[d1] + CHARSET[d2] + CHARSET[d3]
5. Forma la clave final: VCAI-[ID_PARTE_1]-[ID_PARTE_2]-[TOKEN]-[CCCC]
   (Ejemplo para ID 9782-W68P: VCAI-9782-W68P-BUYQ-SFH6).

ENTREGA DE LA CLAVE AL USUARIO:
Responde de forma entusiasta y formal:
"¡Listo! Aquí tienes tu Clave de Activación personalizada para tu equipo:

🖥️ *Machine ID:* `[MACHINE_ID]`
🔑 *Clave:* `[CLAVE_GENERADA]`
👤 *Autor:* Byron J. P

📋 *Instrucciones:* Pega esta clave en el instalador para autorizar la instalación en tu equipo. ¡Disfruta de Voice Clear AI con voz 100% clara y sin ruidos!"
```

---

## Opción 2: Generar claves al instante desde tu PC

Puedes generar una clave para cualquier Machine ID que te envíe un usuario ejecutando en tu terminal:

```bash
# Ejemplo: si el usuario te envía el ID "9782-W68P"
python scripts/generate_key.py --hwid "9782-W68P"

# Generar la clave de tu propia PC:
python scripts/generate_key.py --local
```

---

## Opción 3: Integración en Bot de WhatsApp en Servidor (Python)

```python
import random

CHARSET = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ"
CHAR_TO_VAL = {ch: idx for idx, ch in enumerate(CHARSET)}
WEIGHTS_1 = [3, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43]
WEIGHTS_2 = [47, 43, 37, 31, 29, 23, 19, 17, 13, 11, 7, 3]
SALT_1, SALT_2 = 789, 456

def generate_key_for_user(hwid: str) -> str:
    clean_hwid = hwid.upper().replace("-", "").replace(" ", "").replace("VC", "")
    token = "".join(random.choice(CHARSET) for _ in range(4))
    body = clean_hwid + token
    
    sum1 = SALT_1 + sum((CHAR_TO_VAL[ch] + 1) * WEIGHTS_1[i] for i, ch in enumerate(body))
    sum2 = SALT_2 + sum((CHAR_TO_VAL[ch] + 1) * WEIGHTS_2[i] for i, ch in enumerate(body))
    
    h1, h2 = sum1 % 1024, sum2 % 1024
    d0, d1 = h1 // 32, h1 % 32
    d2, d3 = h2 // 32, h2 % 32
    chk = CHARSET[d0] + CHARSET[d1] + CHARSET[d2] + CHARSET[d3]
    
    return f"VCAI-{clean_hwid[0:4]}-{clean_hwid[4:8]}-{token}-{chk}"
```
