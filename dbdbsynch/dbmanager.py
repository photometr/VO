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

def Replace(db,cursor,data,blazar_id,band_id,tel):
  fields = []
  for date in data.keys():
    fields.append((1,blazar_id,band_id,date,data[date][0],data[date][1],tel))
  try:
    cursor.executemany("""REPLACE INTO phot_arch_spbu_data (creator_id, blazar_id, band_id,
                 jd, mag, mag_err, tel) VALUES (%s, %s, %s, %s, %s, %s, %s)""",
                 fields)
    db.commit()
  except:
    db.rollback()
  return 0

def WriteToDB(obj,tel,filt,data):
  db = MySQLdb.connect(host="localhost", user="", passwd="", db="", charset='latin1')
  cursor = db.cursor()
  bands = GetBandIds(cursor)
  blazars = GetBlazarIds(cursor)
  try:
    blazar_id = blazars[obj]
  except KeyError:
    errmsg = "Object " + obj + "is not in the DB"
    Log(errmsg)
    return 1
  band_id = bands[filt]
  Replace(db,cursor,data,blazar_id,band_id,tel)
