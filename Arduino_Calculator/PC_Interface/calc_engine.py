"""
Local Calculation Engine for PC Interface
Replicates Arduino calculator logic in Python
"""

import math
import cmath
from typing import Union, List, Tuple, Optional


class Complex:
    """Complex number representation"""
    def __init__(self, real: float = 0.0, imag: float = 0.0):
        self.real = real
        self.imag = imag
    
    def to_complex(self) -> complex:
        return complex(self.real, self.imag)
    
    @classmethod
    def from_complex(cls, c: complex):
        return cls(c.real, c.imag)
    
    def __str__(self) -> str:
        if abs(self.imag) < 1e-10:
            return f"{self.real:.6g}"
        if abs(self.real) < 1e-10:
            if abs(self.imag - 1) < 1e-10:
                return "i"
            if abs(self.imag + 1) < 1e-10:
                return "-i"
            return f"{self.imag:.6g}i"
        sign = "+" if self.imag >= 0 else "-"
        imag = abs(self.imag)
        if abs(imag - 1) < 1e-10:
            imag_str = "i"
        else:
            imag_str = f"{imag:.6g}i"
        return f"{self.real:.6g}{sign}{imag_str}"


class Matrix:
    """Matrix representation"""
    def __init__(self, data: List[List[float]] = None):
        if data is None:
            data = []
        self.data = [row[:] for row in data]
        self.rows = len(self.data)
        self.cols = len(self.data[0]) if self.data else 0
    
    def __str__(self) -> str:
        if not self.data:
            return "[]"
        return f"[{self.rows}x{self.cols}]"
    
    def to_string_full(self) -> str:
        if not self.data:
            return "[]"
        lines = []
        for row in self.data:
            lines.append("  [" + ", ".join(f"{x:.4g}" for x in row) + "]")
        return "[\n" + "\n".join(lines) + "\n]"


