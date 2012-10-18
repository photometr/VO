#!/usr/bin/python

import dbmanager
import objects
import time
import datetime
import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as tckr
from sidereal import JulianDate, JULIAN_BIAS

band = "Rc"
imagpath = "/srv/lacerta/lc/"
OUR_JULIAN_BIAS = 2400000

def Log(msg):
  fop = open('plot.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def getLast60Days(dates,mags,maxdate):
  outdates = []
  outmags = []
  for i in range(len(dates)):
    if dates[i] >= maxdate-60:
      outdates.append(dates[i])
      outmags.append(mags[i])
  return outdates, outmags

def jd2gd(jd):
  d = JulianDate(OUR_JULIAN_BIAS+jd).datetime()
  gd = d.year
  return gd

def getJDTicks(xstart, xend):
  jdticksarr = []
  gdticksarr = []
  gdstart = JulianDate(OUR_JULIAN_BIAS + xstart).datetime()
  gdend   = JulianDate(OUR_JULIAN_BIAS + xend).datetime()
  gdstart = gdstart.year + 1
  gdend   = gdend.year
  for year in range(gdstart, gdend+1):
    dtyear = datetime.datetime(year,1,1)
    jdyear = JulianDate(0).fromDatetime(dtyear)
    jdticksarr.append(jdyear.j+JULIAN_BIAS-OUR_JULIAN_BIAS)
    gdticksarr.append(str(year))
  return jdticksarr, gdticksarr

def getMax(datesLX,datesAZT):
  #so complicated because we arrays could be empty
  try:
    maxLX = max(datesLX)
  except:
    maxLX = None
  try:
    maxAZT = max(datesAZT)
  except:
    maxAZT = None
  if maxAZT is not None and maxLX is not None:
    return max(maxAZT,maxLX)
  elif maxAZT is not None:
    return maxAZT
  elif maxLX is not None:
    return maxLX
  else:
    return 0

def getMin(datesLX,datesAZT):
  #so complicated because we arrays could be empty
  try:
    minLX = min(datesLX)
  except:
    minLX = None
  try:
    minAZT = min(datesAZT)
  except:
    minAZT = None
  if minAZT is not None and minLX is not None:
    return min(minAZT,minLX)
  elif minAZT is not None:
    return minAZT
  elif minLX is not None:
    return minLX
  else:
    return 0

def plot(objname,objsafename,data):
  datesLX = []
  magsLX = []
  datesAZT = []
  magsAZT = []
  for line in data:
    if line[3] == "LX-200":
      datesLX.append(line[0])
      magsLX.append(line[1])
    else:
      datesAZT.append(line[0])
      magsAZT.append(line[1])
  fig = plt.figure(figsize=(12.6,8))#,dpi=200
  
  ax = fig.add_subplot(211)
  ax.plot(datesLX,magsLX, 'ro',markersize=3)
  ax.plot(datesAZT,magsAZT, 'bo',markersize=3)
  ax.legend((r'LX-200', r'AZT-8'), loc = "best")
  ax.invert_yaxis()
  maxdate = getMax(datesLX,datesAZT)
  mindate = getMin(datesLX,datesAZT)
  ax.set_xlim((mindate-40,maxdate+40))
  ax.xaxis.set_minor_locator(tckr.MultipleLocator(100))
  ax.yaxis.set_minor_locator(tckr.MultipleLocator(0.1))
  plt.ylabel('R (mag)')
  plt.xlabel('JD - 2400000')
  if maxdate > 0:
    updstr = JulianDate(OUR_JULIAN_BIAS + maxdate).datetime().strftime("%d.%m.%y$ $%H:%M")
  else:
    updstr = "NEVER"#datetime.datetime().now().strftime("%d.%m.%y$ $%H:%M")
  t = plt.title(objname+r' $(updated$ $'+updstr+')$')
  t.set_y(1.09)
  plt.subplots_adjust(wspace=0.5, top=0.86)# make a little extra space between the subplots

  #top axis
  ax1 = ax.twiny()
  x1, x2 = ax.get_xlim()
  ax1.set_xlim(x1, x2)
  jdticksarr,gdticksarr = getJDTicks(x1, x2)
  ax1.set_xticks(jdticksarr)
  ax1.set_xticklabels(gdticksarr)

  #second plot
  datesLX60, magsLX60 = getLast60Days(datesLX,magsLX,maxdate)
  datesAZT60, magsAZT60 = getLast60Days(datesAZT,magsAZT,maxdate)
  ax2 = fig.add_subplot(212)
  ax2.plot(datesLX60,magsLX60, 'ro',markersize=3)
  ax2.plot(datesAZT60,magsAZT60, 'bo',markersize=3)
  ax2.invert_yaxis()
  ax2.set_xlim((maxdate-60,maxdate+2))
  y1, y2 = ax2.get_ylim()
  if abs(y2-y1) < 0.1:
    ax2.set_ylim(y1+0.06, y2-0.06) #just for nice look
  ax2.xaxis.set_major_formatter(tckr.FormatStrFormatter('%d'))
  ax2.xaxis.set_minor_locator(tckr.MultipleLocator(1))
  ax2.yaxis.set_minor_locator(tckr.MultipleLocator(0.1))
  plt.ylabel('R (mag)')
  plt.xlabel('JD - 2400000')
  plt.savefig(imagpath + objsafename + "R.png",bbox_inches='tight')
  plt.close(fig)
  return 0
  
def main():
  for objkey in objects.objects.keys():
    objname = objects.objects[objkey]["dbname"]
    data = dbmanager.GetData(objname,band)
    plot(objname,objkey,data)
    time.sleep(0.5)

if __name__ == "__main__":
  main()
