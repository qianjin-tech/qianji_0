"""
Serial Communication Module for Arduino Calculator PC Interface
Handles connection, command sending, and response parsing
"""

import serial
import serial.tools.list_ports
import threading
import time
from typing import Optional, Callable, List


class SerialComm:
    """Serial communication handler for Arduino Calculator"""
    
    def __init__(self):
        self.ser: Optional[serial.Serial] = None
        self.port: Optional[str] = None
        self.baudrate = 115200
        self.connected = False
        self._read_thread: Optional[threading.Thread] = None
        self._running = False
        self._callback: Optional[Callable[[str], None]] = None
        self._response_buffer: List[str] = []
        self._buffer_lock = threading.Lock()
    
    def list_ports(self) -> List[str]:
        """List available serial ports"""
        return [p.device for p in serial.tools.list_ports.comports()]
    
    def connect(self, port: str) -> bool:
        """Connect to Arduino on specified port"""
        try:
            self.ser = serial.Serial(
                port=port,
                baudrate=self.baudrate,
                timeout=1,
                write_timeout=1
            )
            time.sleep(2)  # Wait for Arduino reset
            self.port = port
            self.connected = True
            self._running = True
            self._read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self._read_thread.start()
            return True
        except Exception as e:
            print(f"Connection error: {e}")
            self.connected = False
            return False
    
    def disconnect(self):
        """Disconnect from Arduino"""
        self._running = False
        if self._read_thread:
            self._read_thread.join(timeout=1)
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.connected = False
        self.port = None
    
    def send_command(self, command: str) -> bool:
        """Send a command to Arduino"""
        if not self.connected or not self.ser:
            return False
        try:
            self.ser.write(f"{command}\n".encode())
            return True
        except Exception as e:
            print(f"Send error: {e}")
            self.connected = False
            return False
    
    def calculate(self, expression: str) -> bool:
        """Send calculation command"""
        return self.send_command(f">CALC {expression}")
    
    def set_mode(self, mode: str) -> bool:
        """Set angle mode (DEG or RAD)"""
        return self.send_command(f">MODE {mode}")
    
    def store_variable(self, name: str, value: float) -> bool:
        """Store variable on Arduino"""
        return self.send_command(f">STORE {name} {value}")
    
    def recall_variable(self, name: str) -> bool:
        """Recall variable from Arduino"""
        return self.send_command(f">RECALL {name}")
    
    def reset(self) -> bool:
        """Reset Arduino calculator"""
        return self.send_command(">RESET")
    
    def get_status(self) -> bool:
        """Get Arduino status"""
        return self.send_command(">STATUS")
    
    def set_callback(self, callback: Callable[[str], None]):
        """Set callback for received data"""
        self._callback = callback
    
    def get_responses(self) -> List[str]:
        """Get all buffered responses"""
        with self._buffer_lock:
            responses = self._response_buffer.copy()
            self._response_buffer.clear()
            return responses
    
    def _read_loop(self):
        """Background read loop"""
        while self._running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode().strip()
                    if line:
                        with self._buffer_lock:
                            self._response_buffer.append(line)
                        if self._callback:
                            self._callback(line)
            except Exception as e:
                print(f"Read error: {e}")
                break
            time.sleep(0.01)
        self.connected = False
