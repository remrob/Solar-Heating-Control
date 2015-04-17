#!/usr/bin/python

import ssl
from websocket import websocket
import sys
from pprint import pprint
import json
from threading import Timer


###### begin Bridge ############

sys.path.insert(0, '/usr/lib/python2.7/bridge/')

from bridgeclient import BridgeClient as bridgeclient

bridgeCli = bridgeclient()

### initial switch state is off
bridgeCli.put('switch1','0')

### timer for looping Bridge output
def loopBridge():
        print "Celsius Outdoor"
        print bridgeCli.get('celsiusOutdoor')
        Timer(5.0, loopBridge).start()
Timer(5.0, loopBridge).start()

##### end Bridge ##################

#### begin ws debugTrace ################
print sys.argv

print "or  with for-loop:"

for i in range(len(sys.argv)):
    if i == 0:
        print "Funktion name: %s" % sys.argv[0]
    else:
        print "%d. Argument: %s" % (i,sys.argv[i])

##### end ws debugTrace #################

##### begin Websocket #############

def on_message(ws, message):
        jsonData = json.loads(message)
        print(message)
        print "unit = %s" % jsonData["unit"]
        print (type(jsonData["task"]))
        print "task = %s" % jsonData["task"]

###  REMROB control
        if int(jsonData["unit"]) == 20 and int(jsonData ["task"]) == 30:
          print("switchOn")
          value.put('switch1','1')
          ws.send('{"unit":20,"state":30}')
        elif int(jsonData ["unit"]) == 20 and int(jsonData ["task"]) == 50:
          print("switchOff")
          value.put('switch1','0')
          ws.send('{"unit":20,"state":50}')
        else:
          print "error"

def on_error(ws, error):
        print (error)
#       pprint(vars(ws.sock.sock))

def on_close(ws):
        print ("... closed ...")

def on_open(ws):
        print ("opend")
#       pprint(vars(ws))
        ### show initial state of switch as Off
        ws.send('{"unit":20,"state":50}')

if __name__ == "__main__":
#websocket.enableTrace(True)
    if len(sys.argv) < 2:
#       host = "wss://echo.websocket.org/"
        print("passed")
        host = 'wss://rws.remrob.com/?model=733&id=70&key=70' # default
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
ws.run_forever()

##### end websocket #############

