import mysql.connector
from dotenv import load_dotenv
import os
cmd='''CREATE TABLE products(Id INT AUTO_INCREMENT PRIMARY KEY, Name VARCHAR(255), Price FLOAT, ImgUrl MEDIUMTEXT, Quantity INT)
CREATE TABLE orders(OrderId int AUTO_INCREMENT PRIMARY KEY, Deliver int, Paid int, Collected int)
CREATE TABLE orderItems(OrderId int, ProductId int, Quantity int)'''
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
    mycursor.execute("DROP DATABASE supermarket")
except:
    pass
mycursor.execute("CREATE DATABASE supermarket")
mydb=mysql.connector.connect(
	host=dbhost,
	user=dbuser,
	password=dbpw,
    db="supermarket"
)
mycursor = mydb.cursor()
for text in cmd.split("\n"): 
    mycursor.execute(text)
print("database and fields created")
