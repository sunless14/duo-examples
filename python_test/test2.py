import serial
import time

# 配置串口参数
port = 'COM10'  # 替换为你的串口设备文件，Windows上可能是 COM3 等
baudrate = 31250  # 根据实际硬件设定的波特率
timeout = 1      # 读取超时设置为1秒

# 创建并打开串口
ser = serial.Serial(port, baudrate, timeout=timeout)

# 检查串口是否打开
if ser.is_open:
    print(f"串口 {port} 已打开")

try:
    while True:
        # 读取串口数据
        data = ser.readline()  # readline()尝试读取一行数据，直到遇到换行符
        if data:
            # 打印接收到的数据
            print("Received:", data)  # 解码字节到字符串
        else:
            # print("No data received.")
            pass
        
        # 简单的延时，避免CPU占用过高
        # time.sleep(0.1)

except KeyboardInterrupt:
    print("程序被用户中断")

finally:
    ser.close()  # 关闭串口
    print("串口已关闭")
