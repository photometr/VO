#!/usr/bin/python
import pickle
import time
import os
import sys
import dbmanager
from objects import objects
import sys
sys.path.append("/home/eti/install")
#from dropbox import client, rest, session
import dropbox

# Get your app key and secret from the Dropbox developer website
APP_KEY = ''
APP_SECRET = ''
# ACCESS_TYPE should be 'dropbox' or 'app_folder' as configured for your app
ACCESS_TYPE = 'dropbox'
folder = "Larionovy/SPb optical/"
filters = {"B":"B","V":"V","Rc":"R","Ic":"I","Pol":"RP"}

def Log(msg):
  fop = open('synchr.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def OpenSession():
  sess = dropbox.session.DropboxSession(APP_KEY, APP_SECRET, ACCESS_TYPE)
  request_token = sess.obtain_request_token()
  if os.path.exists("tkn.pkl"):
    pkl_file = open('tkn.pkl', 'rb')
    tkn = pickle.load(pkl_file)
    pkl_file.close()
    sess.set_token(tkn["Key"], tkn["Secret"])
    Log("Session opened successfully")
  else:
    Log("Couldn't open session. Retry with -a option.")
    print "Couldn't open session. Retry with -a option."
    sys.exit(1)
  return sess

def CreateSession():
  sess = dropbox.session.DropboxSession(APP_KEY, APP_SECRET, ACCESS_TYPE)
  request_token = sess.obtain_request_token()
  url = sess.build_authorize_url(request_token)
  print "url:", url
  print "Please authorize in the browser. After you're done, press enter."
  raw_input()
  # This will fail if the user didn't visit the above URL and hit 'Allow'
  access_token = sess.obtain_access_token(request_token)
  tkn = {"Key" : str(access_token.key), "Secret" : str(access_token.secret)}
  output = open('tkn.pkl', 'wb')
  pickle.dump(tkn, output)
  output.close()
  Log("Created new token for session")
  print "Now run again without -a option"

def GetRevisDict():
  revisions = {}
  if os.path.exists("revisions.list"):
    fop = open("revisions.list",'r')
    for line in fop.readlines():
      if not line.strip():
	continue
      sl = line.split("|")
      revisions[sl[0]] = int(sl[1])
    fop.close()
  else:
    fop = open("revisions.list",'w')
    fop.close()
    revisions = None
  return revisions

def IsUpdated(fname,session):
  client = dropbox.client.DropboxClient(session)
  try:
    metadata = client.metadata(fname)
  except:
    Log("File " + fname + " is not in the Dropbox")
    return False
  revision = metadata['revision']
  revisiondict = GetRevisDict()
  if revisiondict is None:
    return True
  try:
    revisiondict[fname]
  except:
    return True
  if revisiondict[fname] == revision:
    return False
  else:
    return True

def GenFN(obj,tel,filt):
  preffix = objects[obj]["filepreff"]
  fname = preffix+filters[filt]+".DAT"
  return os.path.join(folder,tel,obj,fname)

def ParseData(data):
  datadict = {}
  lines = data.split("\n")
  for line in lines:
    if line.strip() == "":
      continue
    sl = line.split()
    date = float(sl[0])
    mag = float(sl[1])
    magerr = float(sl[2])
    datadict[date] = (mag,magerr)
  return datadict

def WriteNewRevis(fname,metadata):
  outlist = []
  revision = metadata["revision"]
  fop = open("revisions.list",'r')
  for line in fop.readlines():
    if line.startswith(fname):
      continue
    if not line.strip():
      continue
    outlist.append(line)
  fop.close()
  fop = open("revisions.list",'w')
  for line in outlist:
    fop.write(line+"\n")
  fop.write(fname + "|" + str(revision))
  fop.close()

def UpdateDB(obj,tel,filt,session):
  client = dropbox.client.DropboxClient(session)
  fname = GenFN(obj,tel,filt)
  f, metadata = client.get_file_and_metadata(fname)
  data = ParseData(f.read())
  dbmanager.WriteToDB(objects[obj]["dbname"],tel,filt,data)
  WriteNewRevis(fname,metadata)

def DownloadData(session):
  for obj in objects.keys():
    for tel in ["AZT-8","LX-200"]:
      for filt in filters.keys():
	if IsUpdated(GenFN(obj,tel,filt),session):
	  UpdateDB(obj,tel,filt,session)
      

def main():
  Log("Started synchronization")
  session = OpenSession()
  DownloadData(session)

if __name__ == '__main__':
  if len(sys.argv) > 1 and sys.argv[1] == "-a":
    CreateSession()
  else:
    main()
