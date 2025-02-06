import cv2

# Iniciar la captura de video desde la cámara web (0 es la cámara predeterminada)
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("No se pudo abrir la cámara")
    exit()

while True:
    # Capturar frame por frame
    ret, frame = cap.read()

    # Si el frame se capturó correctamente, ret es True
    if not ret:
        print("No se pudo recibir el frame. Saliendo...")
        break

    # Mostrar el frame en una ventana
    cv2.imshow('Camara Web', frame)

    # Esperar 1 milisegundo y verificar si se presionó la tecla 'q'
    if cv2.waitKey(1) == ord('q'):
        break

# Liberar la cámara y cerrar todas las ventanas
cap.release()
cv2.destroyAllWindows()
