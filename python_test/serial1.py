import serial
import os

# 配置串口参数
port = 'com5'
baudrate = 31250  # 根据实际情况调整波特率
timeout = 1      # 读超时设置

# 打开串口
ser = serial.Serial(port, baudrate, timeout=timeout)
if not ser.is_open:
    ser.open()

try:
    # 生成随机数据
    data = b'\xb0\x07x\xb0\x00\x00\xc0\x00'

    # 发送数据
    ser.write(data)
    print(f"Sent data: {data}")
    while(1) :
        data = b'\x90-Z'
        ser.write(data)
        print(f"Sent data: {data}")

finally:
    # 关闭串口
    ser.close()
