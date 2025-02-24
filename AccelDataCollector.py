# The necessary imports for this Python file include:
import os  # For file path operations
import time  # For time-related functions if needed in data collection
import csv
import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import messagebox
import threading

baud_rate = 19200

class AccelDataCollector:
    def __init__(self):
        print("Starting AccelDataCollector...")
        self.root = tk.Tk()
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)  # Handle window closing
        self.root.title("Accelerometer Data Collector")
        self.root.geometry("300x150")
        
        # Add initial status label
        self.status_label = tk.Label(self.root, text="Connecting to device...", fg="blue")
        self.status_label.pack(pady=20)
        self.root.update()
        
        self.setup_serial_connection()

    def setup_serial_connection(self):
        print("Selecting COM port...")
        self.status_label.config(text="Waiting for COM port selection...")
        self.root.update()
        
        self.com_port = self.select_com_port()
        if not self.com_port:
            print("No COM port was selected")
            self.status_label.config(text="No COM port selected", fg="red")
            return
            
        print(f"Attempting to connect to {self.com_port}")
        self.status_label.config(text=f"Connecting to {self.com_port}...", fg="blue")
        self.root.update()
        
        try:
            self.ser = serial.Serial(self.com_port, baud_rate, timeout=5)
            time.sleep(2)  # Wait for connection to establish
            print("COM port connected successfully")
            self.setup_main_window()
        except serial.SerialException as e:
            print(f"Serial connection failed: {e}")
            self.status_label.config(text=f"Connection failed: {e}", fg="red")
            self.root.update()
            retry = messagebox.askretrycancel("Connection Error", 
                f"Failed to connect to {self.com_port}: {e}\nWould you like to try again?")
            if retry:
                self.setup_serial_connection()
            return

    def setup_main_window(self):
        # Clear any existing widgets
        for widget in self.root.winfo_children():
            widget.destroy()
            
        self.is_collecting = False
        self.collected_data = []
        
        # Create and pack buttons
        self.start_button = tk.Button(self.root, text="Start", command=self.start_collection)
        self.start_button.pack(pady=10)
        
        self.pause_button = tk.Button(self.root, text="Pause", command=self.pause_collection)
        self.pause_button.pack(pady=10)
        self.pause_button['state'] = 'disabled'
        
        # Add a reconnect button
        self.reconnect_button = tk.Button(self.root, text="Reconnect", command=self.setup_serial_connection)
        self.reconnect_button.pack(pady=10)

    def on_closing(self):
        if hasattr(self, 'ser') and self.ser.is_open:
            self.ser.close()
        self.root.destroy()

    def select_com_port(self):
        ports = serial.tools.list_ports.comports()
        available_ports = [port.device for port in ports]

        print(f"Available ports: {available_ports}")
        if not available_ports:
            print("No COM ports found")
            messagebox.showerror("Error", "No COM ports available")
            return None

        # Create a separate window for port selection
        select_window = tk.Toplevel(self.root)
        select_window.title("Select COM Port")
        select_window.transient(self.root)
        select_window.grab_set()
        
        # Create a frame to hold the widgets
        frame = tk.Frame(select_window, padx=20, pady=20)
        frame.pack(expand=True, fill='both')
        
        tk.Label(frame, text="Select a COM Port:", font=('Arial', 12)).pack(pady=10)
        
        # Create buttons with better sizing
        for port in available_ports:
            btn = tk.Button(frame, 
                          text=port,
                          command=lambda p=port: self.set_port(p, select_window),
                          width=20,  # Set fixed width for buttons
                          height=2)  # Set fixed height for buttons
            btn.pack(pady=5)
            
        self.selected_port = None
        
        # Force the window to calculate its required size
        select_window.update_idletasks()
        
        # Center the window on screen
        width = max(300, select_window.winfo_reqwidth())  # Minimum width of 300
        height = max(200, select_window.winfo_reqheight())  # Minimum height of 200
        x = (select_window.winfo_screenwidth() // 2) - (width // 2)
        y = (select_window.winfo_screenheight() // 2) - (height // 2)
        select_window.geometry(f'{width}x{height}+{x}+{y}')
        
        self.root.wait_window(select_window)
        return self.selected_port

    def set_port(self, port, window):
        self.selected_port = port
        window.destroy()

    def send_to_arduino(self, command):
        try:
            self.ser.write(command.encode())
        except serial.SerialException as e:
            messagebox.showerror("Error", f"Serial Error: {e}")

    def collect_data_thread(self):
        while self.is_collecting:
            try:
                line = self.ser.readline().decode('utf-8').strip()
                if line:
                    timestamp = time.strftime('%Y-%m-%d %H:%M:%S') + f".{int(time.time() * 1000) % 1000:03d}"  # mS precision
                    print(f"{timestamp} - {line}")
                    self.collected_data.append([timestamp] + line.split('\t'))
            except:
                pass

    def start_collection(self):
        self.is_collecting = True
        self.send_to_arduino('s')
        self.start_button['state'] = 'disabled'
        self.pause_button['state'] = 'normal'
        # Start collection in a separate thread
        self.collection_thread = threading.Thread(target=self.collect_data_thread)
        self.collection_thread.start()

    def pause_collection(self):
        self.is_collecting = False
        self.send_to_arduino('p')
        self.start_button['state'] = 'normal'
        self.pause_button['state'] = 'disabled'
        filename = self.get_save_filename()
        if filename:
            self.save_data_to_csv(filename)
        self.collected_data = []  # Clear the data after saving

    def get_save_filename(self):
        # Create a dialog for filename input
        dialog = tk.Toplevel(self.root)
        dialog.title("Save Data")
        dialog.geometry("300x150")
        dialog.transient(self.root)
        dialog.grab_set()

        tk.Label(dialog, text="Enter filename (without .csv):", pady=10).pack()
        entry = tk.Entry(dialog, width=30)
        entry.pack(pady=10)
        entry.insert(0, "collected_data")
        entry.select_range(0, 'end')  # Select default text for easy replacement
        
        filename_var = [None]  # Use list to store filename from button command
        
        def on_ok():
            base_name = entry.get().strip()
            if base_name:
                filename_var[0] = self.get_unique_filename(base_name)
                dialog.destroy()
            else:
                messagebox.showwarning("Invalid Name", "Please enter a filename")
                
        def on_cancel():
            dialog.destroy()

        btn_frame = tk.Frame(dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

        # Center the dialog
        dialog.update_idletasks()
        width = dialog.winfo_width()
        height = dialog.winfo_height()
        x = (dialog.winfo_screenwidth() // 2) - (width // 2)
        y = (dialog.winfo_screenheight() // 2) - (height // 2)
        dialog.geometry(f'{width}x{height}+{x}+{y}')
        
        dialog.wait_window()
        return filename_var[0]

    def get_unique_filename(self, base_name):
        os.makedirs('./AccelData', exist_ok=True)
        counter = 0
        while True:
            if counter == 0:
                filename = f'./AccelData/{base_name}.csv'
            else:
                filename = f'./AccelData/{base_name}_{counter}.csv'
            
            if not os.path.exists(filename):
                return filename
            counter += 1

    def save_data_to_csv(self, filename):
        with open(filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(['Time', 'Accel1_X [m/s^2]', 'Accel1_Y [m/s^2]', 'Accel1_Z [m/s^2]', 
                               'Rotation1_X [rad/s]', 'Rotation1_Y [rad/s]', 'Rotation1_Z [rad/s]', 
                               'Accel2_X [m/s^2]', 'Accel2_Y [m/s^2]', 'Accel2_Z [m/s^2]', 
                               'Rotation2_X [rad/s]', 'Rotation2_Y [rad/s]', 'Rotation2_Z [rad/s]'])
            writer.writerows(self.collected_data)
        print(f"Data saved to {filename}")

    def run(self):
        self.root.mainloop()

    def __del__(self):
        if hasattr(self, 'ser') and self.ser.is_open:
            self.ser.close()

if __name__ == "__main__":
    try:
        print("Creating application...")
        app = AccelDataCollector()
        if hasattr(app, 'root'):  # Only run if initialization was successful
            print("Running application...")
            app.run()
    except Exception as e:
        print(f"Application failed with error: {e}")