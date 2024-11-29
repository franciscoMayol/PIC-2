import tkinter as tk
from tkinter import messagebox, simpledialog
from PIL import Image, ImageTk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import datetime
import serial
import threading

# Datos de stock y tiempo para cada módulo
stock_modulo1 = [10]
stock_modulo2 = [15]
peso_modulo1 = 0  # Variable para almacenar el peso en Módulo 1
peso_modulo2 = 0  # Variable para almacenar el peso en Módulo 2
peso_por_unidad1 = 500  # Peso por unidad del producto 1 en gramos
peso_por_unidad2 = 700  # Peso por unidad del producto 2 en gramos
tiempo_modulo1 = [datetime.datetime.now()]
tiempo_modulo2 = [datetime.datetime.now()]
producto_nombre1 = "Producto 1"
producto_nombre2 = "Producto 2"

# Configuración de la interfaz gráfica
root = tk.Tk()
root.title("Sistema de Gestión Pick to Light")
root.state('zoomed')  # Abrir en pantalla completa en Windows

# Salir del modo pantalla completa con 'Esc'
root.bind("<Escape>", lambda event: root.attributes("-fullscreen", False))

# Cargar la imagen de fondo original
background_image_original = Image.open("fondo.jpg")  # Ruta a la imagen de fondo

# Función para ajustar el tamaño de la imagen de fondo
def ajustar_imagen(event):
    width = root.winfo_width()
    height = root.winfo_height()
    background_image = background_image_original.resize((width, height), Image.LANCZOS)
    background_photo = ImageTk.PhotoImage(background_image)
    canvas_bg.create_image(0, 0, image=background_photo, anchor="nw")
    canvas_bg.background_photo = background_photo

# Canvas para el fondo, sin relleno adicional
canvas_bg = tk.Canvas(root, highlightthickness=0)
canvas_bg.pack(fill="both", expand=True)
root.bind("<Configure>", ajustar_imagen)



# Función para crear un marco con bordes redondeados
def create_rounded_frame(parent, x, y, width, height, radius=25):
    canvas = tk.Canvas(parent, width=width, height=height, highlightthickness=0, bg="white")
    canvas.place(x=x, y=y)
    canvas.create_arc((0, 0, radius*2, radius*2), start=90, extent=90, fill="white", outline="")
    canvas.create_arc((width-radius*2, 0, width, radius*2), start=0, extent=90, fill="white", outline="")
    canvas.create_arc((0, height-radius*2, radius*2, height), start=180, extent=90, fill="white", outline="")
    canvas.create_arc((width-radius*2, height-radius*2, width, height), start=270, extent=90, fill="white", outline="")
    canvas.create_rectangle((radius, 0, width-radius, height), fill="white", outline="")
    canvas.create_rectangle((0, radius, width, height-radius), fill="white", outline="")
    return canvas

# Función para abrir ventana emergente y editar el nombre del producto
def editar_producto(modulo):
    editar_ventana = tk.Toplevel(root)
    editar_ventana.title("Editar Producto")
    editar_ventana.geometry("300x200")

    tk.Label(editar_ventana, text="Nuevo Nombre:").pack(pady=5)
    nombre_entry = tk.Entry(editar_ventana)
    nombre_entry.pack(pady=5)

    tk.Label(editar_ventana, text="Nuevo Peso por Unidad (g):").pack(pady=5)
    peso_entry = tk.Entry(editar_ventana)
    peso_entry.pack(pady=5)

    def guardar_cambios():
        nuevo_nombre = nombre_entry.get()
        nuevo_peso = peso_entry.get()
        if nuevo_nombre:
            if modulo == 1:
                global producto_nombre1
                producto_nombre1 = nuevo_nombre
                producto_label1.config(text=f"Producto: {producto_nombre1}")
            elif modulo == 2:
                global producto_nombre2
                producto_nombre2 = nuevo_nombre
                producto_label2.config(text=f"Producto: {producto_nombre2}")
        if nuevo_peso:
            try:
                nuevo_peso = int(nuevo_peso)
                if modulo == 1:
                    global peso_por_unidad1
                    peso_por_unidad1 = nuevo_peso
                    peso_unidad_label1.config(text=f"Peso por unidad: {peso_por_unidad1} g")
                    actualizar_stock_por_peso(1)
                elif modulo == 2:
                    global peso_por_unidad2
                    peso_por_unidad2 = nuevo_peso
                    peso_unidad_label2.config(text=f"Peso por unidad: {peso_por_unidad2} g")
                    actualizar_stock_por_peso(2)
            except ValueError:
                messagebox.showerror("Error", "Por favor ingrese un valor numérico para el peso.")
        editar_ventana.destroy()

    guardar_btn = tk.Button(editar_ventana, text="Guardar", command=guardar_cambios)
    guardar_btn.pack(pady=10)

