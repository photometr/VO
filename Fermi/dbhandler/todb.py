#!/usr/bin/python

import sys
import MySQLdb
import time
import objects

objname = sys.argv[1]

spinderr = {"S50716"  : 0.02143,
            "S40954"  : 0.06723,
            "CTA102"  : 0.06691,
            "3C454"   : 0.00806,
            "BLLac"   : 0.03569,
            "Q1633"   : 0.02759,
            "PKS1510" : 0.01428,
            "3C66A"   : 0.02386,
            "WCOM"    : 0.03435,
            "3C273"   : 0.02185,
            "S41030"  : 0.03146,
            "3C345"   : 0.05606,
            "OT081"   : 0.06486,
            "PKS0420" : 0.02840,
            "Q1156"   : 0.02703,
            "Q0836"   : 0.07301,
            }

class point():
  def __init__(self):
    self.MJD = 0
    self.Flux = 0
    self.Flux_Err = 0
    self.Index = 0
    self.IdxErr = 0
    self.UpperLimit = 0
    self.TS_VALUE = 0
    self.Prefactor = 0
    self.Pref_Err = 0
    self.T_START = 0
    self.T_STOP = 0
    self.NPRED = 0
  def setSpIndErr(self,spidxerr):
    if spidxerr < 1e-6:
      self.IdxErr = spinderr[objname]

def Log(msg):
  fop = open(sys.argv[0]+'.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def parsefile():
  data = []
  fop = open(objname+".dat")
  for line in fop.readlines():
    p = point()
    if line.startswith("#"):
      continue
    if line.strip() == "":
      continue
    sl = line.split()
    if sl[2] == "ERROR!":
      continue
    if sl[6] == "Error":
      continue
    p.MJD        = float(sl[1])
    p.Flux       = float(sl[2])
    p.Flux_Err   = float(sl[3])
    p.Index      = float(sl[4])
    p.setSpIndErr( float(sl[5]) )
    if sl[6] == "Ignored":
      p.UpperLimit = None
    else:
      p.UpperLimit = float(sl[6])
    p.TS_VALUE   = float(sl[7])
    p.Prefactor  = float(sl[8])
    p.Pref_Err   = float(sl[9])
    p.T_START    = float(sl[10])
    p.T_STOP     = float(sl[11])
    p.NPRED      = float(sl[12])
    data.append(p)
  fop.close()
  return data

def GetBlazarIds(cursor):
  blazars = {}
  sql = "SELECT id,name FROM Fermi_objects"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    blazars[line[1]] = line[0]
  return blazars

def Replace(db,cursor,data,blazar_id):
  fields = []
  for p in data:
    fields.append((blazar_id,p.MJD,p.Flux,p.Flux_Err,p.Index,p.IdxErr,p.UpperLimit,p.TS_VALUE,p.Prefactor,p.Pref_Err,p.T_START,p.T_STOP,p.NPRED))

  try:
    cursor.executemany("""REPLACE INTO Fermi_data (blazar_id, jd, flux, flux_err, ind, index_err, upp_lim, ts_value, prefactor, pref_err, t_start, t_stop,npred) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", fields)
    db.commit()
  except:
    db.rollback()
    errmsg = "Smth wrong on update of " + objname
    Log(errmsg)
    return 1
  return 0

def writeToDB(data):
  db = MySQLdb.connect(host="localhost", user="", passwd="", db="", charset='latin1')
  cursor = db.cursor()
  blazars = GetBlazarIds(cursor)
  try:
    blazar_id = blazars[objects.objects[objname.lower()]["dbname"]]
  except KeyError:
    errmsg = "Object " + objname + " is not in the DB"
    Log(errmsg)
    return 1
  Replace(db,cursor,data,blazar_id)

if __name__ == "__main__":
  data = parsefile()
  writeToDB(data)
