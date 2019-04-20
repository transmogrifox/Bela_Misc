% King of Tone Analysis
%   --Tonestack

% Powers of 10 notation
k = 1000;
m = 1e-3;
u = 1e-6;
n = 1e-9;

% Frequency variables
fs = 44100;
f = 1:(fs/2);
w = 2*pi*f;
s = j.*w;

% DSP
z1 = e.^(-s/fs);
z2 = e.^(-2*s/fs);
kk = 2*fs;

% Pre-warping
wk = 2*pi*1*k;
%kk = wk/(tan(wk/(2*fs)));
sz = kk*(1 - z1)./(1 + z1);

% Tonestack components
ri = 1*k;         % Feed resistor from final op amp output
rtone = 25*k;     % Tone Pot
apos = 0.0;      % Tone Pot position, primary leg resistance ratio
bpos = 1 - apos;  % Alternate pot leg resistance ratio
rboost = 50*k;    % Boost pot
bstpos = 0.01;     % Boost pot position resistance ratio

% Components as labeled in derivation
ra = ri + apos*rtone;
rb = bpos*rtone + rtone/fs; % This sets one of the zeros, so it needs to avoid going to zero
rc = 6.8*k;
rd = rboost*bstpos;
ro = 100*k*1000*k/(100*k + 1000*k);
c1 = 10*n;
c2 = 10*n;

% Substitutions as labled in derivation
a0 = ro + rc;
a1 = (ro + rd).*rc.*c2 + ro.*rd.*c2;
b1 = (ro+rd).*c2;

N0 = a0;
N1 = rb*c1*a0 + a1;
N2 = rb*c1*a1;
D0 = 1;
D1 = rb*c1 + c1*a0 + b1;
D2 = rb*c1*b1 + c1*a1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Transfer functions  %%%%%%%%%%%%%%%%%%%%%%%%%%%%

% ********************************** Vx/Vi ********************************** %
%vxvi = (s.*s*N2 + s.*N1 + N0)./(s.*s.*(N2 + ra.*D2) + s.*(N1 + ra.*D1) + N0 + ra.*D0);
% Further simplification
gvx = N2/(N2 + ra.*D2);
A0 = N0/N2;
A1 = N1/N2;
B0 = (N0 + ra*D0)/(N2 + ra.*D2);
B1 = (N1 + ra*D1)/(N2 + ra.*D2);

vxvi = gvx.*(s.*s + A1*s + A0)./(s.*s + B1*s + B0);  % Final expression for Vx/Vi
                                                     % in the most simplified
                                                     % form: 
                                                     % Second-order filter 
                                                     % with gain

% ********************************** Vo/Vx ********************************** %
%vovx = (s.*c2*ro*rd + ro)./(s.*c2*(ro*rd+(ro+rd)*rc) + rc + ro);
gvo = (c2*ro*rd)/(c2*(ro*rd+(ro+rd)*rc));
%vovx = gvo.*(s + 1/(c2*rd ))./(s + (rc + ro)/(c2*(ro*rd+(ro+rd)*rc)));

X0 = 1/(c2*rd );
Y0 = (rc + ro)/(c2*(ro*rd+(ro+rd)*rc));

vovx = gvo*(s + X0)./(s + Y0);  % Final expression for Vo/Vx in the most 
                                % simplified form: single pole/zero pair
                                % with gain

% ********************* H(s) = Vo/Vi = (Vx/Vi)*(Vo/Vx) ********************** %
Hs = vxvi.*vovx;  % With more algebra the entire expression could be reduced to 
                  % a second-order transfer function, but the expressions for 
                  % "vxvi" and "vovx" are easy to transform to discrete time
                  % approximations directly, avoiding the extra steps.  
                  % If the computational efficiency for the discrete-time
                  % system is paramount then this exercise should be completed.
                  % For current needs further simplification is considered a
                  % needless expenditure of time, so the final DSP algorithm
                  % will require more computational cost than strictly necessary
                  % for this filter implementation. 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Bilinear Transform  %%%%%%%%%%%%%%%%%%%%%%%%%%%

% ********************************** Vx/Vi ********************************** %
% ************************** Aggregate Coefficients ************************* %
gblt = ((A0 + kk*A1 + kk.*kk)./(B0 + kk*B1 +kk*kk));
gdsp = gvx.*gblt
A1blt = ((2*A0-2*kk*kk)./(A0+kk*A1+kk*kk))
A2blt = ((A0-kk*A1+kk*kk)./(A0+kk*A1+kk*kk))
B1blt = ((2*B0-2*kk*kk)./(B0+kk*B1+kk*kk))
B2blt = ((B0-kk.*B1+kk.*kk)./(B0+kk.*B1+kk.*kk))

% **************************** Stage-1 DSP Filter *************************** %

vxviblt = gdsp.*(A2blt.*z2 + A1blt.*z1 + 1) ./ ( B2blt.*z2 + B1blt.*z1 + 1);

% ********************************** Vo/Vx ********************************** %
% ************************** Aggregate Coefficients ************************* %

g2blt = (X0 + kk)./(Y0 + kk);
gdsp2 = gvo.*g2blt
X0blt = (X0 - kk)./(X0 + kk)
Y0blt = (Y0 - kk)./(Y0 + kk)

% **************************** Stage-2 DSP Filter *************************** %

vovxblt = gdsp2*(X0blt*z1 + 1)./(Y0blt*z1 + 1);

%
% ***************** Cascade Stage-1 and Stage-2 DSP Filters ***************** %
%

Hz = vxviblt.*vovxblt;  
%Hz = vxviblt;   
%Hz = vovxblt;               

%
%%%%%%%%%%%%%%%%%%%%%%%%%%% Check Transfer Functions  %%%%%%%%%%%%%%%%%%%%%%%%%
%

% Check algebra steps used to simplify transfer functions
% Below MATLAB/Octave can handle the complex impedances numerically,
% so the expressions can be entered using a straight-forward approach.
% The final result can be verified using SPICE
zd = rd + 1./(s*c2);
zp = ro.*zd./(ro + zd);
zb = rc + zp;
zc = rb + 1./(s*c1);
zx = zb.*zc./(zb + zc);
vxvick = zx./(zx + ra);
vovxck = zp./(zp + rc);

Hs2 = vxvick.*vovxck;

% BLT Check
%vxviblt = gvx.*(sz.*sz + A1*sz + A0)./(sz.*sz + B1*sz + B0);
%vovxblt = gvo*(sz + X0)./(sz + Y0);
%Hz = vxviblt.*vovxblt;

% Plot
semilogx(f, 20*log10(Hs), 'r')
hold on
semilogx(f, 20*log10(Hs2*0.997), 'g') % when the traces overlap, the 0.997 factor makes them side-by-side visible
semilogx(f, 20*log10(Hz), 'm') 
%hold off



