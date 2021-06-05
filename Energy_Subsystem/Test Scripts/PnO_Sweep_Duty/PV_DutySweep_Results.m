clear; close all;

import = readmatrix('PVPWMSWE.CSV');

% Test 1: 120R resistor on port A, asynchronous
% Newest Test: 120+75R resistor on port A, synchronous

%%
pwm = import(:,2);
voltage = import(:,3);
% voltage = import(:,5);
current = -import(:,4);

power = zeros(size(current,1),1);
for i = 1:size(current,1)
    power(i) = voltage(i)*current(i)/1000000;
end

%%

figure(1);

subplot(3,1,1);
plot(voltage, current, 'rx');
ylabel('I_{PV} (mA)');
xlabel('V_{PV} (mV)');

subplot(3,1,2);
plot(voltage, power, 'rx');
ylabel('P_{PV} (W)');
xlabel('V_{PV} (mV)');

subplot(3,1,3);
plot(pwm*100, power, 'rx');
xlabel('PWM (%)');
ylabel('P_{PV} (W)');

figure(2);

subplot(3,1,1);
plot(pwm*100, current, 'rx');
xlabel('PWM (%)');
ylabel('I_{PV} (mA)');

subplot(3,1,2);
plot(pwm*100, voltage, 'rx');
xlabel('PWM (%)');
ylabel('V_{PV} (mV)');

subplot(3,1,3);
plot(pwm*100, power, 'rx');
xlabel('PWM (%)');
ylabel('P_{PV} (W)');





