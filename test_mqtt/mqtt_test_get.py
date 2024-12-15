# -- coding: UTF-8 --
import json
from local_public.public_json_dir import JsonDir
import paho.mqtt.client as mqtt
import time
from local_public import *
from local_public.public_log import log_init

global glog
global gclient

# 当代理响应订阅请求时被调用。
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        glog.debug("连接成功")
    glog.debug("Connected with result code " + str(rc))


# 当代理响应订阅请求时被调用
def on_subscribe(client, userdata, mid, granted_qos):
    glog.debug("Subscribed: " + str(mid) + " " + str(granted_qos))


# 当使用使用publish()发送的消息已经传输到代理时被调用。
def on_publish(client, obj, mid):
    glog.debug("OnPublish, mid: " + str(mid))


# 当收到关于客户订阅的主题的消息时调用。 message是一个描述所有消息参数的MQTTMessage。
def on_message(client, userdata, msg):
    glog.debug(msg.topic + " " + str(msg.payload))


# 当客户端有日志信息时调用
def on_log(client, obj, level, string):
    glog.debug("Log:" + string)


def create_client(mqtt_client_id):
    global gclient
    # 实例化
    gclient = mqtt.Client(client_id=mqtt_client_id)
    gclient.username_pw_set("test", "password")
    # 回调函数
    gclient.on_connect = on_connect
    gclient.on_subscribe = on_subscribe
    gclient.on_message = on_message
    gclient.on_log = on_log
    # host为启动的broker地址 举例本机启动的ip 端口默认1883
    gclient.connect(host="127.0.0.1", port=10883, keepalive=6000)  # 订阅频道
    time.sleep(1)

    # 多个主题采用此方式
    # client.subscribe([("demo", 0), ("test", 2)])      #  test主题，订阅者订阅此主题，即可接到发布者发布的数据

    # 订阅主题 实现双向通信中接收功能，qs质量等级为2
    gclient.subscribe([("hongrui/sw/C171Z1YM000000/rx", 1), ("hongrui/sw/C171Z1YM000000/event", 1), ("hongrui/sw/C171Z1YM000000/period", 1)])
    gclient.loop_start()


def send_mqtt_get():
    global glog 
    global gclient

    # 打开get中的list
    directory_path = "./mqtt_json/get_json/get_list.json"
    with open(directory_path, 'r', encoding='utf-8') as file:
        data = json.load(file)
    # 获取数组中的每个成员
    get_member_list = data.get('data', [])
    get_msg_fmt = """
        {
    "type": "getConfig",
    "msg_id": "100ab",
    "data": [
        "%s"
    ]
    }"""

    for json_string in get_member_list:
        get_msg = get_msg_fmt % json_string
        glog.info(get_msg)
        try:
            # 发布MQTT信息
            # 消息将会发送给代理，并随后从代理发送到订阅匹配主题的任何客户端。
            # publish(topic, payload=None, qos=0, retain=False)
            # topic:该消息发布的主题
            # payload:要发送的实际消息。如果没有给出，或设置为无，则将使用零长度消息。 传递int或float将导致有效负载转换为表示该数字的字符串。 如果你想发送一个真正的int / float，使用struct.pack（）来创建你需要的负载
            # qos:服务的质量级别 对于Qos级别为1和2的消息，这意味着已经完成了与代理的握手。 对于Qos级别为0的消息，这只意味着消息离开了客户端。
            # retain:如果设置为True，则该消息将被设置为该主题的“最后已知良好” / 保留的消息
            gclient.publish(topic="hongrui/sw/C171Z1YM000000/tx", payload=get_msg, qos=2)
            time.sleep(5)
            # i += 1
        except KeyboardInterrupt:
            glog.debug("EXIT")
            # 这是网络循环的阻塞形式，直到客户端调用disconnect（）时才会返回。它会自动处理重新连接。
            gclient.disconnect()

def send_mqtt_set():
    global glog 
    global gclient

    directory_path = "./mqtt_json/get_json/"
    json_dir_instance = JsonDir(directory_path)
    json_dir_instance.read_json_files_as_strings()


    for json_string in json_dir_instance.json_data:
        glog.info(json_string)
        try:
            # 发布MQTT信息
            # 消息将会发送给代理，并随后从代理发送到订阅匹配主题的任何客户端。
            # publish(topic, payload=None, qos=0, retain=False)
            # topic:该消息发布的主题
            # payload:要发送的实际消息。如果没有给出，或设置为无，则将使用零长度消息。 传递int或float将导致有效负载转换为表示该数字的字符串。 如果你想发送一个真正的int / float，使用struct.pack（）来创建你需要的负载
            # qos:服务的质量级别 对于Qos级别为1和2的消息，这意味着已经完成了与代理的握手。 对于Qos级别为0的消息，这只意味着消息离开了客户端。
            # retain:如果设置为True，则该消息将被设置为该主题的“最后已知良好” / 保留的消息
            gclient.publish(topic="hongrui/sw/C171Z1YM000000/tx", payload=json_string, qos=1)
            time.sleep(5)
            # i += 1
        except KeyboardInterrupt:
            glog.debug("EXIT")
            # 这是网络循环的阻塞形式，直到客户端调用disconnect（）时才会返回。它会自动处理重新连接。
            gclient.disconnect()

if __name__ == '__main__':
    #glog = log_init("./mqtt_send.log")
    logfield_path = None
    mqtt_client_id = "hongrui:sw:69f4cf2ee2a61853767b3bdd25e8484b"

    glog = log_init(logfield_path)
    create_client(mqtt_client_id)
    send_mqtt_get()
    while True:
        pass
        time.sleep(3)