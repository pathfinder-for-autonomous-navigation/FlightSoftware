#11 APR 2020
#graceheadergen.py
#This is a simple script to parse a coefficient file into a c++ header file
#To run, use command "python gravmodelgen.py", add -h flag for help
#it uses fully normalized coefficient in the zero tide tide system
# The coefficients are stored by:
#    index=((((NMAX<<1) - m + 1)*m)>>1) + n

import math


def main(infilename, headerfilename, maxdegree,earth_radius,earth_gravity_constant):
    """parse infilename into headerfilename c++ header file

    The coefficents are fully-normalized.
    indexing is by ((2*maxdegree-m+1)*m)/2+n
    Args:
        infilename(filename): the .COF file that contains the
            spherical harmonic coefficents, download this from http://www2.csr.utexas.edu/grace/gravity/
        headerfilename(string ending in .hpp): the c++ header file.
        max_degree(positive int >1): max degree and order of the model
        earth_radius(positive float): the radius of the earth for the model (m), ex 0.6378136300E+07
        earth_gravity_constant(positive float): the GM of the earth for the model (m^3/s^2), ex 0.3986004415E+15
            Stored in a Coeff, index=((2*maxdegree-m+1)*m)/2+n
                template<int NMAX>//NMAX maximum degree and order
                struct Coeff{
                    real_t earth_radius;
                    real_t earth_gravity_constant;
                    real_t _Cnm[Csize(NMAX, NMAX)];
                    real_t _Snm[Ssize(NMAX, NMAX)];"""
    data= parseescof(infilename,maxdegree)
    cofs= data
    c_cofs=[0]*(((maxdegree+1)*(maxdegree+2))//2)
    s_cofs=[0]*(((maxdegree+1)*(maxdegree+2))//2)
    outstr = '#pragma once\n#include "geograv.hpp"\n//file generated from %s, by gravmodelgen.py\n'%(infilename)
    for cof in cofs:
        n= cof[0]
        m= cof[1]
        c= cof[2]
        s= cof[3]
        c_cofs[((2*maxdegree-m+1)*m)//2+n]= c
        s_cofs[((2*maxdegree-m+1)*m)//2+n]= s
    c_cofs[0]= 0;#manually set the earth mass term to zero
    j2=c_cofs[2]
    c_cofs[2]= 0;#manually set the J2 term to zero
    outstr = outstr+header_file_model_code(headerfilename[:-4],maxdegree,earth_radius,earth_gravity_constant,j2,c_cofs,s_cofs)
    with open(headerfilename,'w') as f:
        f.write(outstr)


def header_file_model_code(modelname,max_degree, earth_radius,earth_gravity_constant,j2,c_cofs,s_cofs):
    """return the code defining the spherical coefficents and model
            Stored in a Coeff, index=((2*maxdegree-m+1)*m)/2+n
                template<int NMAX>//NMAX maximum degree and order
                struct Coeff{
                    real_t earth_radius;
                    real_t earth_gravity_constant;
                    real_t _Cnm[Csize(NMAX, NMAX)];
                    real_t _Snm[Ssize(NMAX, NMAX)];

    Args:
        modelname(str, a valid C++ name): name of the model
        max_degree(positive int >1): max degree and order of the model
        earth_radius(positive float): the radius of the earth for the model (m), ex 0.6378136300E+07
        earth_gravity_constant(positive float): the GM of the earth for the model (m^3/s^2), ex 0.3986004415E+15
        j2(positive float): the j2 or c_cofs[2] term ex, -0.000484165371736
        c_cofs,s_cofs(list of floats): coefficents of the model"""

    head="constexpr geograv::Coeff<%d> %s={%sL,%sL,%sL,\n"%(max_degree,modelname,repr(earth_radius),repr(earth_gravity_constant),repr(j2))
    modeltail= '};\n\n'
    cs='{'
    ss='{'
    for i in range(len(c_cofs)):
        cs+= repr(c_cofs[i])+'L,'
        ss+= repr(s_cofs[i])+'L,'
    cs= cs[:-1]+'}'
    ss= ss[:-1]+'}'
    sqrttable='{'
    for i in range(max(2*max_degree+ 5, 15) + 1):
        sqrttable+= repr(math.sqrt(i))+'L,'
    sqrttable= sqrttable[:-1]+'}'
    return head+cs+',\n'+ss+',\n'+sqrttable+modeltail


def parseescof(infilename, maxdegree):
    """return a list of lists from the infilename cof data file
    n,m,C,S,
    ...

    Args:
        infilename(string ending in .COF): the .COF file that contains the
            spherical harmonic coefficents, download this from https://www.ngdc.noaa.gov/geomag/WMM/DoDWMM.shtml
        maxdegree(positive integer): maximum degree"""
    with open(infilename,'r') as f:
        filestr= f.read()
        filelines= filestr.split('\n')
        l=[]
        C=0
        S=0
        n=0
        m=0
        i=0
        while(n<=maxdegree and m<=maxdegree and i<len(filelines)):
            l.append([n,m,C,S])
            rowa= filelines[i].split()
            try:
                n=int(rowa[0])
                m=int(rowa[1])
                C=float(rowa[2].replace('D','E'))
                S=float(rowa[3].replace('D','E'))
            except:
                try:
                    n=int(rowa[1])
                    m=int(rowa[2])
                    C=float(rowa[3].replace('D','E'))
                    S=float(rowa[4].replace('D','E'))
                except:
                    pass
            i+=1
        return l



if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="""
    #Nathan Zimmerberg
    #21 NOV 2019
    #gravmodelgen.py
    #This is a simple script to parse a coefficient file into a c++ header file
    #To run, use command "python gravmodelgen.py", add -h flag for help
    #it uses fully normalized coefficient in the zero tide tide system
    # The coefficients are stored by:
    #    index=((((NMAX<<1) - m + 1)*m)>>1) + n
    """)
    parser.add_argument('-f',type=str,default='GGM05S.GEO',help="""the file that contains the
        sherical harmonic coefficents""")
    parser.add_argument('-o',type=str,default='GGM05S.hpp',help='the c++ header filename to write the coefficents, and the name of the model')
    parser.add_argument('-n',type=int,default=20,help='maximum number of degrees to use')
    parser.add_argument('-r',type=float,default=0.6378136300E+07,help='radius of earth for model(m)')
    parser.add_argument('-u',type=float,default=0.3986004415E+15,help='earth_gravity_constant for model(m^3/s^2)')

    arg=parser.parse_args()

    main(arg.f,arg.o,arg.n,arg.r,arg.u)
