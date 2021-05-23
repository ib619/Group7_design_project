import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
import random
from database import create_obstacle_record, create_connection, select_all_obstacles, select_top_obstacle

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

class MqttServer:
    def __init__(self, ip, port, name, pw):
        self.brokerip = ip
        self.brokerport = port
        self.obstacle_server = paho.Client("ObstacleHandler")
        self.obstacle_server.on_connect = self.on_connect_obstacle
        self.obstacle_server.on_message = self.on_message_obstacle
        # self.drive_server = paho.Client("DriveHandler")
        # self.drive_server.on_connect = self.on_connect_drive
        # self.drive_server.on_message = self.on_message_drive
        self.username = name
        self.password = pw

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.obstacle_thread = threading.Thread(name="obstacle-thread", target=self.handle_obstacle, daemon=True)
    
    def connect(self):
        try:
            self.obstacle_server.username_pw_set(self.username, self.password)
            self.obstacle_server.connect(self.brokerip, self.brokerport)
            # self.drive_server.username_pw_set(self.username, self.password)
            # self.drive_server.connect(self.brokerip, self.brokerport)
        except:
            logging.debug("Connection Failed")
            exit(1)

    def threadstart(self):
        self.mqtt_thread.start()
        self.obstacle_thread.start()

    ## MQTT Callbacks
    # Obstacle Stuff
    def on_connect_obstacle(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("Obstacle client connected")
            client.subscribe("obstacle/get", qos=1)
            client.subscribe("obstacle/update", qos=1)
        else:
            logging.debug("Obstacle failed to connect")

    # callback for getting a messsage on opstacle topics
    def on_message_obstacle(self, client, userdata, msg):
        if msg.topic == 'obstacle/update':
            # new obstacle data from esp 
            pass
            
        elif msg.topic == 'obstacle/get':
            res = str(msg.payload.decode("utf-8"))
            db = create_connection('db/marsrover.db')
            if res == "all":
                data = select_all_obstacles(db)
            else:
                data = select_top_obstacle(db, res)
            # array of tuples, (colour, x, y, recentness (1 for most recent))
            json_data = json.dumps(data)
            client.publish("obstacle/result", json_data, qos=1)

    # Drive Stuff
    

    # Event Handlers
    def start_server_handler(self):
        logging.debug("Started backend server")
        self.obstacle_server.loop_start()
        # self.drive_server.loop_start()

    def handle_obstacle(self):
        while True:
            # publish recent 5 obstacles every 5s
            time.sleep(5)
            db = create_connection('db/marsrover.db')
            data = select_top_obstacle(db, 5)
            json_data = json.dumps(data)
            self.obstacle_server.publish("obstacle/result", json_data, qos=1)

def main():
    ip  = "localhost"
    port = 1883
    name = "siting"
    password = "password"

    client = MqttServer(ip, port, name, password)
    client.connect()
    client.threadstart()

    while True:
        pass

if __name__ == "__main__":
    main()