class CalcEngine:
    """Calculation engine supporting all calculator operations"""
    
    def __init__(self):
        self.angle_mode = 'DEG'  # 'DEG' or 'RAD'
        self.variables = {}
        self.history = []
        self.max_history = 50
    
    def set_angle_mode(self, mode: str):
        """Set angle mode: 'DEG' or 'RAD'"""
        self.angle_mode = mode.upper()
    
    def to_rad(self, angle: float) -> float:
        """Convert angle to radians"""
        if self.angle_mode == 'DEG':
            return math.radians(angle)
        return angle
    
    def from_rad(self, rad: float) -> float:
        """Convert radians to angle mode"""
        if self.angle_mode == 'DEG':
            return math.degrees(rad)
        return rad
    
    def evaluate(self, expression: str) -> Union[float, Complex, Matrix, str]:
        """Evaluate an expression string"""
        expression = expression.strip()
        if not expression:
            return "Error: Empty expression"
        
        try:
            # Simple expression evaluation using Python's eval with safe dict
            result = self._safe_eval(expression)
            
            # Add to history
            self._add_to_history(expression, result)
            
            return result
        except Exception as e:
            return f"Error: {str(e)}"
    
    def _safe_eval(self, expr: str):
        """Safely evaluate expression with calculator functions"""
        # Replace calculator-specific syntax
        expr = expr.replace('^', '**')
        expr = expr.replace('PI', str(math.pi))
        expr = expr.replace('E', str(math.e))
        
        # Create safe environment
        safe_dict = {
            'sin': self._sin,
            'cos': self._cos,
            'tan': self._tan,
            'asin': self._asin,
            'acos': self._acos,
            'atan': self._atan,
            'sinh': math.sinh,
            'cosh': math.cosh,
            'tanh': math.tanh,
            'ln': math.log,
            'log': math.log10,
            'log2': math.log2,
            'exp': math.exp,
            'sqrt': math.sqrt,
            'cbrt': self._cbrt,
            'abs': abs,
            'floor': math.floor,
            'ceil': math.ceil,
            'round': round,
            'fact': self._factorial,
            'deg': math.degrees,
            'rad': math.radians,
            'pow': pow,
            'pi': math.pi,
            'e': math.e,
        }
        
        # Add complex number support
        safe_dict['i'] = 1j
        
        try:
            result = eval(expr, {"__builtins__": {}}, safe_dict)
            
            # Convert to appropriate type
            if isinstance(result, complex):
                return Complex(result.real, result.imag)
            return result
        except Exception:
            # Try complex evaluation
            try:
                result = eval(expr, {"__builtins__": {}}, {**safe_dict, 'i': 1j})
                if isinstance(result, complex):
                    return Complex(result.real, result.imag)
                return result
            except Exception as e:
                raise e
    
    def _sin(self, x):
        return math.sin(self.to_rad(x))
    
    def _cos(self, x):
        return math.cos(self.to_rad(x))
    
    def _tan(self, x):
        return math.tan(self.to_rad(x))
    
    def _asin(self, x):
        return self.from_rad(math.asin(x))
    
    def _acos(self, x):
        return self.from_rad(math.acos(x))
    
    def _atan(self, x):
        return self.from_rad(math.atan(x))
    
    def _cbrt(self, x):
        if x < 0:
            return -(-x) ** (1/3)
        return x ** (1/3)
    
    def _factorial(self, n):
        n = int(n)
        if n < 0:
            raise ValueError("Factorial not defined for negative numbers")
        result = 1
        for i in range(2, n + 1):
            result *= i
        return result
    
    def store_variable(self, name: str, value):
        """Store a variable"""
        self.variables[name] = value
    
    def recall_variable(self, name: str):
        """Recall a variable"""
        return self.variables.get(name)
    
    def _add_to_history(self, expr: str, result):
        """Add calculation to history"""
        self.history.append((expr, result))
        if len(self.history) > self.max_history:
            self.history.pop(0)
    
    def get_history(self) -> List[Tuple[str, Union[float, Complex, Matrix]]]:
        """Get calculation history"""
        return self.history.copy()
    
    def clear_history(self):
        """Clear history"""
        self.history.clear()
    
    def matrix_add(self, a: Matrix, b: Matrix) -> Matrix:
        """Matrix addition"""
        if a.rows != b.rows or a.cols != b.cols:
            raise ValueError("Matrix dimensions must match")
        result = [[a.data[i][j] + b.data[i][j] for j in range(a.cols)] for i in range(a.rows)]
        return Matrix(result)
    
    def matrix_sub(self, a: Matrix, b: Matrix) -> Matrix:
        """Matrix subtraction"""
        if a.rows != b.rows or a.cols != b.cols:
            raise ValueError("Matrix dimensions must match")
        result = [[a.data[i][j] - b.data[i][j] for j in range(a.cols)] for i in range(a.rows)]
        return Matrix(result)
    
    def matrix_mul(self, a: Matrix, b: Matrix) -> Matrix:
        """Matrix multiplication"""
        if a.cols != b.rows:
            raise ValueError("Matrix A columns must equal Matrix B rows")
        result = [[sum(a.data[i][k] * b.data[k][j] for k in range(a.cols)) 
                   for j in range(b.cols)] for i in range(a.rows)]
        return Matrix(result)
    
    def matrix_transpose(self, a: Matrix) -> Matrix:
        """Matrix transpose"""
        result = [[a.data[j][i] for j in range(a.rows)] for i in range(a.cols)]
        return Matrix(result)
    
    def matrix_det(self, a: Matrix) -> float:
        """Matrix determinant"""
        if a.rows != a.cols:
            raise ValueError("Matrix must be square")
        if a.rows == 1:
            return a.data[0][0]
        if a.rows == 2:
            return a.data[0][0] * a.data[1][1] - a.data[0][1] * a.data[1][0]
        if a.rows == 3:
            return (a.data[0][0] * (a.data[1][1] * a.data[2][2] - a.data[1][2] * a.data[2][1])
                  - a.data[0][1] * (a.data[1][0] * a.data[2][2] - a.data[1][2] * a.data[2][0])
                  + a.data[0][2] * (a.data[1][0] * a.data[2][1] - a.data[1][1] * a.data[2][0]))
        raise ValueError("Determinant for matrices >3x3 not implemented")
