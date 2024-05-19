import socket
import serial
from serial.tools import list_ports
import json
import matplotlib.pyplot as plt
import time

datas = {"#0/72/90/*", "#0/63/90/*", "#0/55/11/*", "#0/56/13/*"}
list_len = 5
data_list = list_of_dicts = [{} for _ in range(list_len)]
list_idx = 0
file_name = "music.txt"
data_len = 0
all_data = list()
x = range(0,1500)

def find_ch340_ports():
    # 列出所有可用的串口
    ports = list_ports.comports()
    ch340_port = str()

    # 检查每个串口的描述信息
    for port in ports:
        if "CP210" in port.description:
            # 如果描述中包含"CH340"，则记录下该串口
            ch340_port = port.device

    return ch340_port

def gen_file_name() ->str:
    return str(time.time()) + ".csv"

def save_data(file, dict) :
    dict = iter(dict.items())
    next(dict)
    for _, v in dict:
        file.write(f"{v},")
    file.write("\n")


def udp_server(host, port, com, file_name):
    # 创建UDP套接字
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ser = serial.Serial(com, 9600, timeout=1)
    # 绑定IP和端口
    # breakpoint()
    udp_socket.bind((host, port))
    print(f"UDP服务器已启动，监听 {host}:{port}")
    global list_idx
    global data_len
    f = open(file_name, 'a+')
    f.write("x,y,z,w,accx,accy,accz,gx,gy,gz,s,p\n")
    # breakpoint()
    try:
        while True:
            # 接收数据
            data, address = udp_socket.recvfrom(1024)
            if ser.isOpen():
                string_data = data.decode("utf-8")
                dict_data = json.loads(string_data)
                print(dict_data)
                data_list[list_idx] = dict_data
                list_idx = list_idx + 1
                data_len = data_len + 1
                if list_idx == list_len:
                    list_idx = 0
                    data = "#0/"
                    note = int()
                    for one in data_list:
                        save_data(f, one)
                        note = note + int(float(one.get('accz')))
                        all_data.append(int(float(one.get('accz'))))
                    data = data + str(note) + "/90/*"
                    print(data)
                    data = data.encode('utf-8')
                    ser.write(data)
                    #f.write(f"{data}\n")
            if data_len == 1500:
                plt.figure(figsize=(10, 6))  # 设置图表大小
                plt.plot(x, all_data, label='acc z轴加速度')  # 绘制x和y的折线图，添加图例说明
                plt.title('Line Plot with 1500 points')  # 添加图表标题
                plt.xlabel('X axis')  # 添加x轴标题
                plt.ylabel('Y axis')  # 添加y轴标题
                plt.legend()  # 显示图例
                plt.show()  # 显示图表
            else:
                print("dadada")
    except KeyboardInterrupt:
        print("服务器已关闭")
    finally:
        # 关闭套接字
        f.close()
        udp_socket.close()

# 替换以下IP和端口为你希望监听的地址和端口
ch340_port = find_ch340_ports()
print(ch340_port)
file_name = gen_file_name()
udp_server('192.168.1.231', 12345, "COM5", file_name)
