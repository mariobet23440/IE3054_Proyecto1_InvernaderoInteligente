
SLA+R = Slave Address + Read
SLA+W = Slave Address + Write

PASOS PARA LLEVAR A CABO UNA COMUNICACIÓN I2C
1. Generar una condición START 
2. Cuando la condición START se haya transmitido, TWINT en TWCR 
   se pondrá en set. - MasterStart() o MasterRepeatedStart()
3. Enviar address de esclavo SLA+W/R - MasterTransmitter(slave_address)
4. Enviar dato a transmitir - MasterTransmitter(data)

MASTER TRANSMITTER (MT) MODE
- PARA ENTRAR AL MODO: Se envia START -> (SLA+W)
1. Enviar condición START
2. Escribir SLA+W en TWDR
3. Cuando SLA+W se haya transmitido, se recibirá un ACK y se observarán
   códigos de estado (0x18, 0x20 o 0x38)
4. Enviar paquete de datos (Escribir byte de datos en TWDR). Esto
   solo se puede hacer cuando TWINT está en alto.
5. Después de actualizar TWDR la bandera TWINT debería bajarse
   (Escribiendo un cero)
6. Para continuar la transferencia, se levanta TWINT.
7. Repetir hasta enviar el último byte de datos
8. Generar un STOP (Activando TWSTO)

MASTER RECEIVER MODE (MR)
- PARA ENTRAR AL MODO: Se envía START -> (SLA+R)
1. Enviar START
2. Escribir SLA+R en TWDR
3. Reiniciar TWCR (TWINT)
4. Recibir datos
5. Terminar transmisión con una condición STPO