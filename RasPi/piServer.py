import MySQLdb
import time
import datetime
import os


filename = "TMPRecors.mws"
noDatabase= False

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
		raise Exception(e)

def mesurTemp():
	return 25.2

def reciveFromRemote():
	#outT,outH,outL,outP,outR,
	#read from transmiter
	stReci = "18.9;10;50;200.244;0"
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
		except:
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
	except:
		addTmpRecord(date+";"+data)
		noDb=True
	
	return noDb


def main():
	noDatabase= False
##	while True:
	strRec = reciveFromRemote()
	noDatabase =storDB(strRec,noDatabase)



main()
