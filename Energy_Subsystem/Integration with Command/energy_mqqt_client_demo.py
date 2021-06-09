import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
from collections import deque
import serial
import pandas as pd
import os

# arduino = serial.Serial('/dev/cu.usbmodem1461301', 9600, timeout=.1)

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

class MqttServer:
    def __init__(self, ip, port, name, pw):
        self.brokerip = ip
        self.brokerport = port
        self.energy_client = paho.Client("energyHandler")
        self.energy_client.on_connect = self.on_connect_energy
        self.energy_client.on_publish = self.on_publish_energy
        self.energy_client.on_message = self.on_message_energy
        self.username = name
        self.password = pw

        self.col_names = ["SOC1","SOC2","SOC3","range"]

        self.discharge_df = pd.read_csv("discharge_demo.csv", names=self.col_names)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
            print(self.discharge_df)
        self.charge_df = pd.read_csv("charge_demo.csv", names=self.col_names)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
            print(self.charge_df)

        self.i = 0 # iterator for discharge_df
        self.j1 = 0 # iterator for charge_df
        self.j2 = 0
        self.j3 = 0

        # Global variables
        self.state_num = 0
        self.SoH_1 = 94
        self.SoH_2 = 96
        self.SoH_3 = 89
        self.SoC_1 = 0
        self.SoC_2 = 0
        self.SoC_3 = 0
        self.range = 200

        # Things to send
        self.prev_cmd = 0
        self.cmd = 0
        self.x,self.y = 0,0
        self.dist_travelled = 0
        self.drive_status = 0
        self.speed = 0

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.energy_thread = threading.Thread(name="obstacle-thread", target=self.handle_energy, daemon=True)

    def connect(self):
        try:
            self.energy_client.username_pw_set(self.username, self.password)
            self.energy_client.connect(self.brokerip, self.brokerport)
        except:
            logging.debug("Connection Failed")
            exit(1)

    def threadstart(self):
        self.mqtt_thread.start()
        self.energy_thread.start()

    ### MQTT Callbacks
    ## Obstacle Stuff
    def on_connect_energy(self, client, obj, flags, rc):
        if rc == 0:
            logging.debug("energy client connected")
            client.subscribe("battery/command", qos=1) # for reset and recalibrate
            client.subscribe("rover/status", qos=1) # for distance travelled (mm) and drive status (at base)
            client.subscribe("position/update", qos=1) # for x,y
            client.subscribe("drive/discrete ", qos=1)
            client.subscribe("drive/t2c ", qos=1)
        else:
            logging.debug("Energy failed to connect")

    def on_publish_energy(self, client, userdata, mid):
        pass #logging.debug("energy data published")

    # callback for getting a messsage on opstacle topics
    def on_message_energy(self, client, userdata, msg):
        if msg.topic == 'battery/command':
            # new data from esp
            self.cmd = int(str(msg.payload.decode("utf-8", "ignore")))
        if msg.topic == 'position/update':
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            self.x = int(data["x"])
            self.y = int(data["y"])
        if msg.topic == 'rover/status':
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            print(data)
            self.drive_status = int(data["drive_status"])
        if msg.topic == "drive/discrete" or msg.topic == "drive/t2c":
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            self.speed = int(data["speed"])

        """ 
        # TODO: write to arduino
        arduino.write((",".join([str(self.cmd), str(self.x), str(self.y), str(self.dist_travelled), str(self.drive_status), str(self.speed)]) + "\n").encode('utf-8'))
        """

    ### Event Handlers
    def start_server_handler(self):
        logging.debug("Started backend server")
        self.energy_client.loop_start()

    def handle_energy(self):
        while True:
            # publish recent 5 obstacles every 5s and saves recent most obstacle value to db
            time.sleep(1)

            if self.prev_cmd == 2 or self.prev_cmd == 1:
                self.cmd = 0

            if self.i == 0:
                #initial value
                self.SoC_1 = self.discharge_df.at[0,"SOC1"]
                self.SoC_2 = self.discharge_df.at[0,"SOC2"]
                self.SoC_3 = self.discharge_df.at[0,"SOC3"]
                self.range = self.discharge_df.at[self.i,"range"]
                self.state_num = 0
                self.i += 1
            elif self.cmd == 2:
                # RESET
                self.state_num = 0
                self.SoC_1 = self.discharge_df.at[self.i,"SOC1"]
                self.SoC_2 = self.discharge_df.at[self.i,"SOC2"]
                self.SoC_3 = self.discharge_df.at[self.i,"SOC3"]
                self.range = self.discharge_df.at[self.i,"range"]
                self.i += 1
                print("Reset only temporarily sets the machine back into idle")
            elif self.cmd == 1:
                print("Recalibration not implemented")
            elif self.drive_status == 2:
                self.state_num = 1

                #CHARGE
                if self.j1 == 0:
                    self.j1 = abs(self.charge_df['SOC1']-self.SoC_1).idxmin()
                self.SoC_1 = self.charge_df.at[self.j1,"SOC1"]
                self.j1 += 1

                if self.j2 == 0:
                    self.j2 = abs(self.charge_df['SOC2']-self.SoC_2).idxmin()
                self.SoC_2 = self.charge_df.at[self.j2,"SOC2"]
                self.j2 += 1

                if self.j3 == 0:
                    self.j3 = abs(self.charge_df['SOC3']-self.SoC_3).idxmin()
                self.SoC_3 = self.charge_df.at[self.j3,"SOC3"]
                self.range = self.charge_df.at[self.i,"range"]
                self.j3 += 1

            elif self.drive_status == 1:
                self.state_num = 8

                #DISCHARGE
                self.SoC_1 = self.discharge_df.at[self.i,"SOC1"]
                self.SoC_2 = self.discharge_df.at[self.i,"SOC2"]
                self.SoC_3 = self.discharge_df.at[self.i,"SOC3"]
                self.range = self.discharge_df.at[self.i,"range"]
                self.i += 1
            elif self.drive_status == 0:
                #IDLE (freeze values)
                self.state_num = 0

            cell1_res = {
                "cell":0, 
                "battery_level":self.SoC_1, 
                "battery_soh":self.SoH_1, 
                "battery_state":self.state_num
            }
            json_data = json.dumps(cell1_res)
            self.energy_client.publish("battery/status", json_data, qos=1)

            cell2_res = {
                "cell":1, 
                "battery_level":self.SoC_2, 
                "battery_soh":self.SoH_2, 
                "battery_state":self.state_num
            }
            json_data = json.dumps(cell2_res)
            self.energy_client.publish("battery/status", json_data, qos=1)

            cell3_res = {
                "cell":2, 
                "battery_level":self.SoC_3, 
                "battery_soh":self.SoH_3, 
                "battery_state":self.state_num
            }
            json_data = json.dumps(cell3_res)
            self.energy_client.publish("battery/status", json_data, qos=1)

            range_res = {
                "range":self.range
            }
            json_data = json.dumps(range_res)
            self.energy_client.publish("rover/status/range", json_data, qos=1)

            self.prev_cmd = self.cmd


            """
            # TODO: write from arduino
            data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars

            if data:
                data = data.decode('utf-8')
                print(data) #strip out the new lines for now
                
                # Either
                    # 1, status, SoC1,2,3, range (cm)
                # OR
                    # 2, SOH1,2,3, 
                # Ignore
                    # 3,....... (diagnosis)
                if data[0].isdigit():

                    header = int(data[0])
                    # print(header)

                    if header == 1 and (self.SoH_1 != 0 and self.SoH_2 != 0 and self.SoH_3 != 0):
                        current_line = data.split(",")
                        state_num = int(current_line[1])
                        self.SoC_1 = int(float(current_line[2]))
                        self.SoC_2 = int(float(current_line[3]))
                        self.SoC_3 = int(float(current_line[4]))
                        self.range = int(float(current_line[5]))
          
                    if header == 2:
                        current_line = data.split(",")
                        self.SoH_1 = int(float(current_line[1]))
                        self.SoH_2 = int(float(current_line[2]))
                        self.SoH_3 = int(float(current_line[3]))
        """            

def main():
    # ip  = "35.177.181.61"
    ip = "localhost"
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