# Sección de Módulo 1 con bordes redondeados
frame_modulo1 = create_rounded_frame(canvas_bg, 330, 200, 300, 200)
producto_label1 = tk.Label(frame_modulo1, text=f"Producto: {producto_nombre1}", font=("Courier New", 16), bg="white", fg="red")
producto_label1.place(relx=0.5, rely=0.15, anchor="center")

stock_label1 = tk.Label(frame_modulo1, text=f"Stock: {stock_modulo1[-1]}", font=("Courier New", 18), bg="white")
stock_label1.place(relx=0.5, rely=0.35, anchor="center")

# Nuevo label para mostrar el peso en Módulo 1
peso_label1 = tk.Label(frame_modulo1, text=f"Peso: {peso_modulo1} g", font=("Courier New", 14), bg="white", fg="gray")
peso_label1.place(relx=0.5, rely=0.45, anchor="center")

# Label para mostrar el peso por unidad en Módulo 1
peso_unidad_label1 = tk.Label(frame_modulo1, text=f"Peso por unidad: {peso_por_unidad1} g", font=("Courier New", 14), bg="white", fg="gray")
peso_unidad_label1.place(relx=0.5, rely=0.75, anchor="center")

# Label para mostrar el stock por peso en Módulo 1
stock_por_peso_label1 = tk.Label(frame_modulo1, text=f"Stock por peso: 0", font=("Courier New", 18), bg="white")
stock_por_peso_label1.place(relx=0.5, rely=0.63, anchor="center")

# Mensajes de cambio de stock para Módulo 1
mensaje_modulo1 = tk.Label(root, text="", font=("Arial", 10), fg="blue")
mensaje_modulo1.place(relx=0.25, rely=0.55, anchor="center")

# Sección de verificación para Módulo 1
verificacion_frame1 = create_rounded_frame(canvas_bg, 330, 420, 300, 50)
verificacion_label1 = tk.Label(verificacion_frame1, text="Verificación: ", font=("Courier New", 16), bg="white")
verificacion_label1.place(relx=0.1, rely=0.5, anchor="w")
coincidencia_icono1 = tk.Label(verificacion_frame1, bg="white")

# Parámetro de desplazamiento para el ícono de verificación
desplazamiento_horizontal1 = 0.7  
coincidencia_icono1.place(relx=desplazamiento_horizontal1, rely=0.5, anchor="w")

btn_editar1 = tk.Button(frame_modulo1, text="Editar", command=lambda: editar_producto(1), bg="lightblue")
btn_editar1.place(relx=0.5, rely=0.9, anchor="center")

# Sección de Módulo 2 con bordes redondeados
frame_modulo2 = create_rounded_frame(canvas_bg, 1300, 200, 300, 200)
producto_label2 = tk.Label(frame_modulo2, text=f"Producto: {producto_nombre2}", font=("Courier New", 16), bg="white", fg="red")
producto_label2.place(relx=0.5, rely=0.15, anchor="center")

stock_label2 = tk.Label(frame_modulo2, text=f"Stock: {stock_modulo2[-1]}", font=("Courier New", 18), bg="white")
stock_label2.place(relx=0.5, rely=0.35, anchor="center")

