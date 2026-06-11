import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import os

class SerialLoader:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("LaunchPad Loader")
        self.root.geometry("600x400")
        
        self.ser = None
        self.running = False
        
        self.create_widgets()
        
    def create_widgets(self):
        # Port selection
        tk.Label(self.root, text="Porta Serial:").pack(pady=5)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(self.root, textvariable=self.port_var, width=30)
        self.port_combo.pack()
        ttk.Button(self.root, text="Atualizar Portas", command=self.update_ports).pack(pady=5)
        
        # Baudrate
        tk.Label(self.root, text="Velocidade (Baud):").pack(pady=5)
        self.baud_var = tk.StringVar(value="921600")
        self.baud_combo = ttk.Combobox(self.root, textvariable=self.baud_var, 
                                      values=["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"])
        self.baud_combo.pack()
        
        # File selection
        tk.Label(self.root, text="Arquivo:").pack(pady=5)
        self.file_var = tk.StringVar()
        tk.Entry(self.root, textvariable=self.file_var, width=50).pack()
        ttk.Button(self.root, text="Selecionar Arquivo", command=self.select_file).pack(pady=5)
        
        # Start Button
        self.start_btn = ttk.Button(self.root, text="START", command=self.start_loader)
        self.start_btn.pack(pady=20)
        
        # Progress
        tk.Label(self.root, text="Progresso:").pack(pady=5)
        self.progress = ttk.Progressbar(self.root, length=500, mode='determinate')
        self.progress.pack(pady=10)
        
        self.status_label = tk.Label(self.root, text="Pronto", fg="blue")
        self.status_label.pack(pady=10)
        
        self.update_ports()
        
    def update_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_var.set(ports[0])
    
    def select_file(self):
        filename = filedialog.askopenfilename(filetypes=[("Binários", "*.bin"), ("Todos", "*.*")])
        if filename:
            self.file_var.set(filename)
    
    def log(self, msg):
        print(f"[LOG] {msg}")
        self.status_label.config(text=msg)
        self.root.update()
    
    def calculate_checksum(self, data_chunk):
        """Soma simples dos bytes com wrap around (uint8)"""
        checksum = 0
        for b in data_chunk:
            checksum = (checksum + b) & 0xFF
        return checksum
    
    def send_dl_command(self):
        self.log("Enviando comando DL...")
        self.ser.write(b"D")
        self.ser.flush()
        
        # Espera resposta "OK" por até 60 segundos
        timeout = time.time() + 60
        response = b""
        while time.time() < timeout:
            if self.ser.in_waiting:
                chunk = self.ser.read(self.ser.in_waiting)
                response += chunk
                if b"O" in response:
                    self.log("Resposta OK recebida!")
                    return True
            time.sleep(0.1)
        self.log("Timeout aguardando OK")
        return False
    
    def send_chunk(self, chunk, is_last):
        if len(chunk) != 256:
            # Preenche com zeros se necessário (último pacote)
            chunk = chunk + b'\x00' * (256 - len(chunk))
        
        checksum = self.calculate_checksum(chunk)
        inv_checksum = (~checksum) & 0xFF
        control = 0x00 if is_last else 0x60
        
        packet = chunk + bytes([checksum, inv_checksum, control])
        
        self.ser.write(packet)
        self.ser.flush()
        return True
    
    def loader_thread(self):
        try:
            port = self.port_var.get()
            baud = int(self.baud_var.get())
            filepath = self.file_var.get()
            
            if not port or not filepath:
                messagebox.showerror("Erro", "Selecione porta e arquivo!")
                return
            
            self.log("Abrindo porta serial...")
            self.ser = serial.Serial(port, baud, timeout=1)
            time.sleep(1)  # Estabilização
            
            with open(filepath, "rb") as f:
                data = f.read()
            
            total_size = len(data)
            self.log(f"Arquivo carregado: {total_size} bytes")
            
            if not self.send_dl_command():
                return
            
            self.progress['maximum'] = total_size
            offset = 0
            packet_count = 0
            
            while offset < total_size:
                if not self.running:
                    break
                    
                chunk_size = min(256, total_size - offset)
                chunk = data[offset:offset + chunk_size]
                is_last = (offset + chunk_size) >= total_size
                
                self.log(f"Enviando pacote {packet_count} ({len(chunk)} bytes)")
                self.send_chunk(chunk, is_last)
                
                # Espera OK
                timeout = time.time() + 10  # 10s por pacote
                ok_received = False
                while time.time() < timeout:
                    if self.ser.in_waiting:
                        resp = self.ser.read(self.ser.in_waiting)
                        if b"O" in resp:
                            ok_received = True
                            break
                    time.sleep(0.0005)
                
                if not ok_received:
                    self.log("Timeout aguardando OK do pacote")
                    break
                
                offset += chunk_size
                self.progress['value'] = offset
                packet_count += 1
                self.root.update()
            
            if offset >= total_size:
                self.log("Upload concluído com sucesso!")
            else:
                self.log("Upload interrompido")
                
        except Exception as e:
            self.log(f"Erro: {str(e)}")
            messagebox.showerror("Erro", str(e))
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()
            self.running = False
            self.start_btn.config(state='normal')
    
    def start_loader(self):
        if self.running:
            return
        self.running = True
        self.start_btn.config(state='disabled')
        self.progress['value'] = 0
        threading.Thread(target=self.loader_thread, daemon=True).start()
    
    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    app = SerialLoader()
    app.run()