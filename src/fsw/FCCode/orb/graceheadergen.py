# 11 APR 2020
# graceheadergen.py
# Nathan Zimmerberg (nhz2@cornell.edu)
#This is a simple script to download GRACE FO data files and turn it into a header file
#To run, use command "python graceheadergen.py", add -h flag for help

import requests
import gzip

def downloaddata(year,month,day,sat):
    """ Returns a list of bytes of the data from gracefo on year,month,day.
        sat is either "C" or "D" """
    url= "https://podaac-tools.jpl.nasa.gov/drive/files/allData/gracefo/L1B/JPL/RL04/ASCII/%d/gracefo_1B_%d-%s-%s_RL04.ascii.noLRI.tgz"%(year,year,str(month).zfill(2),str(day).zfill(2))
    print(url)
    print('\nPlease wait while downloading grace data')
    r= requests.get(url,auth=('nhz2','8dDTxE007RQMZBHFngX'))
    s= gzip.decompress(r.content)
    file_start= s.find(b'GNV1B_%d-%s-%s_%s_04.txt'%(year,str(month).zfill(2).encode('utf-8'),str(day).zfill(2).encode('utf-8'),sat.encode('utf-8')))
    s=s[file_start:]
    data_start= s.find(b'End of YAML header')
    s= s[data_start+18:]
    data_end= s.find(b'\x00')
    s= s[1:data_end-1]
    return s.split(b'\n')


def main(headerfilename, year, month, day,sat):
    """download GRACE FO data files and turn it into a header file
    gracefo on year,month,day.
        sat is either "C" or "D" 

    The data is three arrays
    constexpr uint64_t GRACEGPSTIMENS[]
    constexpr double GRACEPOS[][3]
    constexpr double GRACEVEL[][3]
    Args:
        infilename(filename): the .COF file that contains the
            spherical harmonic coefficents, download this from http://www2.csr.utexas.edu/grace/gravity/
        headerfilename(string ending in .hpp or .h): the c++ header file."""
    data= downloaddata(year,month,day,sat)
    outstr = '#pragma once\n#include <cstdint>\n//file generated from GRACE FO sat %s on %d-%s-%s, by graceheadergen.py\n'%(sat,year,str(month).zfill(2),str(day).zfill(2))
    #GRACEGPSTIMENS
    outstr += '\nconstexpr uint64_t GRACEGPSTIMENS[]={'
    for line in data:
        l= line.decode('utf-8').split(' ')
        outstr+= str((int(l[0])+630763200)*1000000000)+','
    outstr= outstr[:-1]+'};'
    #GRACEPOS
    outstr += '\nconstexpr double GRACEPOS[][3]={'
    for line in data:
        l= line.decode('utf-8').split(' ')
        outstr+= '{%sL,%sL,%sL}'%(l[3],l[4],l[5])+','
    outstr= outstr[:-1]+'};'
    #GRACEVEL
    outstr += '\nconstexpr double GRACEVEL[][3]={'
    for line in data:
        l= line.decode('utf-8').split(' ')
        outstr+= '{%sL,%sL,%sL}'%(l[9],l[10],l[11])+','
    outstr= outstr[:-1]+'};'
    with open(headerfilename,'w') as f:
        f.write(outstr)



if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="""
    #Nathan Zimmerberg (nhz2@cornell.edu)
    #11 APR 2020
    #graceheadergen.py
    #This is a simple script to download GRACE FO data files and turn it into a header file
    #To run, use command "python graceheadergen.py", add -h flag for help
    """)
    parser.add_argument('-o',type=str,default='GRACEORBITS.hpp',help='the c++ header filename to write the data')
    parser.add_argument('-y',type=int,default=2019,help='year')
    parser.add_argument('-m',type=int,default=1,help='month')
    parser.add_argument('-d',type=int,default=1,help='day')
    parser.add_argument('-s',type=str,default='D',help='sat name either "C" or "D"')

    arg=parser.parse_args()

    main(arg.o,arg.y,arg.m,arg.d,arg.s)
