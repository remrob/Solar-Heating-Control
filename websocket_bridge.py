#!/usr/bin/python

# Author: Michael Macherey

import logging
import ssl
from websocket import websocket
import sys
import json
from threading import Timer

###### logging #####
logger = logging.getLogger('solarheating')
logger.setLevel(logging.DEBUG)
## create file handler which logs even debug messages
fh = logging.FileHandler('/root/debug.log')
## create console handler with a higher log level
#ch = logging.StreamHandler()
#ch.setLevel(logging.DEBUG)
## create formatter and add it to the handlers
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
#ch.setFormatter(formatter)
fh.setFormatter(formatter)
## add the handlers to logger
#logger.addHandler(ch)
logger.addHandler(fh)

###### begin Bridge ############
sys.path.insert(0, '/usr/lib/python2.7/bridge/')
from bridgeclient import BridgeClient as bridgeclient

# Impulse Powermeter # Outdoor temperature # Flow temperature
# are declared and assigned an intitial value at "on_open()" Function
#previousPowermeterImpulse = 0
#previousTemperature = 0
#previousTemperatureFlow = 0

bridgeCli = bridgeclient()

### timers for looping Bridge output ###
def loopTemperatureOutdoorBridge():
    global previousTemperature
    if ws.sock:
        try:
            currentTemperature = bridgeCli.get('celsiusOutdoor')
        except Exception as e:
            logger.error('Can not read Bridge celsiusOutdoor')
            logger.error(e)
        logger.info("Temperature Outdoor = "+ str(currentTemperature))
        if abs(int(previousTemperature) - int(currentTemperature)) > 1: # absolute d$
            previousTemperature = currentTemperature
            ws.send('{"key":"tempOut","value":'+str(previousTemperature)+'}')
            ws.send('{"variable":"300","value":'+str(previousTemperature)+'}')

    Timer(10.0, loopTemperatureOutdoorBridge).start()

def loopTemperatureFlowBridge():
    global previousTemperatureFlow
    if ws.sock:
        try:
            currentTemperature = bridgeCli.get('celsiusFlow')
        except Exception as e:
            logger.error('Can not read Bridge celsiusFlow')
            logger.error(e)
        logger.info("previousTemperatureFlow: "+str(previousTemperatureFlow)+" Tempe$
        if abs(int(previousTemperatureFlow) - int(currentTemperature)) > 1: # absolu$
            previousTemperatureFlow = currentTemperature
            ws.send('{"key":"tempFlow","value":'+str(previousTemperatureFlow)+'}')
            ws.send('{"variable":"40","value":'+str(previousTemperatureFlow)+'}')

    Timer(10.0, loopTemperatureFlowBridge).start()


def loopImpulseBridge():
    global previousPowermeterImpulse
    if ws.sock:
        try:
            currentPowermeterImpulse = float(bridgeCli.get('PowerMeterImpulse'))
        except Exception as e:
            logger.error('Can not read Bridge PowerMeterImpulse')
            logger.error(e)
        logger.info("Current Powermeter Impulse = " + str(currentPowermeterImpulse))
        kWmin = currentPowermeterImpulse - previousPowermeterImpulse
        if previousPowermeterImpulse != 0:
            ws.send('{"variable":"70","value":'+str(kWmin)+'}')
        previousPowermeterImpulse = currentPowermeterImpulse
    Timer(60.0, loopImpulseBridge).start()  # have to read every 60 minutes

##### begin Websocket #############
def on_message(ws, message):
    global savedTemperature
    jsonData = json.loads(message)
    logger.info("switch = %s" % jsonData["switch"])
    logger.info("task = %s" % jsonData["task"])

    ###  REMROB control
    if int(jsonData["switch"]) == 20 and int(jsonData ["task"]) == 30:
        logger.info("switchOn")
        bridgeCli.put('switch1','1')
        if ws.sock:
            ws.send('{"switch":20,"state":30}')
    elif int(jsonData ["switch"]) == 20 and int(jsonData ["task"]) == 50:
        logger.info("switchOff")
        bridgeCli.put('switch1','0')
        if ws.sock:
            ws.send('{"switch":20,"state":50}')
    else:
        logger.info("error: unknown message received")

def on_error(ws, error):
    logger.error('on_error: '+str(error))
    #pprint(vars(ws.sock.sock))

def on_close(ws):
    logger.info("... closed ...")
    Timer(10.0, startSocket).start()

def on_open(ws):
    # Needed for resending of the data after reconnection, otherwise "off" would stay
    global previousPowermeterImpulse,previousTemperature,previousTemperatureFlow
    previousPowermeterImpulse = 0
    previousTemperature = 0
    previousTemperatureFlow = 0

    logger.info("... opend ...")
    Timer(5.0, loopImpulseBridge).start()
    Timer(10.0, loopTemperatureOutdoorBridge).start()
    Timer(15.0, loopTemperatureFlowBridge).start()
    ### initial switch state is off
    bridgeCli.put('switch1','0')
    ### show initial state of switch as Off
    ws.send('{"switch":20,"state":50}')

ws = websocket.WebSocketApp('wss://objects.remrob.com/?model=733&id=70&key=70',on_me$
ws.on_open = on_open

def startSocket():
        ws.run_forever()

startSocket()
