import socket
import os
import time

def send_udp_data(ip, port):
    # 创建一个socket对象，AF_INET 表示使用IPv4，SOCK_DGRAM 表示使用UDP
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        # 生成随机长度的随机数据，长度最大为256字节
        data_length = os.urandom(1)[0]  # 生成一个0-255的随机数作为数据长度
        data = os.urandom(data_length)  # 生成相应长度的随机数据
        data = "ni sgi ge da sb!!!!!!".encode('utf-8')
        while(1):
        # 发送数据
          sock.sendto(data, (ip, port))
          time.sleep(0.01)
        # print(f"Sent {data_length} bytes of data to {ip}:{port}")

    finally:
        # 关闭socket
        sock.close()

# 使用示例
send_udp_data('192.168.1.231', 12345)