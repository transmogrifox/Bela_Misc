% Pre-emphasis stage for King of Tone 
% Implementation like BluesBreaker first stage
k = 1000;
n = 1e-9;

fs=44100.0;
Ts = 1/fs;

f=10:10:fs;
w=2*pi*f;
s = j.*w;

z1 = e.^(-s*Ts);
kz = 2/Ts;
kz2 = kz*kz;

% Cirtuit params
r1 = 27*k;
r2 = 33*k;
c1 = 10*n;
c2 = 10*n;

% Brute-force analog model
zr2 = r2 + 1./(s*c2);
Zx = zr2.*r1./(zr2 + r1) + 1./(s*c1);
Hix = 1./Zx;

% Check algebra
ga = (r1 + r2)/(r1*r2);
gs = 1/ga;
z0=1/(r2*c2);
p0 = 1/((r1+r2)*c2);

B2 = 0;
B1 = ga*p0;
B0 = ga;
A0 = 1;
A1 = (c1*gs*z0+1)/(c1*gs);
A2 = p0/(c1*gs);

Zs = gs.*(s + z0)./(s + p0) + 1./(s*c1);
Hs = 1./Zs;

Hs = ga*(s.*s + B1*s)./(s.*s + A1*s + A2);

% DSP compare
sz = kz.*(1-z1)./(1+z1);

zb0 =(B0*kz2 + B1*kz + B2)/(A0*kz2+A1*kz+A2);
zb1 =(2*B2 - 2*B0*kz2)/(A0*kz2+A1*kz+A2);
zb2 =(B0*kz2 - B1*kz + B2)/(A0*kz2+A1*kz+A2);

za1 =(2*A2-2*A0*kz2)/(A0*kz2+A1*kz+A2);
za0 =(A0*kz2-A1*kz+A2)/(A0*kz2+A1*kz+A2);

%Hz = ga*(sz.*sz + A1*sz)./(sz.*sz + B1*sz + B0);
Hz = (zb0.*z1.*z1 + zb1.*z1 + zb2)./(za0.*z1.*z1 + za1.*z1 + 1);

zn0 = 1;
zn1 = -2*kz*kz/(kz*kz - k*A1);
zn2 = 1;

zd0 = (kz*kz - kz*B1 + B0)/(kz*kz - kz*B1 + B0);
zd1 = (2*B0 - 2*kz*kz)/(kz*kz - kz*B1 + B0);
zd2 = 1;

gz = (kz*kz - k*B1)/(kz*kz - kz*B1 + B0);

%Hz = ga.*gz.*(zn2.*z1.*z1 + zn1.*z1 + zn1)./(zd2.*z1.*z1 + zd1.*z1 + zd0);

semilogx(f,20*log10(Hix),f,20*log10(Hz));
