from machine import UART, Pin

uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))
uart.init(bits=8, parity=None, stop=2)

def calculate(string):
    result = 0
    num1 = 0
    num2 = 0
    sign = ""
    sign_index = 0
    
    # Search Sign
    for index, char in enumerate(string):
        if char in "+-*/":
            sign = char
            sign_index = index

    # Read Nums
    num1 = int(string[0:sign_index])
    num2 = int(string[(sign_index + 1):])
    
    # Calculate
    if sign == "+":
        result = num1 + num2
        
    elif sign == "-":
        result = num1 - num2

    elif sign == "*":
        result = num1 * num2

    else:
        result = num1 / num2

    return num1, sign, num2, result

def main():
    string = ""
    
    try:
        string = uart.read()
        string = string.decode().strip()
        print(f"Received: {string}")
        
        num1, sign, num2, result = calculate(string)
        
        string = f"{num1} {sign} {num2} = {result}"
        
        print(f"Send: {string}")
        uart.write(string)
        
    except Exception as e:
        print(f"Error: {e}")
        uart.write(f"Error: {e}")

while True:
    if uart.any():
        main()