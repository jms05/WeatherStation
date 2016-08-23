import RPi.GPIO as GPIO
import MySQLdb
import time
import datetime
import os
from nrf24 import NRF24
import spidev
import Adafruit_DHT as DHT

GPIO.setmode(GPIO.BCM)
filename = "TMPRecors.mws"
filenameLog = "WeatherStation.log"
noDatabase= False
dbServer="ServerIP"
dbUser="UserID"
dbPassword="UserPassword"
dbSchema"dvSchema"

def setupReciver():
        pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]
        radioN = NRF24()
        radioN.begin(0, 0,25,18) #set gpio 25 as CE pin
        radioN.setRetries(15,15)
        radioN.setPayloadSize(32)
        radioN.setChannel(0x4c)
        radioN.setDataRate(NRF24.BR_250KBPS)
        radioN.setPALevel(NRF24.PA_MAX)
        radioN.setCRCLength(NRF24.CRC_8);
        radioN.setAutoAck(1)
        radioN.openWritingPipe(pipes[0])
        radioN.openReadingPipe(1, pipes[1])

        radioN.startListening()
        radioN.stopListening()

#        radioN.printDetails()
        radioN.startListening()
        return radioN




radio = setupReciver()
timeSleep=2
pinTemp = 4

def log(exception):
	date = str(datetime.datetime.now())
	f=open(filenameLog,"a")
        f.write(date+":"+str(exception)+"\n")
        f.close()

def insert(date,insideT,outT,outH,outL,outP,outR,outWs,outWd):
	db = MySQLdb.connect(dbServer,dbUser,dbPassword,dbSchema)
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
	return round((t/1.0),3)

def mesurTempS():
        h,t = DHT.read_retry(DHT.DHT22,pinTemp)
        return str(t) + "*C " + str(h)


def reciveFromRemote():
	#outT,outH,outL,outP,outR,
	#read from transmiter
    	pipe = [0]
	print "Espera receber"
    	while not radio.available(pipe, True):
        	time.sleep(1000/100000.0)
    	recv_buffer = []
    	radio.read(recv_buffer)
    	out = ''.join(chr(i) for i in recv_buffer)
    	print "Recived:" +out
	#lico =raw_input('-->')
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
		print("Detalhes")
		radio.printDetails()
		insertTmpS()
	except Exception as e:
		log(e)

	noDatabase= False
	'''
	try:
		setupReciver()
	except Exception as e:
		log(e)
		raise e
	'''
	while True:
		strRec = reciveFromRemote()
		noDatabase =storDB(strRec,noDatabase)
#		time.sleep(900) ##tira medicoes de 15 em 15 tirar quando entrar  ardino 


main()

#a=0
#while True:
#	date = str(datetime.datetime.now())
#	tI = round(mesurTemp(),2)
#	print "Em "+date+" TMP: " + str(tI)
#	insert(date,tI,-999,-999,-999,-999,-999,-999,-999)
#	print "inserido Registo"
#	time.sleep(900) #regita de 15 em 15
