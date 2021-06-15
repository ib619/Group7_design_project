import paho.mqtt.client as paho
import time 
import json 
import threading
import logging

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

class MqttServer:
    def __init__(self, ip, port, name, pw):
        self.brokerip = ip
        self.brokerport = port
        self.test_obstacle_server = paho.Client("testobstacleHandler")
        self.test_obstacle_server.on_connect = self.on_connect_obstacle
        self.test_obstacle_server.on_message = self.on_message_obstacle
        self.test_position_server = paho.Client("testpositionHandler")
        self.test_position_server.on_connect = self.on_connect_position
        self.test_position_server.on_message = self.on_message_position
        self.test_trip_server = paho.Client("testtripHandler")
        self.test_trip_server.on_connect = self.on_connect_trip
        self.test_trip_server.on_message = self.on_message_trip
        self.username = name
        self.password = pw

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.test_thread = threading.Thread(name="test-thread", target=self.start_test, daemon=True)

        # impt variables
        self.print_obstacle = True
        self.print_path = True

    def connect(self):
        try:
            self.test_obstacle_server.username_pw_set(self.username, self.password)
            self.test_obstacle_server.connect(self.brokerip, self.brokerport)
            self.test_position_server.username_pw_set(self.username, self.password)
            self.test_position_server.connect(self.brokerip, self.brokerport)
            self.test_trip_server.username_pw_set(self.username, self.password)
            self.test_trip_server.connect(self.brokerip, self.brokerport)
        except:
            logging.debug("Connection Failed")
            exit(1)

    def threadstart(self):
        self.mqtt_thread.start()
        self.test_thread.start()

    ### MQTT Callbacks
    ## Obstacle Stuff
    def on_connect_obstacle(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("obstacle client connected")
            client.subscribe("obstacle/result", qos=1)
        else:
            logging.debug("Obstacle failed to connect")

    # callback for getting a messsage on opstacle topics
    def on_message_obstacle(self, client, userdata, msg):
        # db = create_connection('db/marsrover.db')
        if self.print_obstacle:
            # new obstacle data from esp
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format\
            logging.debug("New Obstacle data")
            logging.debug(data)

    ## Position Stuff
    def on_connect_position(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("position client connected")
            client.subscribe("path", qos=1)
        else:
            logging.debug("position failed to connect")

    # callback for getting a messsage on position topics
    def on_message_position(self, client, userdata, msg):
        if self.print_path:
            # only add to db if 10th position update
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data) # decode string into json format
            logging.debug("new path data")
            logging.debug(data)
            
    ## Trip Stuff
    def on_connect_trip(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("trip client connected")
            client.subscribe("rover/status", qos=1)
        else:
            logging.debug("trip failed to connect")
        
    # callback for getting a messsage on trip topics
    def on_message_trip(self, client, userdata, msg):
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
        self.test_obstacle_server.loop_start()
        self.test_position_server.loop_start()
        self.test_trip_server.loop_start()

    def start_test(self):
        # starts a new trip
        # publish some obstacle values, and check if top 5 is accurately returned
        # check terminal print screen
        logging.debug("Testing top 5 obstacle functionality")
        red_obstacle = {"colour": "red", "x": 1, "y": 1}
        for _ in range(8):
            time.sleep(1)
            data = json.dumps(red_obstacle)
            self.test_obstacle_server.publish("obstacle/update", data)
            red_obstacle["x"] += 1
            red_obstacle["y"] += 1  
        
        # send yellow obstacle and check if result is still correct
        # test with negative numbers to make sure db supports -ve numbers
        yellow_obstacle = {"colour": "yellow", "x": 1, "y": 1}
        for _ in range(8):
            time.sleep(1)
            data = json.dumps(yellow_obstacle)
            self.test_obstacle_server.publish("obstacle/update", data)
            yellow_obstacle["x"] -= 1
            yellow_obstacle["y"] -= 1  
        self.print_obstacle = False

        # publish some position values, and check if the path is returned accurately
        # check terminal print screen
        logging.debug("Testing path data functionality")
        position = {"x": 0, "y": 0, "heading": 0}
        for _ in range(1, 20):
            time.sleep(1)
            data = json.dumps(position)
            self.test_position_server.publish("position/update", data)
            position["x"] += 1
            position["y"] += 1
        self.print_path = False

        self.print_obstacle = True
        # publish rover status to at base and check if backend restarts obstacle data
        # check that obstacle update resets to only have green
        logging.debug("Rover is back at base")
        rover_status = {"drive_status": 2}
        data = json.dumps(rover_status)
        self.test_trip_server.publish("rover/status", data)

        logging.debug("Sending new set of obstacle data")
        green_obstacle = {"colour": "green", "x": 10, "y": 10}
        for _ in range(5):
            time.sleep(1)
            data = json.dumps(green_obstacle)
            self.test_obstacle_server.publish("obstacle/update", data)
            green_obstacle["x"] += 5
            green_obstacle["y"] += 5
        
        logging.debug("Testing is done!!")
       

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