# Nuevo label para mostrar el peso en Módulo 2
peso_label2 = tk.Label(frame_modulo2, text=f"Peso: {peso_modulo2} g", font=("Courier New", 14), bg="white", fg="gray")
peso_label2.place(relx=0.5, rely=0.45, anchor="center")

# Label para mostrar el peso por unidad en Módulo 2
peso_unidad_label2 = tk.Label(frame_modulo2, text=f"Peso por unidad: {peso_por_unidad2} g", font=("Courier New", 14), bg="white", fg="gray")
peso_unidad_label2.place(relx=0.5, rely=0.75, anchor="center")

# Label para mostrar el stock por peso en Módulo 2
stock_por_peso_label2 = tk.Label(frame_modulo2, text=f"Stock por peso: 0", font=("Courier New", 18), bg="white")
stock_por_peso_label2.place(relx=0.5, rely=0.63, anchor="center")

# Mensajes de cambio de stock para Módulo 2
mensaje_modulo2 = tk.Label(root, text="", font=("Arial", 10), fg="blue")
mensaje_modulo2.place(relx=0.75, rely=0.55, anchor="center")

# Sección de verificación para Módulo 2
verificacion_frame2 = create_rounded_frame(canvas_bg, 1300, 420, 300, 50)
verificacion_label2 = tk.Label(verificacion_frame2, text="Verificación: ", font=("Courier New", 16), bg="white")
verificacion_label2.place(relx=0.1, rely=0.5, anchor="w")
coincidencia_icono2 = tk.Label(verificacion_frame2, bg="white")

# Parámetro de desplazamiento para el ícono de verificación
desplazamiento_horizontal2 = 0.7  
coincidencia_icono2.place(relx=desplazamiento_horizontal2, rely=0.5, anchor="w")

btn_editar2 = tk.Button(frame_modulo2, text="Editar", command=lambda: editar_producto(2), bg="lightblue")
btn_editar2.place(relx=0.5, rely=0.9, anchor="center")

# Gráfica para Módulo 1
fig1 = Figure(figsize=(4, 2), dpi=100)
ax1 = fig1.add_subplot(111)
canvas1 = FigureCanvasTkAgg(fig1, master=canvas_bg)
canvas1.get_tk_widget().place(relx=0.1, rely=0.6, relwidth=0.3, relheight=0.36)

# Gráfica para Módulo 2
fig2 = Figure(figsize=(4, 2), dpi=100)
ax2 = fig2.add_subplot(111)
canvas2 = FigureCanvasTkAgg(fig2, master=canvas_bg)
canvas2.get_tk_widget().place(relx=0.60, rely=0.6, relwidth=0.3, relheight=0.36)

# Función de actualización de gráficos y stock
def actualizar_stock(modulo, cantidad):
    hora_actual = datetime.datetime.now().strftime("%H:%M:%S")
    if modulo == 1:
        stock_modulo1.append(cantidad)
        tiempo_modulo1.append(datetime.datetime.now())
        stock_label1.config(text=f"Stock: {cantidad}")
        peso_label1.config(text=f"Peso: {peso_modulo1} g")
        mensaje_modulo1.config(text=f"{hora_actual} - {producto_nombre1} ({cantidad:+})")
    elif modulo == 2:
        stock_modulo2.append(cantidad)
        tiempo_modulo2.append(datetime.datetime.now())
        stock_label2.config(text=f"Stock: {cantidad}")
        peso_label2.config(text=f"Peso: {peso_modulo2} g")
        mensaje_modulo2.config(text=f"{hora_actual} - {producto_nombre2} ({cantidad:+})")
    actualizar_grafica()
    actualizar_icono_coincidencia(modulo)

