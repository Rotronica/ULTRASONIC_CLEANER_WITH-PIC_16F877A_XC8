# **Código de remplazo para un limpiador ultrasónico utilizando un microcontrolador PIC16F877A**

## Se tiene el siguiente circuito:
<img src="https://raw.githubusercontent.com/Rotronica/ULTRASONIC_CLEANER_WITH-PIC_16F877A_XC8/refs/heads/master/CIRCUITO.png" width="2000" height="2000" />

## Como descargar el proyecto:
1. Ir a la rectangulo verde donde dice "<>code" desplegar la lista y hacer click en Download ZIP(Recomendado)
2. Puede clonar el repositorio utilizando git con el comando:
`git clone https://github.com/Rotronica/ULTRASONIC_CLEANER_WITH-PIC_16F877A_XC8.git`

## Código fuente: 
[Click aquí para ver el Código_fuente](https://github.com/Rotronica/ULTRASONIC_CLEANER_WITH-PIC_16F877A_XC8/blob/master/Codigo_fuente.c)
## Funcionamiento:
1. Cuando se energiza el circuito pasa a un estado inicial de espera.
2. Puede escoger la potencia que quiera ya sea 35W(P35) o 50W(P50).
3. Una vez que se escoge la potencia se puede configurar el tiempo de temporización con los botones de 35W y 50W.
4. Una vez seleccionado la potencia y configurado la temporización se procede a presionar el boton de START y el temporizador empezara a decrementar hasta llegar a 000, oprimir el boton reset para iniciar de nuevo(el boton reset es la configuración por defecto que trae el microcontrolador segun hoja de datos por el pin uno con un configuración pull-down con una resistencia de 1k o 10k ohms).
---
### Para ver el funcionamiento del la multiplexación ingresar:
* [Multiplexación con buffer para caracteres](https://github.com/Rotronica/PIC_16F877A_BUFFER_MUX_MS_XC8.git)
### Para ver el funcionamiento en físico
* [Pruebas en protoboard(Video)](https://www.tiktok.com/@rodtronica/video/7550036934698011910?is_from_webapp=1&sender_device=pc&web_id=7546430987099768376)

---

## Autor

- [Rodrigo C.C](https://github.com/Rotronica)  
