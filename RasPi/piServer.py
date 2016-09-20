
import RPi.GPIO as GPIO
import MySQLdb
import time
import datetime
import os
from nrf24 import NRF24
import spidev
import Adafruit_DHT as DHT
import requests
import json
GPIO.setmode(GPIO.BCM)

filename = "TMPRecors.mws"
filenameLog = "WeatherStation.log"

RF_CH=0x53

jsonFile = "/home/pi/card.json"

noDatabase= False
dbServer=""
dbUser=""
dbPassword=""
dbSchema=""


timeSleep=2
pinTemp = 4


stationid = ""
stationPW = ""

def encode(text):
#	return text.encode('utf-8')
	return text
def loadJson():
	dataFile = open(jsonFile,'r')    
    	data = json.load(dataFile)
	dataFile.close()

        global dbServer
        global dbUser
        global dbPassword
        global dbSchema


        global stationid
        global stationPW

	dbServer=encode(data["db_Server"])
	dbUser=encode(data["db_User"])
	dbPassword=encode(data["db_Pw"])
	dbSchema=encode(data["db_Schema"])

	
	stationid=encode(data["Station_Id"])
	stationPW=encode(data["Station_Pw"])



def log(exception):
        date = str(datetime.datetime.now())
        f=open(filenameLog,"a")
        f.write(date+":"+str(exception)+"\n")
	f.close()
try:
	loadJson()
except Exception as e:
        log(e)
        raise(e)

def ptGlobal():
	print "DB Server: "+ str(dbServer)
	print "DB User: " + str(dbUser)
        print "DB PW: "+ str(dbPassword)
        print "DB Schema: " + str(dbSchema)
	print "Site User: " + str(stationid)
	print "Site PW: " + str(stationPW)


def uploadRej(time,tempC,humid):
	temperature= tempC*1.8+32
	timeI = str(time).split(".")[0]
	server = "http://rtupdate.wunderground.com"

	path ="/weatherstation/updateweatherstation.php?ID=" + str(stationid) + "&PASSWORD=" + str(stationPW) + "&dateutc=" + str(timeI) + "&tempf=" + str(temperature) +"&humidity="+str(humid)+ "&softwaretype=RaspberryPi&action=updateraw"
	res = requests.get(server+path)
	if ((int(res.status_code) == 200) & ("success" in res.text)):
		log("Successful Upload Temperature C=" + str(tempC) + " Humid " + str(humid))

	else:
		log("Not Successful Upload Temperature C=" + str(tempC) +" Humid " + str(humid))

def setupReciver():
        pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]
        radioN = NRF24()
        radioN.begin(0, 0,25,18) #set gpio 25 as CE pin
        radioN.setRetries(15,15)
        radioN.setPayloadSize(32)
        radioN.setChannel(RF_CH)
        radioN.setDataRate(NRF24.BR_250KBPS)
        radioN.setPALevel(NRF24.PA_MAX)
        radioN.setCRCLength(NRF24.CRC_8);
        radioN.setAutoAck(1)
	radioN.enableDynamicAck()
        radioN.openWritingPipe(pipes[0])
        radioN.openReadingPipe(1, pipes[1])

        radioN.startListening()
        radioN.stopListening()

        radioN.startListening()
        return radioN

try:
	radio= setupReciver()
except Exception as e:
	log(e)
	GPIO.cleanup()
	raise e

def insert(date,insideT,outT,outH,outL,outP,outR,UVindex,outWs,outWd):
	try:
		uploadRej(date,float(outT),float(outH))
	except Exception as e:
		log(e)
#	ptGlobal()
	db = MySQLdb.connect(dbServer,dbUser,dbPassword,dbSchema)
	cursor = db.cursor()
	sql = "INSERT INTO Record\n "
	sql+= "(date,inside_Temp,out_Temp,out_Hum,out_Light,out_Press,out_Rain,out_WindS,out_WindD,UV_Index)\n "
	sql+= "VALUES\n "
	sql+="('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}');\n".format(date,insideT,outT,outH,outL,outP,outR,outWs,outWd,UVindex)
	try:	
		cursor.execute(sql)	
		db.commit()
		db.close()
	except Exception as e:
		db.rollback()
		db.close()
		log(e)
		raise Exception(e)
	log("Inserted at "+ date)
	

def mesurTemp():
	h,t = DHT.read_retry(DHT.DHT22,pinTemp)
	return round((t/1.0),3)


def reciveFromRemote():
	#outT,outH,outL,outP,outR,UV_Index
    	pipe = [0]
	
	print "Espera receber"
    	while not radio.available(pipe, True):
        	time.sleep(1000/100000.0)
	recv_buffer = []
    	radio.read(recv_buffer)
    	out = ''.join(chr(i) for i in recv_buffer)
    	print "Recived: " +out
	inte = mesurTemp();
	ret = str(inte)+";"+out+";-1;-1"
	print "Registo: " +ret
	return(ret)



def insertTmpS():
	f = open(filename,"r")
	str=""
    	for data  in f:
		data = data.replace("\n","")
		try:
			campos = data.split(';')
        	        insert(campos[0],campos[1],campos[2],campos[3],campos[4],campos[5],campos[6],campos[7],campos[8],campos[9])
		except Exception as e:
			log(e)
			str+=data
			str+="\n"
	f.close()
	f = open(filename, "w")
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
		insert(date,campos[0],campos[1],campos[2],campos[3],campos[4],campos[5],campos[6],campos[7],campos[8])
		noDb=False
	except Exception as e:
		addTmpRecord(date+";"+data)
		noDb=True
		log(e)
	
	return noDb


def main():
	lastInDB=0
	delayRec=60*5 #so aceito registo de 5 em 5 min
	try:
		print("Detalhes")
		radio.printDetails()
		insertTmpS()
	except Exception as e:
		log(e)

	noDatabase= False
	while True:
		strRec = reciveFromRemote()
		current_time = int(round(time.time()))
                if(current_time>lastInDB+delayRec):
                        lastInDB=current_time
			print "OK"
			noDatabase =storDB(strRec,noDatabase)
                else:
                        print "Muito Seguido"
#		time.sleep(900) ##tira medicoes de 15 em 15 tirar quando entrar  ardino 

#while True:
try:
	main()
except:
	GPIO.cleanup()
