import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
from collections import deque
import serial
import pandas as pd

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

        self.col_names = ["SOC1","SOC2","SOC3","range","runtime"]

        self.discharge_df = pd.read_csv("discharge_demo2.csv", names=self.col_names)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
            print(self.discharge_df)
        self.charge_df = pd.read_csv("charge_demo2.csv", names=self.col_names)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
            print(self.charge_df)

        self.i = 0 # iterator for discharge_df
        self.j1 = 0 # iterator for charge_df
        self.j2 = 0
        self.j3 = 0

        # Global variables
        self.state_num = 0
        self.SoH_1 = 90 # state of health
        self.SoH_2 = 87
        self.SoH_3 = 92
        self.SoC_1 = 0 # state of charge
        self.SoC_2 = 0
        self.SoC_3 = 0
        self.range = 200 # range in cm
        self.runtime = 0 #remaining time in seconds
        self.error1 = 0 # type of error: (0 no error) (1 overvoltage) (2 undervoltage) (3 )
        self.error2 = 0
        self.error3 = 0
        self.cycle1 = 15
        self.cycle2 = 14
        self.cycle3 = 9

        # Things to send
        self.prev_cmd = 0
        self.cmd = 0
        self.x,self.y = 0,0
        self.dist_travelled = 0
        self.drive_status = 0
        self.speed = 0
        self.seconds = 0

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
            time.sleep(1)

            if self.prev_cmd == 2 or self.prev_cmd == 1:
                self.cmd = 0

            if self.seconds == 20:
                self.state_num = 5
                self.error1 = 1
            elif self.i == 0:
                #initial value
                self.SoC_1 = self.discharge_df.at[0,"SOC1"]
                self.SoC_2 = self.discharge_df.at[0,"SOC2"]
                self.SoC_3 = self.discharge_df.at[0,"SOC3"]
                self.range = self.discharge_df.at[self.i,"range"]
                self.runtime = self.discharge_df.at[self.i,"runtime"]
                self.state_num = 0
                self.i += 1
            elif self.cmd == 2:
                # RESET
                self.state_num = 0
                self.error1 = 0
                self.SoC_1 = self.discharge_df.at[self.i,"SOC1"]
                self.SoC_2 = self.discharge_df.at[self.i,"SOC2"]
                self.SoC_3 = self.discharge_df.at[self.i,"SOC3"]
                self.range = self.discharge_df.at[self.i,"range"]
                self.runtime = self.discharge_df.at[self.i,"runtime"]
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
                # self.state_num = 0
                pass

            cell1_res = {
                "battery_level":self.SoC_1, 
                "battery_soh":self.SoH_1, 
                "cycles":self.cycle1,
                "error":self.error1
            }
            json_data = json.dumps(cell1_res)
            self.energy_client.publish("battery/status/cell0", json_data, qos=1)

            cell2_res = {
                "battery_level":self.SoC_2, 
                "battery_soh":self.SoH_2, 
                "cycles":self.cycle2,
                "error":self.error2
            }
            json_data = json.dumps(cell2_res)
            self.energy_client.publish("battery/status/cell1", json_data, qos=1)

            cell3_res = {
                "battery_level":self.SoC_3, 
                "battery_soh":self.SoH_3, 
                "cycles":self.cycle3,
                "error":self.error3
            }
            json_data = json.dumps(cell3_res)
            self.energy_client.publish("battery/status/cell2", json_data, qos=1)

            range_res = {
                "range":self.range/100,
                "time":self.runtime
            }
            json_data = json.dumps(range_res)
            self.energy_client.publish("rover/status/energy", json_data, qos=1)

            status_res = self.state_num
            self.energy_client.publish("battery/status", status_res, qos=1)

            self.prev_cmd = self.cmd
            self.seconds += 1


            """
            # TODO: write from arduino - not implemented for demo file
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