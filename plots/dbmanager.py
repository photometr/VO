import MySQLdb
import time

def Log(msg):
  fop = open('plot.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def GetBandIds(cursor):
  bands = {}
  sql = "SELECT id,band FROM phot_arch_spbu_bands;"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    bands[line[1]] = line[0]
  return bands

def GetCursor():
  db = MySQLdb.connect(host="localhost", user="", passwd="", db="", charset="latin1")
  cursor = db.cursor()
  return cursor

def GetBlazarIds(cursor):
  blazars = {}
  sql = "SELECT id,name FROM phot_arch_spbu_objects;"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    blazars[line[1]] = line[0]
  return blazars

def GetData(blazar, band):
  cursor = GetCursor()
  blazars = GetBlazarIds(cursor)
  bands = GetBandIds(cursor)
  band_id = bands[band]
  blazar_id = blazars[blazar]
  sql = "SELECT jd,mag,mag_err,tel FROM phot_arch_spbu_data WHERE band_id=%s AND blazar_id=%s;" % (band_id,blazar_id)
  cursor.execute(sql)
  data = cursor.fetchall()
  return data
  
