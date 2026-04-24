"""
Arduino Advanced Calculator - PC Interface
Main GUI Application using Tkinter
"""

import tkinter as tk
from tkinter import ttk, messagebox
import sys
import os

# Add current directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from themes import THEME, FONTS, BUTTON_STYLES
from calc_engine import CalcEngine, Complex, Matrix
from serial_comm import SerialComm


class CalculatorGUI:
    """Main calculator GUI application"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Arduino Advanced Calculator v2.0")
        self.root.geometry("700x650")
        self.root.configure(bg=THEME['bg'])
        self.root.resizable(True, True)
        
        # Initialize engines
        self.calc = CalcEngine()
        self.serial = SerialComm()
        self.use_hardware = False
        
        # State
        self.current_input = ""
        self.last_result = None
        self.fn_state = 0  # 0=off, 1=FN, 2=FN2
        
        self._setup_styles()
        self._create_widgets()
        self._update_serial_ports()
    
    def _setup_styles(self):
        """Configure ttk styles"""
        style = ttk.Style()
        style.theme_use('clam')
        
        style.configure('TFrame', background=THEME['bg'])
        style.configure('TLabel', background=THEME['bg'], foreground=THEME['fg'], font=FONTS['label'])
        style.configure('TButton', font=FONTS['button'])
        
        # Notebook (tabs)
        style.configure('TNotebook', background=THEME['bg'], tabmargins=[2, 5, 2, 0])
        style.configure('TNotebook.Tab', 
                       background=THEME['tab_bg'],
                       foreground=THEME['tab_fg'],
                       font=FONTS['button_small'],
                       padding=[10, 5])
        style.map('TNotebook.Tab',
                 background=[('selected', THEME['tab_active_bg'])],
                 foreground=[('selected', THEME['tab_active_fg'])])
    
    def _create_widgets(self):
        """Create all GUI widgets"""
        # Main container
        main_frame = tk.Frame(self.root, bg=THEME['bg'])
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # === Display Area ===
        self._create_display(main_frame)
        
        # === Status Bar ===
        self._create_status_bar(main_frame)
        
        # === Keypad and Function Panels ===
        content_frame = tk.Frame(main_frame, bg=THEME['bg'])
        content_frame.pack(fill=tk.BOTH, expand=True, pady=(10, 0))
        
        # Left: 4x4 Keypad
        self._create_keypad(content_frame)
        
        # Right: Function buttons + Tabs
        self._create_function_area(content_frame)
    
    def _create_display(self, parent):
        """Create display area for input and result"""
        display_frame = tk.Frame(parent, bg=THEME['display_border'], bd=2)
        display_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Input display
        self.input_var = tk.StringVar(value="")
        input_label = tk.Label(display_frame, 
                              textvariable=self.input_var,
                              bg=THEME['display_bg'],
                              fg=THEME['display_fg'],
                              font=FONTS['display_small'],
                              anchor='e',
                              padx=10,
                              pady=5)
        input_label.pack(fill=tk.X)
        
        # Result display
        self.result_var = tk.StringVar(value="= ")
        result_label = tk.Label(display_frame,
                               textvariable=self.result_var,
                               bg=THEME['display_bg'],
                               fg=THEME['display_fg'],
                               font=FONTS['display'],
                               anchor='e',
                               padx=10,
                               pady=10)
        result_label.pack(fill=tk.X)
    
    def _create_status_bar(self, parent):
        """Create status bar with mode and connection info"""
        status_frame = tk.Frame(parent, bg=THEME['status_bg'], height=30)
        status_frame.pack(fill=tk.X, pady=(0, 10))
        status_frame.pack_propagate(False)
        
        # Mode indicator
        self.mode_var = tk.StringVar(value="[NORM]")
        mode_label = tk.Label(status_frame,
                             textvariable=self.mode_var,
                             bg=THEME['status_bg'],
                             fg=THEME['status_fg'],
                             font=FONTS['status'])
        mode_label.pack(side=tk.LEFT, padx=10)
        
        # Angle mode
        self.angle_var = tk.StringVar(value="[DEG]")
        angle_label = tk.Label(status_frame,
                              textvariable=self.angle_var,
                              bg=THEME['status_bg'],
                              fg=THEME['status_fg'],
                              font=FONTS['status'])
        angle_label.pack(side=tk.LEFT, padx=5)
        
        # FN state
        self.fn_var = tk.StringVar(value="")
        fn_label = tk.Label(status_frame,
                           textvariable=self.fn_var,
                           bg=THEME['status_bg'],
                           fg=THEME['accent'],
                           font=FONTS['status'])
        fn_label.pack(side=tk.LEFT, padx=5)
        
        # Serial port selector
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(status_frame,
                                       textvariable=self.port_var,
                                       values=[],
                                       width=12,
                                       state='readonly')
        self.port_combo.pack(side=tk.RIGHT, padx=5)
        
        # Connect button
        self.connect_btn = tk.Button(status_frame,
                                    text="Connect",
                                    command=self._toggle_connection,
                                    bg=THEME['btn_accent_bg'],
                                    fg=THEME['btn_accent_fg'],
                                    font=FONTS['status'],
                                    relief='flat',
                                    bd=0,
                                    padx=10)
        self.connect_btn.pack(side=tk.RIGHT, padx=5)
        
        # Connection status
        self.conn_var = tk.StringVar(value="Disconnected")
        conn_label = tk.Label(status_frame,
                             textvariable=self.conn_var,
                             bg=THEME['status_bg'],
                             fg=THEME['status_disconnected'],
                             font=FONTS['status'])
        conn_label.pack(side=tk.RIGHT, padx=10)
    
    def _create_keypad(self, parent):
        """Create 4x4 virtual keypad matching hardware layout"""
        keypad_frame = tk.Frame(parent, bg=THEME['bg'])
        keypad_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Keypad buttons layout (matching 4x4 hardware)
        keypad_layout = [
            [('7', 'num'), ('8', 'num'), ('9', 'num'), ('/', 'op')],
            [('4', 'num'), ('5', 'num'), ('6', 'num'), ('*', 'op')],
            [('1', 'num'), ('2', 'num'), ('3', 'num'), ('-', 'op')],
            [('0', 'num'), ('.', 'num'), ('=', 'accent'), ('+', 'special')],  # + is FN key
        ]
        
        for row_idx, row in enumerate(keypad_layout):
            for col_idx, (text, style_name) in enumerate(row):
                btn = tk.Button(keypad_frame,
                               text=text,
                               command=lambda t=text: self._on_keypad_press(t),
                               **BUTTON_STYLES[style_name])
                btn.grid(row=row_idx, column=col_idx, sticky='nsew', padx=3, pady=3)
        
        # Configure grid weights
        for i in range(4):
            keypad_frame.grid_rowconfigure(i, weight=1)
            keypad_frame.grid_columnconfigure(i, weight=1)
    
    def _create_function_area(self, parent):
        """Create function buttons and tabbed panels"""
        func_frame = tk.Frame(parent, bg=THEME['bg'])
        func_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(10, 0))
        
        # Quick function buttons
        quick_frame = tk.Frame(func_frame, bg=THEME['bg'])
        quick_frame.pack(fill=tk.X, pady=(0, 5))
        
        quick_funcs = ['sin', 'cos', 'tan', 'log', 'ln', 'sqrt', 'exp', '^']
        for i, func in enumerate(quick_funcs):
            btn = tk.Button(quick_frame,
                           text=func,
                           command=lambda f=func: self._on_function_press(f),
                           **BUTTON_STYLES['func'])
            btn.grid(row=i // 4, column=i % 4, sticky='nsew', padx=2, pady=2)
        
        for i in range(4):
            quick_frame.grid_columnconfigure(i, weight=1)
        
        # Tabbed panels
        self.notebook = ttk.Notebook(func_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)
        
        # Scientific tab
        self._create_scientific_tab()
        
        # Complex tab
        self._create_complex_tab()
        
        # Matrix tab
        self._create_matrix_tab()
        
        # History tab
        self._create_history_tab()
    
    def _create_scientific_tab(self):
        """Create scientific functions tab"""
        tab = tk.Frame(self.notebook, bg=THEME['bg'])
        self.notebook.add(tab, text="Scientific")
        
        funcs = [
            ['asin', 'acos', 'atan', 'sinh', 'cosh', 'tanh'],
            ['abs', 'floor', 'ceil', 'round', 'fact', 'cbrt'],
            ['PI', 'E', 'deg(', 'rad(', '(', ')'],
        ]
        
        for row_idx, row in enumerate(funcs):
            for col_idx, func in enumerate(row):
                btn = tk.Button(tab,
                               text=func,
                               command=lambda f=func: self._on_function_press(f),
                               **BUTTON_STYLES['func'])
                btn.grid(row=row_idx, column=col_idx, sticky='nsew', padx=3, pady=3)
            tab.grid_columnconfigure(col_idx, weight=1)
    
    def _create_complex_tab(self):
        """Create complex number operations tab"""
        tab = tk.Frame(self.notebook, bg=THEME['bg'])
        self.notebook.add(tab, text="Complex")
        
        label = tk.Label(tab,
                        text="Complex Number Operations",
                        bg=THEME['bg'],
                        fg=THEME['fg'],
                        font=FONTS['label'])
        label.pack(pady=10)
        
        info = tk.Label(tab,
                       text="Use 'i' for imaginary unit\nExample: 3+4i, (1+2i)*(3-i)",
                       bg=THEME['bg'],
                       fg=THEME['status_fg'],
                       font=FONTS['status'],
                       justify=tk.LEFT)
        info.pack(pady=5)
        
        complex_funcs = ['abs', 'arg', 'conj', 'real', 'imag']
        for func in complex_funcs:
            btn = tk.Button(tab,
                           text=func,
                           command=lambda f=func: self._on_function_press(f + '('),
                           **BUTTON_STYLES['func'])
            btn.pack(fill=tk.X, padx=10, pady=2)
    
    def _create_matrix_tab(self):
        """Create matrix operations tab"""
        tab = tk.Frame(self.notebook, bg=THEME['bg'])
        self.notebook.add(tab, text="Matrix")
        
        label = tk.Label(tab,
                        text="Matrix Operations",
                        bg=THEME['bg'],
                        fg=THEME['fg'],
                        font=FONTS['label'])
        label.pack(pady=10)
        
        info = tk.Label(tab,
                       text="Use [a,b;c,d] for matrix input",
                       bg=THEME['bg'],
                       fg=THEME['status_fg'],
                       font=FONTS['status'])
        info.pack(pady=5)
        
        matrix_funcs = ['det', 'inv', 'trans', 'rank']
        for func in matrix_funcs:
            btn = tk.Button(tab,
                           text=func,
                           command=lambda f=func: self._on_function_press(f + '('),
                           **BUTTON_STYLES['func'])
            btn.pack(fill=tk.X, padx=10, pady=2)
    
    def _create_history_tab(self):
        """Create calculation history tab"""
        tab = tk.Frame(self.notebook, bg=THEME['bg'])
        self.notebook.add(tab, text="History")
        
        # History list
        self.history_listbox = tk.Listbox(tab,
                                         bg=THEME['display_bg'],
                                         fg=THEME['display_fg'],
                                         font=FONTS['display_small'],
                                         selectmode=tk.SINGLE,
                                         relief='flat',
                                         bd=0,
                                         highlightthickness=0)
        self.history_listbox.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Scrollbar
        scrollbar = tk.Scrollbar(self.history_listbox)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.history_listbox.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.history_listbox.yview)
        
        # Buttons
        btn_frame = tk.Frame(tab, bg=THEME['bg'])
        btn_frame.pack(fill=tk.X, padx=5, pady=5)
        
        use_btn = tk.Button(btn_frame,
                           text="Use Selected",
                           command=self._use_history_item,
                           **BUTTON_STYLES['accent'])
        use_btn.pack(side=tk.LEFT, padx=5)
        
        clear_btn = tk.Button(btn_frame,
                             text="Clear History",
                             command=self._clear_history,
                             **BUTTON_STYLES['special'])
        clear_btn.pack(side=tk.RIGHT, padx=5)
    
    def _on_keypad_press(self, key: str):
        """Handle keypad button press"""
        if key == '+':
            # FN key
            self._toggle_fn_state()
            return
        
        if key == '=':
            self._calculate()
            return
        
        # Apply FN layer mapping if active
        if self.fn_state > 0:
            key = self._apply_fn_layer(key)
            # Reset FN_ONCE after one use
            if self.fn_state == 1:
                self.fn_state = 0
                self.fn_var.set("")
        
        self.current_input += key
        self._update_display()
    
    def _on_function_press(self, func: str):
        """Handle function button press"""
        self.current_input += func
        self._update_display()
    
    def _apply_fn_layer(self, key: str) -> str:
        """Apply FN layer mapping to key"""
        fn_map = {
            '7': 'sin', '8': 'cos', '9': 'tan', '/': 'log',
            '4': 'asin', '5': 'acos', '6': 'atan', '*': 'ln',
            '1': 'exp', '2': 'sqrt', '3': 'cbrt', '-': '^',
            '0': '(', '.': ')', '=': 'DEL',
        }
        fn2_map = {
            '7': 'A', '8': 'B', '9': 'C', '/': 'i',
            '4': 'G', '5': 'H', '6': 'J', '*': 'K',
            '1': '[', '2': ']', '3': ';', '-': 'M',
            '0': 'N', '.': 'O', '=': 'CLR', '+': 'MODE',
        }
        
        if self.fn_state == 2:
            return fn2_map.get(key, key)
        return fn_map.get(key, key)
    
    def _toggle_fn_state(self):
        """Toggle FN layer state"""
        self.fn_state = (self.fn_state + 1) % 3
        names = ["", "FN", "FN2"]
        self.fn_var.set(f"[{names[self.fn_state]}]" if self.fn_state > 0 else "")
    
    def _calculate(self):
        """Calculate current expression"""
        if not self.current_input:
            return
        
        # Handle special commands
        if self.current_input == 'CLR':
            self.current_input = ""
            self.result_var.set("= ")
            self._update_display()
            return
        
        if self.current_input == 'DEL':
            self.current_input = self.current_input[:-3]  # Remove 'DEL'
            self.current_input = self.current_input[:-1] if self.current_input else ""
            self._update_display()
            return
        
        if self.current_input == 'MODE':
            self._toggle_calc_mode()
            self.current_input = ""
            self._update_display()
            return
        
        # Calculate
        if self.use_hardware and self.serial.connected:
            # Send to Arduino
            self.serial.calculate(self.current_input)
            # Wait for response (simplified)
            self.result_var.set("= Calculating...")
        else:
            # Local calculation
            result = self.calc.evaluate(self.current_input)
            self.last_result = result
            
            if isinstance(result, str) and result.startswith("Error"):
                self.result_var.set(f"= {result}")
            else:
                self.result_var.set(f"= {result}")
                self._add_to_history(self.current_input, result)
        
        self.current_input = ""
        self._update_display()
    
    def _toggle_calc_mode(self):
        """Toggle calculator mode"""
        modes = ["NORM", "MAT", "CPLX"]
        current = self.mode_var.get().strip('[]')
        try:
            idx = modes.index(current)
            next_mode = modes[(idx + 1) % len(modes)]
        except ValueError:
            next_mode = "NORM"
        self.mode_var.set(f"[{next_mode}]")
    
    def _toggle_angle_mode(self):
        """Toggle angle mode"""
        if self.angle_var.get() == "[DEG]":
            self.angle_var.set("[RAD]")
            self.calc.set_angle_mode('RAD')
            if self.use_hardware:
                self.serial.set_mode('RAD')
        else:
            self.angle_var.set("[DEG]")
            self.calc.set_angle_mode('DEG')
            if self.use_hardware:
                self.serial.set_mode('DEG')
    
    def _update_display(self):
        """Update input display"""
        self.input_var.set(self.current_input)
    
    def _add_to_history(self, expr: str, result):
        """Add item to history list"""
        display_text = f"{expr} = {result}"
        self.history_listbox.insert(tk.END, display_text)
        self.history_listbox.see(tk.END)
    
    def _use_history_item(self):
        """Use selected history item"""
        selection = self.history_listbox.curselection()
        if selection:
            text = self.history_listbox.get(selection[0])
            # Extract expression part
            if '=' in text:
                expr = text.split('=')[0].strip()
                self.current_input = expr
                self._update_display()
    
    def _clear_history(self):
        """Clear history"""
        self.history_listbox.delete(0, tk.END)
        self.calc.clear_history()
    
    def _update_serial_ports(self):
        """Update available serial ports"""
        ports = self.serial.list_ports()
        self.port_combo['values'] = ports
        if ports:
            self.port_var.set(ports[0])
    
    def _toggle_connection(self):
        """Toggle serial connection"""
        if self.serial.connected:
            self.serial.disconnect()
            self.connect_btn.config(text="Connect", bg=THEME['btn_accent_bg'])
            self.conn_var.set("Disconnected")
            self.conn_var_label = self.conn_var
            # Update color
            for widget in self.root.winfo_children():
                self._update_conn_color(widget, THEME['status_disconnected'])
            self.use_hardware = False
        else:
            port = self.port_var.get()
            if not port:
                messagebox.showwarning("No Port", "Please select a serial port")
                return
            
            if self.serial.connect(port):
                self.connect_btn.config(text="Disconnect", bg=THEME['btn_special_bg'])
                self.conn_var.set(f"Connected {port}")
                for widget in self.root.winfo_children():
                    self._update_conn_color(widget, THEME['status_connected'])
                self.use_hardware = True
            else:
                messagebox.showerror("Connection Failed", f"Could not connect to {port}")
    
    def _update_conn_color(self, widget, color):
        """Update connection status color"""
        try:
            if isinstance(widget, tk.Label) and hasattr(widget, 'textvariable'):
                if widget.textvariable == self.conn_var:
                    widget.config(fg=color)
        except:
            pass
    
    def _on_serial_data(self, data: str):
        """Handle incoming serial data"""
        if data.startswith("<OK "):
            result = data[4:]
            self.result_var.set(f"= {result}")
        elif data.startswith("<ERR "):
            error = data[5:]
            self.result_var.set(f"= Error: {error}")
    
    def run(self):
        """Start the GUI"""
        self.serial.set_callback(self._on_serial_data)
        self.root.mainloop()
        self.serial.disconnect()


def main():
    """Main entry point"""
    app = CalculatorGUI()
    app.run()


if __name__ == "__main__":
    main()
