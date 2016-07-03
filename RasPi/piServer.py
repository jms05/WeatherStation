import RPi.GPIO as GPIO
import MySQLdb
import time
import datetime
import os
from lib_nrf24 import NRF24
import spidev
import Adafruit_DHT as DHT

GPIO.setmode(GPIO.BCM)
filename = "TMPRecors.mws"
filenameLog = "WeatherStation.log"
noDatabase= False
radio = None
timeSleep=2
pinTemp = 4

def log(exception):
	date = str(datetime.datetime.now())
	f=open(filenameLog,"a")
        f.write(date+":"+str(exception)+"\n")
        f.close()

def setupReciver():
	pipes = [[0xE8, 0xE8, 0xF0, 0xF0, 0xE1], [0xF0, 0xF0, 0xF0, 0xF0, 0xE1]]
	radio = NRF24(GPIO,spidev.SpiDev())
	radio.begin(0,17)
	
	radio.setPayloadSize(32)
	radio.setChannel(0x76)
	radio.setDataRate(NRF24.BR_250KBPS)
	radio.setPALevel(NRF.PA_MAX)
	
	radio.setAutoAck(True)
	radio.enableDynamicPayloads()
	radio.enableAckPayload()
	radio.openReadingPipe(1,pipes[1]) 
	radio.statrListening()
#	return radio

def insert(date,insideT,outT,outH,outL,outP,outR,outWs,outWd):
	db = MySQLdb.connect("192.168.1.192","raspi","Password","WeatherStationDB" )
	cursor = db.cursor()
	sql = "INSERT INTO Record\n "
	sql+= "(date,inside_Temp,out_Temp,out_Hum,out_Light,out_Press,out_Rain,out_WindS,out_WindD)\n "
	sql+= "VALUES\n "
	sql+="('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}');\n".format(date,insideT,outT,outH,outL,outP,outR,outWs,outWd)
	try:	
		cursor.execute(sql)	
		db.commit()
		db.close()
	except Exception as e:
		db.rollback()
		db.close()
		log(e)
		raise Exception(e)

	

def mesurTemp():
	h,t = DHT.read_retry(DHT.DHT22,pinTemp)
	#return t
	return 25.2

def reciveFromRemote():
	#outT,outH,outL,outP,outR,
	#read from transmiter
	pipe = [0]
	while not radio.available(pipe, True):
		time.sleep(timeSleep)
		
	recivedM=[]
	radio.read(recivedM,radio.getDynamicPayloadSize())
	
	stReci=""
	for b in recivedM:
		if (b>=32 and b<=126): #ver isto aqui
			stReci+=chr(b)

#	stReci = "18.9;10;50;200.244;0"
	inte = mesurTemp();
	ret = str(inte)+";"+stReci+";-1;-1"
	return(ret)



def insertTmpS():
	f = open(filename,"r")
	str=""
    	for data  in f:
		data = data.replace("\n","")
		try:
			campos = data.split(';')
        	        insert(campos[0],campos[1],campos[2],campos[3],campos[4],campos[5],campos[6],campos[7],campos[8])
		except Exception as e:
			log(e)
			str+=data
			str+="\n"
	f.close()
	f = open(fName, "w")
	if(not(str is "")):
		lines = str.split("\n")
		for line in lines:
			f.write(line+"\n")
	f.close()


def addTmpRecord(data):
	f=open(filename,"a")
	f.write(data+"\n")
	f.close()

def storDB(data,NoDB):
	data = data.replace("\n","")
	date = str(datetime.datetime.now())
	if NoDB:
		insertTmpS()
	try:
		campos = data.split(';')			
		insert(date,campos[0],campos[1],campos[2],campos[3],campos[4],campos[5],campos[6],campos[7])
		noDb=False
	except Exception as e:
		addTmpRecord(date+";"+data)
		noDb=True
		log(e)
	
	return noDb


def main():
	try:
		insertTmpS()
	except Exception as e:
		log(e)

	noDatabase= False
	
	try:
		setupReciver()
	except Exception as e:
		log(e)
		raise e
##	while True:
	strRec = reciveFromRemote()
	noDatabase =storDB(strRec,noDatabase)



main()
