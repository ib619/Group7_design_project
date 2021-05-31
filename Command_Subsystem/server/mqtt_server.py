import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
from database import *

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

class MqttServer:
    def __init__(self, ip, port, name, pw):
        self.brokerip = ip
        self.brokerport = port
        self.obstacle_server = paho.Client("obstacleHandler")
        self.obstacle_server.on_connect = self.on_connect_obstacle
        self.obstacle_server.on_publish = self.on_publish_obstacle
        self.obstacle_server.on_message = self.on_message_obstacle
        self.position_server = paho.Client("positionHandler")
        self.position_server.on_connect = self.on_connect_position
        self.position_server.on_publish = self.on_publish_position
        self.position_server.on_message = self.on_message_position
        self.username = name
        self.password = pw

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.obstacle_thread = threading.Thread(name="obstacle-thread", target=self.handle_obstacle, daemon=True)

        # impt variables
        self.rover_id = 1 # testing values
        # self.rover_id = create_trip_record(create_connection('db/marsrover.db'))

    def connect(self):
        try:
            self.obstacle_server.username_pw_set(self.username, self.password)
            self.obstacle_server.connect(self.brokerip, self.brokerport)
            # self.drive_server.username_pw_set(self.username, self.password)
            # self.drive_server.connect(self.brokerip, self.brokerport)
            self.position_server.username_pw_set(self.username, self.password)
            self.position_server.connect(self.brokerip, self.brokerport)
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

    def on_publish_obstacle(self, client, userdata, mid):
        logging.debug("Obstacle data published")

    # callback for getting a messsage on opstacle topics
    def on_message_obstacle(self, client, userdata, msg):
        db = create_connection('db/marsrover.db')
        if msg.topic == 'obstacle/update':
            # new obstacle data from esp 
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format
            record = (data["colour"], data["x"], data["y"], self.rover_id)
            create_obstacle_record(db, record)

    # Position Stuff
    def on_connect_position(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("Position client connected")
            client.subscribe("position/update", qos=1)
            client.subscribe("reset", qos=1)
        else:
            logging.debug("Position failed to connect")

    def on_publish_position(self, client, userdata, mid):
        logging.debug("Path data published")

    # callback for getting a messsage on position topics
    def on_message_position(self, client, userdata, msg):
        db = create_connection('db/marsrover.db')
        if msg.topic == 'position/update':
            # new position data from esp 
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format
            record = (data["x"], data["y"], data["heading"], self.rover_id)
            create_position_record(db, record)

            # Publish path on every new position update 
            path = select_all_positions(db, self.rover_id)
            json_path = json.dumps(path)
            client.publish("path", json_path, qos=1)
        elif msg.topic == 'reset':
            # reset from react 
            self.rover_id = 1
            # ends current trip & starts new trip
            # end_trip(db, self.rover_id)
            # self.rover_id = create_trip_record(db)
        
    # Event Handlers
    def start_server_handler(self):
        logging.debug("Started backend server")
        self.obstacle_server.loop_start()
        # self.drive_server.loop_start()
        self.position_server.loop_start()

    def handle_obstacle(self):
        while True:
            # publish recent 5 obstacles every 5s
            time.sleep(5)
            db = create_connection('db/marsrover.db')
            data = select_top_obstacle(db, 5, self.rover_id)
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