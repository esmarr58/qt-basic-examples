#!/usr/bin/env python3
"""Simulador del firmware del TC05 (ESP32-S3 + SG90) para probar la app de Qt sin hardware.

Responde igual que firmware_tc05_servo.ino:
  al conectar          -> {"tipo":"saludo","mensaje":"Servidor TC05 listo","puerto":81,"angulo":n}
  {"tipo":"servo","angulo":a} -> recorta a 0..180 y manda {"tipo":"servo_ack","angulo":n} a TODOS
  {"tipo":"barrido"}   -> 0 -> 180 -> 0 de 5 en 5 grados cada 40 ms (un servo_ack por paso)
  {"tipo":"consulta"}  -> servo_ack con el angulo actual
  otro / JSON invalido -> {"tipo":"error","mensaje":"..."} solo a ese cliente
  cada 100 ms          -> {"tipo":"pot","valor":0..4095,"voltaje":v} a TODOS (potenciometro simulado:
                          una onda lenta de 0 a 3.3 V con un poco de ruido, como si alguien girara la perilla)
Uso: python3 simulador_esp32.py [puerto]   (por defecto 8181; el 81 requiere sudo)
"""
import asyncio
import json
import math
import random
import sys
import time

import websockets

PUERTO = int(sys.argv[1]) if len(sys.argv) > 1 else 8181
clientes = set()
estado = {"angulo": 90, "barrido": None}


def recortar(a):
    return max(0, min(180, a))


async def avisar():
    texto = json.dumps({"tipo": "servo_ack", "angulo": estado["angulo"]}, separators=(",", ":"))
    for c in list(clientes):
        try:
            await c.send(texto)
        except Exception:
            pass


async def barrido():
    estado["angulo"] = 0
    await avisar()
    direccion = 5
    while True:
        await asyncio.sleep(0.04)
        siguiente = estado["angulo"] + direccion
        if siguiente >= 180:
            siguiente, direccion = 180, -5
        if siguiente <= 0 and direccion < 0:
            estado["angulo"] = 0
            await avisar()
            print("Barrido terminado", flush=True)
            return
        estado["angulo"] = siguiente
        await avisar()


async def error(ws, mensaje):
    print("Error:", mensaje, flush=True)
    await ws.send(json.dumps({"tipo": "error", "mensaje": mensaje}, separators=(",", ":")))


async def atender(ws, path=None):
    clientes.add(ws)
    print("Cliente conectado:", ws.remote_address[0], flush=True)
    await ws.send(json.dumps({"tipo": "saludo", "mensaje": "Servidor TC05 listo", "puerto": 81,
                              "angulo": estado["angulo"]}, separators=(",", ":")))
    try:
        async for texto in ws:
            print("Recibido:", texto, flush=True)
            try:
                doc = json.loads(texto)
                assert isinstance(doc, dict)
            except Exception:
                await error(ws, "JSON invalido")
                continue
            tipo = doc.get("tipo")
            if tipo is None:
                await error(ws, 'Falta el campo "tipo"')
            elif tipo == "servo":
                a = doc.get("angulo")
                if not isinstance(a, (int, float)) or isinstance(a, bool):
                    await error(ws, 'Falta el campo numerico "angulo" (0..180)')
                    continue
                if estado["barrido"]:
                    estado["barrido"].cancel()
                    estado["barrido"] = None
                estado["angulo"] = recortar(round(a))
                await avisar()
            elif tipo == "barrido":
                if estado["barrido"]:
                    estado["barrido"].cancel()
                estado["barrido"] = asyncio.create_task(barrido())
            elif tipo == "consulta":
                await avisar()
            else:
                await error(ws, "Tipo de comando desconocido")
    finally:
        clientes.discard(ws)
        print("Cliente desconectado", flush=True)


async def potenciometro():
    t0 = time.monotonic()
    while True:
        await asyncio.sleep(0.1)
        if not clientes:
            continue
        t = time.monotonic() - t0
        v = 1.65 + 1.5 * math.sin(2 * math.pi * t / 8) + random.uniform(-0.03, 0.03)
        v = max(0.0, min(3.3, v))
        lectura = round(v / 3.3 * 4095)
        texto = json.dumps({"tipo": "pot", "valor": lectura, "voltaje": round(lectura * 3.3 / 4095, 2)},
                           separators=(",", ":"))
        for c in list(clientes):
            try:
                await c.send(texto)
            except Exception:
                pass


async def main():
    asyncio.create_task(potenciometro())
    async with websockets.serve(atender, "0.0.0.0", PUERTO):
        print(f"Simulador TC05 escuchando en ws://0.0.0.0:{PUERTO}/", flush=True)
        await asyncio.Future()


asyncio.run(main())
