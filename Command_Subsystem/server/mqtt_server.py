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
    def __init__(self, ip, port):
        self.brokerip = ip
        self.brokerport = port
        self.obstacle_server = paho.Client("ObstacleHandler")
    
    def connect():
        pass

def main():
    # add some data to the database for testing
    db = create_connection('db/marsrover.db')

    # # to create database with artificial data
    # colours = ["red", "green", "grey", "blue", "yellow"]
    # for _ in range(10):
    #     for _, colour in enumerate(colours):  
    #         create_obstacle_record(db, (colour, random.randint(1, 3), random.randint(4, 62), time.time()))
    #         time.sleep(1)

    print(select_top_obstacle(db))

if __name__ == "__main__":
    main()