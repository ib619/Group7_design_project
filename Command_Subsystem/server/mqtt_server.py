import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
from database import *
from collections import deque

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
        self.trip_server = paho.Client("tripHandler")
        self.trip_server.on_connect = self.on_connect_trip
        self.trip_server.on_publish = self.on_publish_trip
        self.trip_server.on_message = self.on_message_trip
        self.username = name
        self.password = pw

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.obstacle_thread = threading.Thread(name="obstacle-thread", target=self.handle_obstacle, daemon=True)

        # impt variables
        # self.rover_id = 1 # testing values
        self.counter = 0
        self.rover_id = create_trip_record(create_connection('db/marsrover.db'))
        self.obstacle_record = {
            "red": deque([]), 
            "green": deque([]), 
            "yellow": deque([]), 
            "blue": deque([]), 
            "grey": deque([])
        }

    def connect(self):
        try:
            self.obstacle_server.username_pw_set(self.username, self.password)
            self.obstacle_server.connect(self.brokerip, self.brokerport)
            self.position_server.username_pw_set(self.username, self.password)
            self.position_server.connect(self.brokerip, self.brokerport)
            self.trip_server.username_pw_set(self.username, self.password)
            self.trip_server.connect(self.brokerip, self.brokerport)
        except:
            logging.debug("Connection Failed")
            exit(1)

    def threadstart(self):
        self.mqtt_thread.start()
        self.obstacle_thread.start()

    ### MQTT Callbacks
    ## Obstacle Stuff
    def on_connect_obstacle(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("obstacle client connected")
            client.subscribe("obstacle/update", qos=1)
        else:
            logging.debug("Obstacle failed to connect")

    def on_publish_obstacle(self, client, userdata, mid):
        logging.debug("obstacle data published")

    # callback for getting a messsage on opstacle topics
    def on_message_obstacle(self, client, userdata, msg):
        # db = create_connection('db/marsrover.db')
        if msg.topic == 'obstacle/update':
            # new obstacle data from esp
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format

            # add data into deque on the left, if more than 5: remove from right
            self.obstacle_record[data["colour"]].appendleft([data["colour"], data["x"], data["y"]])
            if len(self.obstacle_record[data["colour"]]) > 5:
                self.obstacle_record[data["colour"]].pop()
            # record = (data["colour"], data["x"], data["y"], self.rover_id)
            # create_obstacle_record(db, record)

    ## Position Stuff
    def on_connect_position(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("position client connected")
            client.subscribe("position/update", qos=1)
        else:
            logging.debug("position failed to connect")

    def on_publish_position(self, client, userdata, mid):
        logging.debug("path data published")

    # callback for getting a messsage on position topics
    def on_message_position(self, client, userdata, msg):
        db = create_connection('db/marsrover.db')
        if msg.topic == 'position/update':
            if self.counter == 5:
                # only add to db if 10th position update
                data = str(msg.payload.decode("utf-8", "ignore"))
                data = json.loads(data) # decode string into json format
                record = (data["x"], data["y"], data["heading"], self.rover_id)
                create_position_record(db, record)

                self.counter = 0
            else:
                self.counter += 1
            
            # Publish path on every new position update 
            path = select_all_positions(db, self.rover_id)
            json_path = json.dumps(path)
            client.publish("path", json_path, qos=1)
        
    ## Trip Stuff
    def on_connect_trip(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("trip client connected")
            client.subscribe("rover/status", qos=1)
        else:
            logging.debug("trip failed to connect")

    def on_publish_trip(self, client, userdata, mid):
        logging.debug("trip published")

    # callback for getting a messsage on trip topics
    def on_message_trip(self, client, userdata, msg):
        db = create_connection('db/marsrover.db')
        if msg.topic == 'rover/status':
            # rover status update
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format
            if data["drive_status"] == 2:
                # rover is back at base, start a new trip and reset local obstacle data
                self.rover_id = create_trip_record(db)
                self.obstacle_record = {
                    "red": deque([]), 
                    "green": deque([]), 
                    "yellow": deque([]), 
                    "blue": deque([]), 
                    "grey": deque([])
                }
        
    ### Event Handlers
    def start_server_handler(self):
        logging.debug("Started backend server")
        self.obstacle_server.loop_start()
        self.position_server.loop_start()
        self.trip_server.loop_start()

    def handle_obstacle(self):
        while True:
            # publish recent 5 obstacles every 5s and saves recent most obstacle value to db
            time.sleep(1)
            db = create_connection('db/marsrover.db')
            # data = select_top_obstacle(db, 5, self.rover_id)

            # clean up data for easy reading on react side
            res = []
            for _, item in self.obstacle_record.items():
                item_copy = list(item).copy()
                for i, data in enumerate(item_copy):
                    res.append(data+[i+1])
                    if i == 0:
                        create_obstacle_record(db, data+[self.rover_id])
    
            json_data = json.dumps(res)
            self.obstacle_server.publish("obstacle/result", json_data, qos=1)

def main():
    ip  = "localhost"
    port = 1883
    name = "admin"
    password = "marsrover"

    client = MqttServer(ip, port, name, password)
    client.connect()
    client.threadstart()

    while True:
        pass

if __name__ == "__main__":
    main()
