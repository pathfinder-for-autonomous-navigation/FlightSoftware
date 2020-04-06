import sympy as sp
from sympy.vector import CoordSys3D, express
from sympy.codegen.rewriting import optimize, optims_c99
from sympy.codegen.rewriting import create_expand_pow_optimization
from sympy.codegen.ast import Assignment
mu,t,w,e1,e2,e3,dt= sp.symbols('mu t w e1 e2 e3 dt')
x0,y0,z0,vx0,vy0,vz0= sp.symbols('x y z v_x v_y v_z')
sp.init_printing(use_unicode=True)
#position and velocity
#intial frame
ECEF0 = CoordSys3D('ECEF0')
r= x0*ECEF0.i+y0*ECEF0.j+z0*ECEF0.k
v_E= vx0*ECEF0.i+vy0*ECEF0.j+vz0*ECEF0.k
earth_axis= ECEF0.k
earth_rate= w*earth_axis

v_I= earth_rate.cross(r)+v_E

#inertial frames
ECEFhalf = ECEF0.orient_new_axis('ECEFhalf', w*dt/2,earth_axis)
ECEF1 = ECEF0.orient_new_axis('ECEF1', w*dt,earth_axis)
#drift
r= r+v_I*dt/2
#kick
#pos_ecef_half= r.to_matrix(ECEFhalf);
g= -mu*r/(r.magnitude()**3)
v_I= v_I+g*dt
#drift
r= r+v_I*dt/2
v_E= v_I - earth_rate.cross(r);
v_ecef= v_E.to_matrix(ECEF1)
r_ecef= r.to_matrix(ECEF1)

# sp.ccode(r_ecef[0].diff(x0))
#sp.ccode(optimize(expand_opt(r_ecef[0].diff(x0)), optims_c99))
outputs= [r_ecef[0],r_ecef[1],r_ecef[2],v_ecef[0],v_ecef[1],v_ecef[2]]
inputs= [x0,y0,z0,vz0,vy0,vz0]
#sp.cse((r_ecef[0].diff(x0),r_ecef[1].diff(x0),r_ecef[2].diff(x0))
expressions=[]
for outp in outputs:
    for inp in inputs:
        expressions.append(outp.diff(inp))
cseofmatrix=sp.cse(expressions)

def csetupletocline(t,real_type):
    """Returns a string line of c code of t, a tuple from sympy cse.
        Uses real_type a string of either "double" or "float" """
    expand_opt = create_expand_pow_optimization(3)
    return real_type + " " + sp.ccode(Assignment(t[0],expand_opt(t[1])))

[print(csetupletocline(t,"double")) for t in cseofmatrix[0]]
[print("jac(%d,%d)= %s;"%(i//6,i%6,str(cseofmatrix[1][i]))) for i in range(len(cseofmatrix[1]))]