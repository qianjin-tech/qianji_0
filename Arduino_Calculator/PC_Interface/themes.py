"""
Theme Configuration for Arduino Calculator PC Interface
Modern dark theme inspired by Catppuccin Mocha
"""

THEME = {
    # Window
    'bg': '#1e1e2e',
    'fg': '#cdd6f4',
    'accent': '#89b4fa',
    
    # Display area
    'display_bg': '#313244',
    'display_fg': '#a6e3a1',
    'display_border': '#45475a',
    
    # Buttons - numbers
    'btn_num_bg': '#45475a',
    'btn_num_fg': '#cdd6f4',
    'btn_num_active': '#585b70',
    
    # Buttons - operators
    'btn_op_bg': '#585b70',
    'btn_op_fg': '#f9e2af',
    'btn_op_active': '#6c7086',
    
    # Buttons - functions
    'btn_func_bg': '#6c7086',
    'btn_func_fg': '#89b4fa',
    'btn_func_active': '#7f849c',
    
    # Buttons - special (FN, Clear, etc)
    'btn_special_bg': '#f38ba8',
    'btn_special_fg': '#1e1e2e',
    'btn_special_active': '#eba0ac',
    
    # Buttons - accent
    'btn_accent_bg': '#89b4fa',
    'btn_accent_fg': '#1e1e2e',
    'btn_accent_active': '#b4befe',
    
    # Status bar
    'status_bg': '#181825',
    'status_fg': '#a6adc8',
    'status_connected': '#a6e3a1',
    'status_disconnected': '#f38ba8',
    
    # Error
    'error_fg': '#f38ba8',
    
    # Panel tabs
    'tab_bg': '#313244',
    'tab_fg': '#a6adc8',
    'tab_active_bg': '#45475a',
    'tab_active_fg': '#cdd6f4',
}

FONTS = {
    'display': ('Consolas', 18, 'bold'),
    'display_small': ('Consolas', 12),
    'button': ('Segoe UI', 11, 'bold'),
    'button_small': ('Segoe UI', 9),
    'status': ('Segoe UI', 9),
    'label': ('Segoe UI', 10),
}

BUTTON_STYLES = {
    'num': {
        'bg': THEME['btn_num_bg'],
        'fg': THEME['btn_num_fg'],
        'activebackground': THEME['btn_num_active'],
        'activeforeground': THEME['btn_num_fg'],
        'font': FONTS['button'],
        'relief': 'flat',
        'bd': 0,
        'highlightthickness': 0,
        'padx': 10,
        'pady': 8,
    },
    'op': {
        'bg': THEME['btn_op_bg'],
        'fg': THEME['btn_op_fg'],
        'activebackground': THEME['btn_op_active'],
        'activeforeground': THEME['btn_op_fg'],
        'font': FONTS['button'],
        'relief': 'flat',
        'bd': 0,
        'highlightthickness': 0,
        'padx': 10,
        'pady': 8,
    },
    'func': {
        'bg': THEME['btn_func_bg'],
        'fg': THEME['btn_func_fg'],
        'activebackground': THEME['btn_func_active'],
        'activeforeground': THEME['btn_func_fg'],
        'font': FONTS['button_small'],
        'relief': 'flat',
        'bd': 0,
        'highlightthickness': 0,
        'padx': 8,
        'pady': 6,
    },
    'special': {
        'bg': THEME['btn_special_bg'],
        'fg': THEME['btn_special_fg'],
        'activebackground': THEME['btn_special_active'],
        'activeforeground': THEME['btn_special_fg'],
        'font': FONTS['button'],
        'relief': 'flat',
        'bd': 0,
        'highlightthickness': 0,
        'padx': 10,
        'pady': 8,
    },
    'accent': {
        'bg': THEME['btn_accent_bg'],
        'fg': THEME['btn_accent_fg'],
        'activebackground': THEME['btn_accent_active'],
        'activeforeground': THEME['btn_accent_fg'],
        'font': FONTS['button'],
        'relief': 'flat',
        'bd': 0,
        'highlightthickness': 0,
        'padx': 10,
        'pady': 8,
    },
}
