import paho.mqtt.client as paho
import time 
import json 
import threading
import logging
import serial

arduino = serial.Serial('/dev/cu.usbmodem14301', 9600, timeout=.1)

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

        ########## Things to send to MQTT ##########
        self.state_num = 0
        self.SoH_1 = 100 # state of health
        self.SoH_2 = 100
        self.SoH_3 = 100
        self.SoC_1 = 0 # state of charge
        self.SoC_2 = 0
        self.SoC_3 = 0
        self.range = 0 # range in cm
        self.runtime = 0 #remaining time in seconds
        self.error1 = 0 # type of error: (0 no error) (1 overvoltage) (2 undervoltage) (3 )
        self.error2 = 0
        self.error3 = 0
        self.cycle1 = 0
        self.cycle2 = 0
        self.cycle3 = 0

        # Things to receive from MQTT and send to arduino
        self.cmd = 0
        self.x,self.y = 0,0
        self.dist_travelled = 0
        self.drive_status = 0
        self.speed = 0

        # threads 
        self.mqtt_thread = threading.Thread(name="mqtt-thread", target=self.start_server_handler, daemon=True)
        self.energy_thread = threading.Thread(name="energy-thread", target=self.handle_energy, daemon=True)

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
            self.cmd = str(msg.payload.decode("utf-8", "ignore"))
        if msg.topic == 'position/update':
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            self.x = data["x"]
            self.y = data["y"]
        if msg.topic == 'rover/status':
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            print("rover status received" + str(data))
            self.drive_status = int(data["drive_status"])
        if msg.topic == "drive/discrete" or msg.topic == "drive/t2c":
            data = str(msg.payload.decode("utf-8", "ignore"))
            data = json.loads(data)
            self.speed = data["speed"]
            
        #NOTE: write to arduino
        arduino.write((",".join([str(self.cmd), str(self.x), str(self.y), str(self.dist_travelled), str(self.drive_status), str(self.speed)]) + "\n").encode('utf-8'))
        
    ### Event Handlers
    def start_server_handler(self):
        logging.debug("Started energy python script")
        self.energy_client.loop_start()

    def handle_energy(self):
        while True:
            # publish recent 5 obstacles every 5s and saves recent most obstacle value to db
            time.sleep(0.5)

            #NOTE: write from arduino
            data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars

            if data:
                data = data.decode('utf-8')
                print(data) #strip out the new lines for now
                
                # Either
                    # 1, status, SoC1,2,3, range (mm to cm), time (s), error1,2,3
                # OR
                    # 2, SOH1,2,3, cycle1,2,3
                # Ignore
                    # 3,....... (diagnosis)
                if data[0].isdigit():

                    header = int(data[0])
                    # print(header)

                    if header == 1 and (self.SoH_1 != 0 and self.SoH_2 != 0 and self.SoH_3 != 0):
                        current_line = data.split(",")
                        self.state_num = int(current_line[1])
                        self.SoC_1 = int(float(current_line[2]))
                        self.SoC_2 = int(float(current_line[3]))
                        self.SoC_3 = int(float(current_line[4]))
                        self.range = int(float(current_line[5]))
                        self.runtime = int(float(current_line[6]))
                        self.error1 = int(float(current_line[7]))
                        self.error2 = int(float(current_line[8]))
                        self.error3 = int(float(current_line[9]))
          
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

                    if header == 2:
                        current_line = data.split(",")
                        self.SoH_1 = int(float(current_line[1]))
                        self.SoH_2 = int(float(current_line[2]))
                        self.SoH_3 = int(float(current_line[3]))
                        self.cycle1 = int(float(current_line[4]))
                        self.cycle2 = int(float(current_line[5]))
                        self.cycle3 = int(float(current_line[6]))
                    

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