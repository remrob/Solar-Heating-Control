#!/usr/bin/python

import ssl
from websocket import websocket
import sys
from pprint import pprint
import json
from threading import Timer

#### if ws websocket connected #####
wsOnOff = 0

###### print to logfile ########

#old_stdout =sys.stdout

#log_file = open("message.log","w")

#sys.stdout = log_file

#sys.stdout = old_stdout
#log_file.close()

###### begin Bridge ############

sys.path.insert(0, '/usr/lib/python2.7/bridge/')
from bridgeclient import BridgeClient as bridgeclient

# Impulse Powermeter
previousPowermeterImpulse = 0

# Outdoor temperature
previousTemperature = 0

# Flow temperature
previousTemperatureFlow = 0


bridgeCli = bridgeclient()

### initial switch state is off
bridgeCli.put('switch1','0')

### timers for looping Bridge output
def loopTemperatureOutdoorBridge():
        global previousTemperature, wsOnOff
        print("wsOnOff = "+ str(wsOnOff))
        if wsOnOff == 0:
          Timer(5.0, loopTemperatureOutdoorBridge).start()
          return False

        currentTemperature = bridgeCli.get('celsiusOutdoor')
        print("Temperature Outdoor :")
        print(currentTemperature)
        if abs(int(previousTemperature) - int(currentTemperature)) > 1: # absolute d$
            previousTemperature = currentTemperature
            ws.send('{"key":"tempOut","value":'+str(previousTemperature)+'}')
            ws.send('{"variable":"300","value":'+str(previousTemperature)+'}')

        Timer(5.0, loopTemperatureOutdoorBridge).start()

def loopTemperatureFlowBridge():
        global previousTemperatureFlow, wsOnOff
        if wsOnOff == 0:
          Timer(5.0, loopTemperatureFlowBridge).start()
          return False

        currentTemperature = bridgeCli.get('celsiusFlow')
        print("Temperature Flow :")
        print(currentTemperature)
        if abs(int(previousTemperatureFlow) - int(currentTemperature)) > 1: # absolute d$
                previousTemperatureFlow = currentTemperature
                ws.send('{"key":"tempFlow","value":'+str(previousTemperatureFlow)+'}')
                ws.send('{"variable":"40","value":'+str(previousTemperatureFlow)+'}')

        Timer(5.0, loopTemperatureFlowBridge).start()

def loopImpulseBridge():
        global previousPowermeterImpulse, wsOnOff
        if wsOnOff == 0:
          Timer(60.0, loopImpulseBridge).start()
          return False

        currentPowermeterImpulse = float(bridgeCli.get('PowerMeterImpulse'))
        print(currentPowermeterImpulse)
        kWmin = currentPowermeterImpulse - previousPowermeterImpulse
        if previousPowermeterImpulse != 0:
          ws.send('{"variable":"70","value":'+str(kWmin)+'}')
        previousPowermeterImpulse = currentPowermeterImpulse
        Timer(60.0, loopImpulseBridge).start()  # have to read every 60 minutes

Timer(1.0, loopTemperatureOutdoorBridge).start()
Timer(1.5, loopTemperatureFlowBridge).start()
Timer(2.0, loopImpulseBridge).start()


##### end Bridge ##################

#### begin ws debugTrace ################
print sys.argv

for i in range(len(sys.argv)):
    if i == 0:
        print "Funktion name: %s" % sys.argv[0]
    else:
        print "%d. Argument: %s" % (i,sys.argv[i])

##### end ws debugTrace #################

##### begin Websocket #############

def on_message(ws, message):
        global savedTemperature
        jsonData = json.loads(message)
        print "switch = %s" % jsonData["switch"]
        print (type(jsonData["task"]))
        print "task = %s" % jsonData["task"]

###  REMROB control
        if int(jsonData["switch"]) == 20 and int(jsonData ["task"]) == 30:
          print("switchOn")
          bridgeCli.put('switch1','1')
          if ws:
           ws.send('{"switch":20,"state":30}')
        elif int(jsonData ["switch"]) == 20 and int(jsonData ["task"]) == 50:
          print("switchOff")
          bridgeCli.put('switch1','0')
          if ws:
           ws.send('{"switch":20,"state":50}')
        else:
          print "error"

def on_error(ws, error):
        print (error)
#       pprint(vars(ws.sock.sock))

def on_close(ws):
        global wsOnOff
        wsOnOff = 0
        print ("... closed ...")
        pprint(vars(ws.sock))
        #sys.stdout = old_stdout
        Timer(10.0, startSocket).start()
        #log_file.close()

def on_open(ws):
        global wsOnOff
        wsOnOff = 1
#       print(ws)
        print ("opend")
        pprint(vars(ws.sock))
        ### show initial state of switch as Off
        ws.send('{"switch":20,"state":50}')

if __name__ == "__main__":
# websocket.enableTrace(True)
    if len(sys.argv) < 2:
#       host = "wss://echo.websocket.org/"
        print("passed")
        host = 'wss://objects.remrob.com/?model=733&id=70&key=70' # default
    else:
        host = sys.argv[1]

ws = websocket.WebSocketApp(host,
                               on_message = on_message,
                                on_error = on_error,
                                on_close = on_close)

#ws = websocket.WebSocketApp("wss://echo.websocket.org")

ws.on_open = on_open
#    ws.on_message = on_message
#    ws.on_error = on_error
#    ws.on_close = on_close

def startSocket():
        global ws
        ws.run_forever()

startSocket()

##### end websocket #############
