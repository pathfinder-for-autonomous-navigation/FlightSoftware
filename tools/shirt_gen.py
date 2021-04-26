f = open("leader.csv", "w")
f.write("")

mult = 10
#one_day_hr
odh = 10
l_states = [0,1,3,5,7,8,9]
f_states = [0,1,3,4,6,8,9]

l_dur = [0.5, 1.5, 24, 5*odh, 7*odh, 0.1, 1]
f_dur = l_dur

l_print = []
f_print = []

for s,d in zip(l_states, l_dur):
    for _ in range(int(mult*d)):
        l_print.append(s)

for s,d in zip(f_states, f_dur):
    for _ in range(int(mult*d)):
        f_print.append(s)

for a,b in zip(l_print, f_print):
    f.write(f'{a}, {b},\n')

f.close()