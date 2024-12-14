import paho.mqtt.client as mqtt
import time


# 当代理响应订阅请求时被调用。
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("连接成功")
    print("Connected with result code " + str(rc))


# 当代理响应订阅请求时被调用
def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))


# 当使用使用publish()发送的消息已经传输到代理时被调用。
def on_publish(client, obj, mid):
    print("OnPublish, mid: " + str(mid))


# 当收到关于客户订阅的主题的消息时调用。 message是一个描述所有消息参数的MQTTMessage。
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


# 当客户端有日志信息时调用
def on_log(client, obj, level, string):
    print("Log:" + string)


# 实例化
client = mqtt.Client()
# client.username_pw_set("admin", "password")
# 回调函数
client.on_connect = on_connect
client.on_subscribe = on_subscribe
client.on_message = on_message
client.on_log = on_log
# host为启动的broker地址 举例本机启动的ip 端口默认1883
client.connect(host="127.0.0.1", port=1883, keepalive=6000)  # 订阅频道
time.sleep(1)

# 多个主题采用此方式
# client.subscribe([("demo", 0), ("test", 2)])

# 订阅主题 实现双向通信中接收功能，qs质量等级为2
client.subscribe(("demo", 2))
client.loop_start()

i = 0
while True:
    try:
        # 发布MQTT信息
        sensor_data = "hello word......from topic-test"
        # 消息将会发送给代理，并随后从代理发送到订阅匹配主题的任何客户端。
        # publish(topic, payload=None, qos=0, retain=False)
        # topic:该消息发布的主题
        # payload:要发送的实际消息。如果没有给出，或设置为无，则将使用零长度消息。 传递int或float将导致有效负载转换为表示该数字的字符串。 如果你想发送一个真正的int / float，使用struct.pack（）来创建你需要的负载
        # qos:服务的质量级别 对于Qos级别为1和2的消息，这意味着已经完成了与代理的握手。 对于Qos级别为0的消息，这只意味着消息离开了客户端。
        # retain:如果设置为True，则该消息将被设置为该主题的“最后已知良好” / 保留的消息
        client.publish(topic="test", payload=sensor_data, qos=2)
        time.sleep(5)
        # i += 1
    except KeyboardInterrupt:
        print("EXIT")
        # 这是网络循环的阻塞形式，直到客户端调用disconnect（）时才会返回。它会自动处理重新连接。
        client.disconnect()


