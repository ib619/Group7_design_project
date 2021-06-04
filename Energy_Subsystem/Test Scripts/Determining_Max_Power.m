clear; close all;

PWM = [0.25 0.5 0.75 1];
V = [0.74 1.47 2.22 2.97];
I = [0.04 0.043 0.046 0.055];

%%

figure(1);
plot(PWM, V)
xlabel('PWM (%)');
ylabel('V (V)');
title('PWM against V');

figure(2);
plot(I, V)
xlabel('I (A)');
ylabel('V (V)');
title('I against V');
