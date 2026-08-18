#!/usr/bin/env python3
"""
=============================================================================
Voice Clear AI — HWID Machine-Locked Product Key Generator & Validator
Autor: Byron J. P
=============================================================================
Este script genera y valida claves de producto únicas bloqueadas por hardware
(Machine ID) para el instalador de Voice Clear AI.
"""

import sys
import random
import argparse
import platform

# Conjunto seguro de 32 caracteres (sin 0/O ni 1/I para evitar confusiones al escribir)
CHARSET = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ"
CHAR_TO_VAL = {ch: idx for idx, ch in enumerate(CHARSET)}

# Coeficientes polinomiales y salts secretos
WEIGHTS_1 = [3, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43]
WEIGHTS_2 = [47, 43, 37, 31, 29, 23, 19, 17, 13, 11, 7, 3]
SALT_1 = 789
SALT_2 = 456

def get_local_machine_guid() -> str:
    """Obtiene el MachineGuid de Windows desde el registro del sistema."""
    if platform.system() == "Windows":
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\Cryptography") as key:
                guid, _ = winreg.QueryValueEx(key, "MachineGuid")
                return guid
        except Exception:
            pass
    return platform.node() + "VOICECLEAR"

def compute_machine_id(guid_or_name: str) -> str:
    """Convierte un MachineGuid o nombre de equipo en un Machine ID seguro de 8 caracteres (XXXX-YYYY)."""
    cleaned = guid_or_name.upper().replace("-", "").replace("{", "").replace("}", "").replace(" ", "")
    sum1 = 12345
    sum2 = 67890
    for ch in cleaned:
        val = ord(ch)
        sum1 = (sum1 * 31 + val) % 1048576
        sum2 = (sum2 * 37 + val) % 1048576

    d0 = (sum1 // 32768) % 32
    d1 = (sum1 // 1024) % 32
    d2 = (sum1 // 32) % 32
    d3 = sum1 % 32

    d4 = (sum2 // 32768) % 32
    d5 = (sum2 // 1024) % 32
    d6 = (sum2 // 32) % 32
    d7 = sum2 % 32

    return f"{CHARSET[d0]}{CHARSET[d1]}{CHARSET[d2]}{CHARSET[d3]}-{CHARSET[d4]}{CHARSET[d5]}{CHARSET[d6]}{CHARSET[d7]}"

def compute_checksum(body_12: str) -> str:
    """Calcula los 4 caracteres de checksum a partir de los 12 caracteres del cuerpo."""
    if len(body_12) != 12:
        raise ValueError("El cuerpo de la clave debe tener exactamente 12 caracteres.")
    
    sum1 = SALT_1
    sum2 = SALT_2
    
    for i, ch in enumerate(body_12.upper()):
        if ch not in CHAR_TO_VAL:
            raise ValueError(f"Carácter inválido en la clave: '{ch}'")
        val = CHAR_TO_VAL[ch] + 1
        sum1 += val * WEIGHTS_1[i]
        sum2 += val * WEIGHTS_2[i]
        
    h1 = sum1 % 1024
    h2 = sum2 % 1024
    
    d0 = h1 // 32
    d1 = h1 % 32
    d2 = h2 // 32
    d3 = h2 % 32
    
    return CHARSET[d0] + CHARSET[d1] + CHARSET[d2] + CHARSET[d3]

def generate_key_for_hwid(hwid: str, prefix: bool = True) -> str:
    """Genera una clave de activación bloqueada a un Machine ID específico."""
    clean_hwid = hwid.upper().strip().replace(" ", "").replace("-", "").replace("VC", "")
    if len(clean_hwid) != 8:
        raise ValueError(f"El Machine ID debe tener 8 caracteres (recibido: '{hwid}')")
        
    for ch in clean_hwid:
        if ch not in CHAR_TO_VAL:
            raise ValueError(f"Carácter inválido en Machine ID: '{ch}'")

    # 4 caracteres de token de seguridad aleatorio
    token = "".join(random.choice(CHARSET) for _ in range(4))
    body = clean_hwid + token
    chk = compute_checksum(body)

    b1 = clean_hwid[0:4]
    b2 = clean_hwid[4:8]
    b3 = token
    b4 = chk

    key = f"{b1}-{b2}-{b3}-{b4}"
    if prefix:
        return f"VCAI-{key}"
    return key

def validate_key(raw_key: str, expected_hwid: str = None) -> tuple[bool, str, str]:
    """
    Valida una clave.
    Retorna (is_valid, machine_id_asociado, mensaje_error).
    """
    cleaned = raw_key.upper().strip().replace(" ", "").replace("-", "")
    if cleaned.startswith("VCAI"):
        cleaned = cleaned[4:]
        
    if len(cleaned) != 16:
        return False, "", "Longitud de clave incorrecta (debe tener 16 caracteres)."
        
    for ch in cleaned:
        if ch not in CHAR_TO_VAL:
            return False, "", f"Carácter no permitido: '{ch}'"
            
    body = cleaned[0:12]
    chk_given = cleaned[12:16]
    key_hwid = f"{cleaned[0:4]}-{cleaned[4:8]}"
    
    try:
        chk_expected = compute_checksum(body)
        if chk_given != chk_expected:
            return False, key_hwid, "Checksum criptográfico inválido (clave falsa o alterada)."
    except Exception as e:
        return False, key_hwid, str(e)
        
    if expected_hwid:
        clean_exp_hwid = expected_hwid.upper().strip().replace(" ", "").replace("-", "").replace("VC", "")
        clean_key_hwid = key_hwid.replace("-", "")
        if clean_key_hwid != clean_exp_hwid:
            return False, key_hwid, f"La clave pertenece al Machine ID '{key_hwid}', pero este equipo es '{expected_hwid}'."
            
    return True, key_hwid, "Clave válida y auténtica."

def main():
    parser = argparse.ArgumentParser(description="Voice Clear AI - Generador de Claves Únicas por Máquina (HWID)")
    parser.add_argument("--hwid", type=str, default=None, help="Machine ID del usuario (ej: 8F3A-92B1)")
    parser.add_argument("--local", action="store_true", help="Generar clave para la computadora actual")
    parser.add_argument("-v", "--validate", type=str, default=None, help="Validar una clave existente")
    parser.add_argument("--for-hwid", type=str, default=None, help="Validar si la clave coincide con este HWID")
    parser.add_argument("-n", "--count", type=int, default=1, help="Número de claves a generar")
    
    args = parser.parse_args()
    
    local_guid = get_local_machine_guid()
    local_hwid = compute_machine_id(local_guid)
    
    if args.validate:
        is_valid, key_hwid, msg = validate_key(args.validate, expected_hwid=args.for_hwid)
        if is_valid:
            print(f"[OK] {msg}")
            print(f" -> Bloqueada para Machine ID: {key_hwid}")
            sys.exit(0)
        else:
            print(f"[ERROR] Clave Inválida: {msg}")
            sys.exit(1)
            
    target_hwid = args.hwid
    if args.local or not target_hwid:
        if not target_hwid:
            print(f"[*] Machine ID de este equipo detectado: {local_hwid}")
            target_hwid = local_hwid

    print(f"\n=== Generador de Claves de Activación Voice Clear AI (Autor: Byron J. P) ===")
    print(f"Equipo destino (Machine ID): {target_hwid}\n")
    for i in range(args.count):
        key = generate_key_for_hwid(target_hwid)
        print(f"[{i+1:02d}] {key}")

if __name__ == "__main__":
    main()
