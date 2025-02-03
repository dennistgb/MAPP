from flask import Flask, jsonify, render_template, request
from flask_cors import CORS
import mysql.connector
import datetime
from flask_cors import CORS, cross_origin

app = Flask(__name__)
CORS(app)

def getdb():
    return mysql.connector.connect(
        host="localhost",
        user="root",
        password="H11amden",
        database="MAPP"
    )

# **📝 Route to Store Sensor Data**
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
# **🌐 Route to Retrieve Data**
@app.route("/data", methods=["GET"])
def get_sensor_data():
    length = request.args.get("entries", default=10, type=int)  # Default 10 latest entries
    query = f"SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT {length}"
    
    conn = getdb()
    cur = conn.cursor()
    cur.execute(query)
    result = cur.fetchall()

    if not result:
        return jsonify([]), 200  # Return empty list if no data is available

    final = []
    items = ["water_level", "moisture_level", "light_level", "temperature", "humidity", "light_status", "pump_status"]

    for row in result:
        final_row = {"timestamp": row[0].strftime("%Y-%m-%d %H:%M:%S")}
        for i in range(1, len(items) + 1):
            final_row[items[i - 1]] = row[i]
        final.append(final_row)

    return jsonify(final), 200
@app.route("/")
def home():
    return render_template("index.html")  # Load the HTML file

# **🚀 Run Flask Server**
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=3000, debug=True)
