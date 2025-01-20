import time
from app import app
from app import getdb
from flask import jsonify,request
import controller
from flask_cors import CORS, cross_origin

@app.route("/update", methods=["POST"])
@cross_origin()
def update():
    items = ["water_level", "moisture_level", "light_level","temperature","humidity", "light_status", "pump_status"]
    results=[]
    count=0
    for i in items:
        item=request.args.get(i)
        if item ==None:
            return (i + " missing from request"), 400
        results.append(str(item))
        count+=1
    conn=getdb()
    cur=conn.cursor()
    req="INSERT INTO sensor_data VALUES(NOW(), "+', '.join(results)+")"
    cur.execute(req)
    conn.commit()
    return "success",200
@app.route("/data", methods=["GET"])
@cross_origin()
def get_sensor_data():
    length=request.args.get("entries")
    query="SELECT * FROM sensor_data ORDER BY timestamp ASC"
    if length!=None:
        query="SELECT * FROM sensor_data ORDER BY timestamp ASC LIMIT "+length
    print(query)
    conn=getdb()
    cur=conn.cursor()
    cur.execute(query)
    result=cur.fetchall()
    final=[]
    items = ["water_level", "moisture_level", "light_level","temperature","humidity", "light_status", "pump_status"]
    for row in result:
        final_row={"timestamp":row[0].strftime("%Y-%m-%d %H:%M")}
        for i in range(1,len(items)):
            final_row[items[i]]=row[i]
        final.append(final_row)

    return jsonify(final),200
