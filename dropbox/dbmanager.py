import MySQLdb
import time

def Log(msg):
  fop = open('synchr.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def GetBandIds(cursor):
  bands = {}
  sql = "SELECT id,band FROM phot_arch_spbu_bands"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    bands[line[1]] = line[0]
  return bands

def GetBlazarIds(cursor):
  blazars = {}
  sql = "SELECT id,name FROM phot_arch_spbu_objects"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    blazars[line[1]] = line[0]
  return blazars

def GetUpdEntries(cursor,obj,tel,filt,data,bands,blazars):
  updatedates = []
  try:
    blazar_id = blazars[obj]
  except:
    Log("Can't handle "+obj+". Not in the DB")
    exit(1)
  try:
    band_id = bands[filt]
  except:
    Log("Can't find "+filt+". Not in the DB")
    exit(1)
  for date in data.keys():
    sql = """SELECT jd FROM phot_arch_spbu_data WHERE jd=%(jd)s AND blazar_id=%(blazar_id)i
    AND band_id=%(band_id)i"""%{"jd" : date, "blazar_id" : blazar_id, "band_id" : band_id}
    cursor.execute(sql)
    resp = cursor.fetchone()
    if resp is not None:
      updatedates.append(date)
  return updatedates

def Insert(db,cursor,data,insentries,blazar_id,band_id,tel):
  fields = []
  for date in data.keys():
    if date in insentries:
      fields.append((1,blazar_id,band_id,date,data[date][0],data[date][1],tel))
  try:
    cursor.executemany("""INSERT INTO phot_arch_spbu_data (creator_id, blazar_id, band_id,
                 jd, mag, mag_err, tel) VALUES (%s, %s, %s, %s, %s, %s, %s)""",
                 fields)
    db.commit()
  except:
    db.rollback()
  return 0

def Update(db,cursor,data,updentries,blazar_id,band_id,tel):
  return 0

def WriteToDB(obj,tel,filt,data):
  db = MySQLdb.connect(host="localhost", user="", passwd="", db="", charset='utf8')
  cursor = db.cursor()
  bands = GetBandIds(cursor)
  blazars = GetBlazarIds(cursor)
  updentries = GetUpdEntries(cursor,obj,tel,filt,data,bands,blazars)
  insentries = []
  for date in data.keys():
    if date not in updentries:
      insentries.append(date)
  blazar_id = blazars[obj]
  band_id = bands[filt]
  Insert(db,cursor,data,insentries,blazar_id,band_id,tel)
  Update(db,cursor,data,updentries,blazar_id,band_id,tel)
