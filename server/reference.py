from app import app
from app import getdb
from flask import jsonify,request
import controller
from flask_cors import CORS, cross_origin


# get entire list of products database
# GET /products
@app.route("/products", methods=["GET"])
@cross_origin()
def getProducts():
    return controller.controlGetProducts(),200

# create products given name and amount
@app.route("/newproduct", methods=["POST"])
@cross_origin()
def createProduct():
    items = ["Name", "Price", "ImageUrl","Quantity"]
    results=[]
    count=0
    for i in items:
        item=request.args.get(i)
        if item ==None:
            return (i + " missing from request"), 400
        results.append(item)
        count+=1
    return controller.controlCreateProduct(*results)

    


# edit fields of product database
# requires at least id
# PUT /products?{field to edit}={new value}
@app.route("/products", methods=["PUT"])
@cross_origin()
def editProducts():
    conn=getdb()
    cur=conn.cursor()
    if request.args.get("id")==None:
        return "please specify id", "400"
    Id=request.args.get('id')
    items = ["Name", "Price", "ImageUrl","Quantity"]
    fields= ["Name", "Price", "ImgUrl", "Quantity"]
    finals= {}
    for i in range(len(items)):
        testr=request.args.get(items[i])
        if testr != None:
            finals[fields[i]]=testr
    if len(finals.keys())==0:
        return "Please include changes", 400
    controller.controlEditProduct(Id,finals)
    return {"Status": "Update Success"}, 200
    

# create new order
@app.route('/orders', methods=["POST"])
@cross_origin()
def newOrder():
    body=request.json
    if body ==None:
        return "please include data",400
    if not "Items" in body:
        return "Please include items",400
    if len(body["Items"])==0:
        return "list cannot be empty"
    if not "Deliver" in body:
        return "include whether it should be delivered"
    conn=getdb()
    cur=conn.cursor()
    paid=1 if body["Deliver"]==1 else 0

    req="INSERT INTO orders(Deliver, Paid, Collected) VALUES("+str(body["Deliver"])+", "+str(paid)+", "+str(paid)+")"
    cur.execute(req)
    conn.commit()

    cur.execute("SELECT LAST_INSERT_ID()")
    Id=cur.fetchone()[0]
    for i in body["Items"]:
        req="INSERT INTO orderItems(OrderId, ProductId, Quantity) VALUES("+str(Id)+", "+str(i["itemId"])+", "+str(i["amount"])+")"
        cur.execute(req)
        conn.commit()
        originalamt=controller.controlgetproduct(i["itemId"])[4]
        newamt=originalamt-int(i["amount"])
        controller.controlEditProduct(i["itemId"], {"Quantity":newamt})
    cur.close()
    conn.close()
    return {"orderId":Id}, 200
    
# read orders
@app.route('/orders', methods=["GET"])
@cross_origin()
def getOrders():
    conn=getdb()
    cur=conn.cursor()
    cur.execute("SELECT * FROM orders")
    allOrders=cur.fetchall()
    final = []
    for i in allOrders:
        order={
            "OrderId":i[0],
            "Deliver":i[1],
            "Paid":i[2],
            "Collected":i[3],
            "Items":[]
        }
        itemsl=[]
        req="SELECT * FROM orderItems WHERE OrderId = "+str(i[0])
        cur.execute(req)
        items=cur.fetchall()
        for i in items:
            itemsl.append({"itemId" : i[1], "quantity" : i[2]})
        order["Items"]=itemsl
        final.append(order)
    return final,200
    
	
# confirm order is complete
@app.route('/collected/<Id>',methods=["PUT"])
@cross_origin()
def confirmOrder(Id):
    req=f'UPDATE orders set Collected = 1 where OrderId = {Id}'
    conn=getdb()
    cur=conn.cursor()
    cur.execute(req)
    conn.commit()
    cur.close()
    conn.close()
    return {"status":'Order updated successfully'},200

# confirm order is paid for 
@app.route('/paid/<Id>', methods=["PUT"])
@cross_origin()
def paidOrder(Id):
    req=f'UPDATE orders set Collected = 1 where OrderId = {Id}'
    conn=getdb()
    cur=conn.cursor()
    cur.execute(req)
    conn.commit()
    cur.close()
    conn.close()
    return {"status":'Order updated successfully'},200