def actualizar_stock_por_peso(modulo):
    if modulo == 1 and peso_por_unidad1 > 0:
        stock_por_peso1 = peso_modulo1 // peso_por_unidad1
        stock_por_peso_label1.config(text=f"Stock por peso: {stock_por_peso1}")
        actualizar_icono_coincidencia(1)
    elif modulo == 2 and peso_por_unidad2 > 0:
        stock_por_peso2 = peso_modulo2 // peso_por_unidad2
        stock_por_peso_label2.config(text=f"Stock por peso: {stock_por_peso2}")
        actualizar_icono_coincidencia(2)

def actualizar_icono_coincidencia(modulo):
    if modulo == 1:
        if stock_modulo1[-1] == (peso_modulo1 // peso_por_unidad1):
            img = ImageTk.PhotoImage(Image.open("tick_verde.jpg").resize((50, 50)))
        else:
            img = ImageTk.PhotoImage(Image.open("cruz_roja.jpg").resize((50, 50)))
        coincidencia_icono1.config(image=img)
        coincidencia_icono1.image = img
    elif modulo == 2:
        if stock_modulo2[-1] == (peso_modulo2 // peso_por_unidad2):
            img = ImageTk.PhotoImage(Image.open("tick_verde.jpg").resize((50, 50)))
        else:
            img = ImageTk.PhotoImage(Image.open("cruz_roja.jpg").resize((50, 50)))
        coincidencia_icono2.config(image=img)
        coincidencia_icono2.image = img

def modificar_stock(modulo, entry, agregar=True):
    try:
        cantidad = int(entry.get())
        if not agregar:
            cantidad = -cantidad
        nuevo_stock = stock_modulo1[-1] + cantidad if modulo == 1 else stock_modulo2[-1] + cantidad
        if modulo == 1:
            actualizar_stock(1, nuevo_stock)
        else:
            actualizar_stock(2, nuevo_stock)
    except ValueError:
        messagebox.showerror("Error", "Por favor ingrese un número entero válido.")

def actualizar_grafica():
    ax1.clear()
    ax1.plot(tiempo_modulo1, stock_modulo1, label="Stock Módulo 1", color="blue")
    ax1.set_title("Stock en función del tiempo - Módulo 1")
    ax1.set_xlabel("Tiempo")
    ax1.set_ylabel("Cantidad")
    ax1.legend()

    ax2.clear()
    ax2.plot(tiempo_modulo2, stock_modulo2, label="Stock Módulo 2", color="green")
    ax2.set_title("Stock en función del tiempo - Módulo 2")
    ax2.set_xlabel("Tiempo")
    ax2.set_ylabel("Cantidad")
    ax2.legend()

    canvas1.draw()
    canvas2.draw()

# Configuración del puerto serial
try:
    ser = serial.Serial('COM3', 9600, timeout=1)  # Cambiar 'COM3' por el puerto adecuado
except:
    messagebox.showerror("Error", "No se pudo abrir el puerto serial. Verifique la conexión.")
    ser = None

# Función para leer datos del puerto serial
def leer_datos_serial():
    if ser is None:
        return

    while True:
        try:
            linea = ser.readline().decode('utf-8').strip()
            if linea.startswith("Stock 1"):
                valor = int(linea.split()[2])
                actualizar_stock(1, valor)
            elif linea.startswith("Stock 2"):
                valor = int(linea.split()[2])
                actualizar_stock(2, valor)
            elif linea.startswith("Peso 1"):
                valor = int(linea.split()[2])
                global peso_modulo1
                peso_modulo1 = valor
                peso_label1.config(text=f"Peso: {peso_modulo1} g")
                actualizar_stock_por_peso(1)
            elif linea.startswith("Peso 2"):
                valor = int(linea.split()[2])
                global peso_modulo2
                peso_modulo2 = valor
                peso_label2.config(text=f"Peso: {peso_modulo2} g")
                actualizar_stock_por_peso(2)
        except Exception as e:
            print(f"Error leyendo del puerto serial: {e}")

# Ejecutar la función de lectura del puerto serial en un hilo separado
hilo_serial = threading.Thread(target=leer_datos_serial, daemon=True)
hilo_serial.start()

# Inicializar gráfica
actualizar_grafica()

root.mainloop()


