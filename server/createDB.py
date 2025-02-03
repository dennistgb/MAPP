import mysql.connector
from dotenv import load_dotenv
import os
cmd='''CREATE TABLE sensor_data(timestamp DateTime, water_level FLOAT, moisture_level FLOAT, light_level FLOAT, temperature FLOAT, humidity FLOAT, light_status BOOL, pump_status BOOL)
CREATE TABLE settings(light_tresh FLOAT, water_tresh FLOAT)'''
load_dotenv()
dbuser=os.getenv("DBUSER")
dbhost=os.getenv("DBHOST")
dbpw=os.getenv("DBPASSWORD")
print(dbuser, dbhost, dbpw)
mydb=mysql.connector.connect(
	host=dbhost,
	user=dbuser,
	password=dbpw
)
mycursor = mydb.cursor()
try:
    mycursor.execute("DROP DATABASE MAPP")
except:
    pass
mycursor.execute("CREATE DATABASE MAPP")
mydb=mysql.connector.connect(
	host=dbhost,
	user=dbuser,
	password=dbpw,
    db="MAPP"
)
mycursor = mydb.cursor()
mycursor.execute("SHOW DATABASES")
for db in mycursor:
    print(db)

for text in cmd.split("\n"): 
    mycursor.execute(text)
print("database and fields created")
