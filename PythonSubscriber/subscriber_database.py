import paho.mqtt.client as mqtt
import pandas as pd
#Create the CSV file.

filepath = "/Users/shehjarsadhu/Desktop"

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe([
                      ("neuroPort/adc1115/voltage0/Device1",1)]
                      )
    # msg_byte_payload = str(msg.payload)
    # msg_byte_payload.replace("b'", "") 
    # msg_string_payload = msg_byte_payload.replace("'", "")
    # print(float(msg_string_payload))
    # print(type(msg.payload))
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    with open(filepath + "/test_live_demo_b8.txt",'a+') as f:
        f.write(str(msg.topic) + "," + str(msg.payload) + "\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("test.mosquitto.org")

client.loop_forever()