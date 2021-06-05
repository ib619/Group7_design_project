clear; close all;

import = readmatrix('PVCELLDT_LampOff.CSV');

% Test 1: 120R resistor on port A, asynchronous
% Newest Test: 120+75R resistor on port A, synchronous

%%
pwm = import(:,2);
v_b = import(:,3);
v_a = import(:,5);
current = import(:,4);

power = zeros(size(current,1),1);
for i = 1:size(current,1)
    power(i) = v_a(i)*current(i)/1000000;
end

%%

figure(1);

subplot(3,1,1);
plot(v_a, current, 'rx');
ylabel('I_{PV} (mA)');
xlabel('V_{PV} (mV)');

subplot(3,1,2);
plot(v_a, power, 'rx');
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
plot(pwm*100, v_a, 'rx');
xlabel('PWM (%)');
ylabel('V_{PV} (mV)');

subplot(3,1,3);
plot(pwm*100, power, 'rx');
xlabel('PWM (%)');
ylabel('P_{PV} (W)');


%figure(2);
%plot(current, current.*v_b, 'rx');
%ylabel('P_{PV} (W)');
%xlabel('I_{PV} (mA)');




